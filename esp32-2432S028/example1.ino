#include <lvgl.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9341 _panel_instance;
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;
  //----------------------------------------------------------------------
public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = 14;
      cfg.pin_mosi = 13;
      cfg.pin_miso = 12;
      cfg.pin_dc = 2;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 15;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.panel_width = 240;
      cfg.panel_height = 320;   // 実際に表示可能な高さ
      cfg.offset_x = 0;         // パネルのX方向オフセット量
      cfg.offset_y = 0;         // パネルのY方向オフセット量
      cfg.offset_rotation = 0;  // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel = 8; // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits = 1;  // ピクセル外のデータ読出し前のダミーリードのビット数
      cfg.readable = true;      // データ読出しが可能な場合 trueに設定
      cfg.invert = false;       // パネルの明暗が反転場合 trueに設定
      cfg.rgb_order = false;    // パネルの赤と青が入れ替わる場合 trueに設定 ok
      cfg.dlen_16bit = false;   // データ長16bit単位で送信するパネル trueに設定
      // cfg.bus_shared      = false;
      cfg.bus_shared = true; // Manual
      _panel_instance.config(cfg);
    }
    {                                      // バックライト制御の設定を行います。(必要なければ削除）
      auto cfg = _light_instance.config(); // バックライト設定用の構造体を取得します。
      cfg.pin_bl = 21;                     // バックライトが接続されているピン番号 BL
      cfg.invert = false;                  // バックライトの輝度を反転させる場合 true
      cfg.freq = 44100;                    // バックライトのPWM周波数
      cfg.pwm_channel = 7;                 // 使用するPWMのチャンネル番号
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // バックライトをパネルにセットします。
    }
    { // タッチスクリーン制御の設定を行います。（必要なければ削除）
      auto cfg = _touch_instance.config();

      // Manual config
      cfg.x_min = 0;           // タッチスクリーンから得られる最小のX値(生の値)
      cfg.x_max = 239;         // タッチスクリーンから得られる最大のX値(生の値)
      cfg.y_min = 0;           // タッチスクリーンから得られる最小のY値(生の値)
      cfg.y_max = 319;         // タッチスクリーンから得られる最大のY値(生の値)
      cfg.pin_int = -1;        // INTが接続されているピン番号, TP IRQ 36
      cfg.bus_shared = true;   // 画面と共通のバスを使用している場合 trueを設定
      cfg.offset_rotation = 0; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

      cfg.spi_host = VSPI_HOST; // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
      cfg.freq = 1000000;       // SPIクロックを設定
      cfg.pin_sclk = 25;        // SCLKが接続されているピン番号, TP CLK
      cfg.pin_mosi = 32;        // MOSIが接続されているピン番号, TP DIN
      cfg.pin_miso = 39;        // MISOが接続されているピン番号, TP DOUT
      cfg.pin_cs = 33;          // CS  が接続されているピン番号, TP CS
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance); // タッチスクリーンをパネルにセットします。
    }
    setPanel(&_panel_instance); // 使用するパネルをセットします。
  }
};

LGFX tft;

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  // tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

void setup()
{
  Serial.begin(115200); /* prepare for possible serial debug */

  tft.begin();        /* TFT init */
  tft.setRotation(1); /* Landscape orientation, flipped */
  tft.setBrightness(255);
  uint16_t calData[] = {357, 3689, 251, 173, 3733, 3659, 3707, 168};
  tft.setTouchCalibrate(calData);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  lv_example_get_started_1();
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}

static void textarea_event_handler(lv_event_t *e)
{
  lv_obj_t *ta = lv_event_get_target(e);
  LV_UNUSED(ta);
  LV_LOG_USER("Enter was pressed. The current text is: %s", lv_textarea_get_text(ta));
}

static void btnm_event_handler(lv_event_t *e)
{
  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
  const char *txt = lv_btnmatrix_get_btn_text(obj, lv_btnmatrix_get_selected_btn(obj));

  if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
    lv_textarea_del_char(ta);
  else if (strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
    lv_event_send(ta, LV_EVENT_READY, NULL);
  else
    lv_textarea_add_text(ta, txt);
}

void lv_example_get_started_1(void)
{
  /*Change the active screen's background color to black */
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_PART_MAIN);

  /*Create a white label, set its text and align it to the center*/
  lv_obj_t *corplabel = lv_label_create(lv_scr_act());
  lv_label_set_text(corplabel, "Enter your CorpID");
  lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(corplabel, LV_ALIGN_TOP_MID, 0, 0);

  lv_obj_t *ta = lv_textarea_create(lv_scr_act());
  lv_textarea_set_one_line(ta, true);
  lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_add_event_cb(ta, textarea_event_handler, LV_EVENT_READY, ta);
  lv_obj_add_state(ta, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/

  static const char *btnm_map[] = {"1", "2", "3", "\n",
                                   "4", "5", "6", "\n",
                                   "7", "8", "9", "\n",
                                   LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""};

  lv_obj_t *btnm = lv_btnmatrix_create(lv_scr_act());
  lv_obj_set_size(btnm, 200, 150);
  lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_add_event_cb(btnm, btnm_event_handler, LV_EVENT_VALUE_CHANGED, ta);
  lv_obj_clear_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE); /*To keep the text area focused on button clicks*/
  lv_btnmatrix_set_map(btnm, btnm_map);
}
