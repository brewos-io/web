/**
 * BrewOS Display Driver Implementation
 * 
 * Uses LovyanGFX for hardware abstraction
 */

#include "display/display.h"
#include "config.h"

// LovyanGFX for display driver
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// =============================================================================
// LovyanGFX Panel Configuration for Round Display
// =============================================================================

class LGFX : public lgfx::LGFX_Device {
    // SPI bus instance
    lgfx::Bus_SPI _bus_instance;
    
    // Panel instance (GC9A01 is common for round displays)
    lgfx::Panel_GC9A01 _panel_instance;
    
    // Backlight control
    lgfx::Light_PWM _light_instance;
    
public:
    LGFX(void) {
        // SPI Bus configuration
        {
            auto cfg = _bus_instance.config();
            
            cfg.spi_host = DISPLAY_SPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = DISPLAY_SPI_FREQ;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = DISPLAY_SCLK_PIN;
            cfg.pin_mosi = DISPLAY_MOSI_PIN;
            cfg.pin_miso = -1;  // Not used
            cfg.pin_dc = DISPLAY_DC_PIN;
            
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        
        // Panel configuration
        {
            auto cfg = _panel_instance.config();
            
            cfg.pin_cs = DISPLAY_CS_PIN;
            cfg.pin_rst = DISPLAY_RST_PIN;
            cfg.pin_busy = -1;
            
            cfg.panel_width = DISPLAY_WIDTH;
            cfg.panel_height = DISPLAY_HEIGHT;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;      // GC9A01 typically needs invert
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            
            _panel_instance.config(cfg);
        }
        
        // Backlight configuration
        {
            auto cfg = _light_instance.config();
            
            cfg.pin_bl = DISPLAY_BL_PIN;
            cfg.invert = false;
            cfg.freq = BACKLIGHT_PWM_FREQ;
            cfg.pwm_channel = BACKLIGHT_PWM_CHANNEL;
            
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }
        
        setPanel(&_panel_instance);
    }
};

// Global LovyanGFX instance
static LGFX lcd;

// Global display instance
Display display;

// =============================================================================
// Display Class Implementation
// =============================================================================

Display::Display()
    : _display(nullptr)
    , _buf1(nullptr)
    , _buf2(nullptr)
    , _backlightLevel(BACKLIGHT_DEFAULT)
    , _backlightSaved(BACKLIGHT_DEFAULT)
    , _isDimmed(false)
    , _lastActivityTime(0) {
}

bool Display::begin() {
    LOG_I("Initializing display...");
    
    // Initialize hardware
    initHardware();
    
    // Initialize LVGL
    initLVGL();
    
    // Set initial backlight
    setBacklight(BACKLIGHT_DEFAULT);
    resetIdleTimer();
    
    LOG_I("Display initialized: %dx%d", DISPLAY_WIDTH, DISPLAY_HEIGHT);
    return true;
}

void Display::initHardware() {
    // Initialize LovyanGFX
    lcd.init();
    lcd.setRotation(DISPLAY_ROTATION / 90);
    lcd.setBrightness(BACKLIGHT_DEFAULT);
    
    // Clear screen to black
    lcd.fillScreen(TFT_BLACK);
    
    LOG_I("Display hardware initialized");
}

void Display::initLVGL() {
    // Initialize LVGL
    lv_init();
    
    // Allocate display buffers in PSRAM for best performance
    size_t bufferSize = LVGL_BUFFER_SIZE * sizeof(lv_color_t);
    
    _buf1 = (lv_color_t*)heap_caps_malloc(bufferSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!_buf1) {
        LOG_E("Failed to allocate LVGL buffer 1 in PSRAM, trying regular RAM");
        _buf1 = (lv_color_t*)malloc(bufferSize);
    }
    
#if LVGL_DOUBLE_BUFFER
    _buf2 = (lv_color_t*)heap_caps_malloc(bufferSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!_buf2) {
        LOG_W("Failed to allocate LVGL buffer 2, using single buffer");
        _buf2 = nullptr;
    }
#endif
    
    if (!_buf1) {
        LOG_E("Failed to allocate any LVGL buffer!");
        return;
    }
    
    // Initialize draw buffer
    lv_disp_draw_buf_init(&_drawBuf, _buf1, _buf2, LVGL_BUFFER_SIZE);
    
    // Initialize display driver
    lv_disp_drv_init(&_dispDrv);
    _dispDrv.hor_res = DISPLAY_WIDTH;
    _dispDrv.ver_res = DISPLAY_HEIGHT;
    _dispDrv.flush_cb = flushCallback;
    _dispDrv.draw_buf = &_drawBuf;
    _dispDrv.user_data = this;
    
    // Register display
    _display = lv_disp_drv_register(&_dispDrv);
    
    LOG_I("LVGL initialized with %s buffer (%d bytes)", 
          _buf2 ? "double" : "single",
          bufferSize);
}

void Display::flushCallback(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    // Use LovyanGFX to push pixels
    lcd.startWrite();
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    lcd.writePixels((lgfx::rgb565_t*)color_p, w * h);
    lcd.endWrite();
    
    // Notify LVGL that flushing is done
    lv_disp_flush_ready(drv);
}

uint32_t Display::update() {
    // Update backlight based on idle time
    updateBacklightIdle();
    
    // Run LVGL timer handler
    return lv_timer_handler();
}

void Display::setBacklight(uint8_t brightness) {
    _backlightLevel = brightness;
    lcd.setBrightness(brightness);
}

void Display::backlightOn() {
    _isDimmed = false;
    setBacklight(_backlightSaved);
}

void Display::backlightOff() {
    _backlightSaved = _backlightLevel;
    setBacklight(0);
}

void Display::resetIdleTimer() {
    _lastActivityTime = millis();
    
    // If dimmed, restore brightness
    if (_isDimmed) {
        _isDimmed = false;
        setBacklight(_backlightSaved);
    }
}

void Display::updateBacklightIdle() {
    unsigned long now = millis();
    unsigned long idleTime = now - _lastActivityTime;
    
#if BACKLIGHT_OFF_TIMEOUT > 0
    // Check for off timeout
    if (idleTime >= BACKLIGHT_OFF_TIMEOUT && _backlightLevel > 0) {
        if (!_isDimmed) {
            _backlightSaved = _backlightLevel;
        }
        setBacklight(0);
        _isDimmed = true;
        return;
    }
#endif
    
    // Check for dim timeout
    if (idleTime >= BACKLIGHT_DIM_TIMEOUT && !_isDimmed) {
        _backlightSaved = _backlightLevel;
        setBacklight(BACKLIGHT_DIM_LEVEL);
        _isDimmed = true;
    }
}

