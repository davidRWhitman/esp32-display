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
      // cfg.spi_host = VSPI_HOST; // Manual

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

      _bus_instance.config(cfg);              // 設定値をバスに反映します。
      _panel_instance.setBus(&_bus_instance); // バスをパネルにセットします。
    }
    {                                      // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config(); // 表示パネル設定用の構造体を取得します。
      cfg.pin_cs = 15;                     // CS  が接続されているピン番号(-1 = disable)
      cfg.pin_rst = -1;                    // RST が接続されているピン番号(-1 = disable)
      cfg.pin_busy = -1;                   // BUSYが接続されているピン番号(-1 = disable)
      cfg.memory_width = 240;              // ドライバICがサポートしている最大の幅
      cfg.memory_height = 320;             // ドライバICがサポートしている最大の高さ
      cfg.panel_width = 240;               // 実際に表示可能な幅
      cfg.panel_height = 320;              // 実際に表示可能な高さ
      cfg.offset_x = 0;                    // パネルのX方向オフセット量
      cfg.offset_y = 0;                    // パネルのY方向オフセット量
      cfg.offset_rotation = 0;             // 回転方向の値のオフセット 0~7 (4~7は上下反転)
      cfg.dummy_read_pixel = 8;            // ピクセル読出し前のダミーリードのビット数
      cfg.dummy_read_bits = 1;             // ピクセル外のデータ読出し前のダミーリードのビット数
      cfg.readable = true;                 // データ読出しが可能な場合 trueに設定
      cfg.invert = false;                  // パネルの明暗が反転場合 trueに設定
      cfg.rgb_order = false;               // パネルの赤と青が入れ替わる場合 trueに設定 ok
      cfg.dlen_16bit = false;              // データ長16bit単位で送信するパネル trueに設定
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
      // From Macsbug Wordpress..
      // cfg.x_min      = 300;    // タッチスクリーンから得られる最小のX値(生の値)
      // cfg.x_max      = 3900;   // タッチスクリーンから得られる最大のX値(生の値)
      // cfg.y_min      = 200;    // タッチスクリーンから得られる最小のY値(生の値)
      // cfg.y_max      = 3700;   // タッチスクリーンから得られる最大のY値(生の値)
      // cfg.pin_int    = -1;     // INTが接続されているピン番号, TP IRQ 36
      // cfg.bus_shared = false;  // 画面と共通のバスを使用している場合 trueを設定
      // cfg.offset_rotation = 6; // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定

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