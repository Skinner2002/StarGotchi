#pragma once
// Board: Waveshare ESP32-S3-Touch-AMOLED-1.75 (see HARDWARE_REFERENCE.md
// in waveshareteam/ESP32-S3-Touch-AMOLED-1.75).

#define LCD_WIDTH 466
#define LCD_HEIGHT 466

#define PIN_LCD_SDIO0 4
#define PIN_LCD_SDIO1 5
#define PIN_LCD_SDIO2 6
#define PIN_LCD_SDIO3 7
#define PIN_LCD_SCLK 38
#define PIN_LCD_CS 12
#define PIN_LCD_RESET 39

#define PIN_I2C_SDA 15
#define PIN_I2C_SCL 14

#define PIN_TP_INT 11
#define PIN_TP_RESET 40
#define TOUCH_I2C_ADDR 0x5A

#define PIN_I2S_MCLK 42
#define PIN_I2S_BCLK 9
#define PIN_I2S_WS 45
#define PIN_I2S_DOUT 8   // ESP32 -> ES8311 DSDIN
#define PIN_I2S_DIN 10   // ES7210 -> ESP32 (unused)
#define PIN_PA_CTRL 46
