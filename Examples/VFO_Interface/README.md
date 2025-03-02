# ADF4351 VFO Interface

This example implements a VFO (Variable Frequency Oscillator) interface for the ADF4351 frequency synthesizer. It provides a user-friendly way to control the ADF4351 using a rotary encoder and various display options.

## Features

- Frequency tuning using a rotary encoder with debouncing
- Support for multiple display types (LCD, OLED, TFT)
- Band selection button to cycle through ham radio bands
- Step size selection button to change tuning resolution
- RF output toggle and power level control
- Real-time frequency display with appropriate units

## Hardware Requirements

- Arduino-compatible board (Raspberry Pi Pico 2 recommended)
- ADF4351 frequency synthesizer module
- Rotary encoder with push button
- Display (choose one):
  - I2C LCD display (16x2 or 20x4)
  - I2C OLED display (128x64)
  - SPI TFT display (ST7735 or similar)
- 3 push buttons for band selection, step size, and function control

## Wiring Connections

### ADF4351 Connections
- LE (Latch Enable) -> GPIO 5
- CLK (Clock) -> GPIO 2
- DATA (Data) -> GPIO 3
- CE (Chip Enable) -> GPIO 4

### Rotary Encoder
- Pin A -> GPIO 6
- Pin B -> GPIO 7
- Button -> GPIO 8

### Control Buttons
- Band Button -> GPIO 9
- Step Button -> GPIO 10
- Function Button -> GPIO 11

### Display Connections
#### I2C LCD
- SDA -> I2C SDA pin on your board
- SCL -> I2C SCL pin on your board
- VCC -> 5V
- GND -> GND

#### I2C OLED
- SDA -> I2C SDA pin on your board
- SCL -> I2C SCL pin on your board
- VCC -> 3.3V
- GND -> GND

#### SPI TFT
- CS -> GPIO 17
- RST -> GPIO 16
- DC -> GPIO 15
- MOSI -> SPI MOSI pin on your board
- SCK -> SPI SCK pin on your board
- VCC -> 3.3V or 5V (check your display)
- GND -> GND

## Required Libraries

- ADF4351 (included in this project)
- Wire (for I2C communication)
- Encoder by Paul Stoffregen
- Display library (based on your display choice):
  - LiquidCrystal_I2C (for I2C LCD)
  - Adafruit_SSD1306 and Adafruit_GFX (for OLED)
  - Adafruit_ST7735 and Adafruit_GFX (for TFT)

## Display Configuration

The sketch supports three different display types. To select your display type, uncomment only one of these lines at the beginning of the sketch:

```cpp
#define USE_LCD_I2C      // I2C LCD display (16x2 or 20x4)
//#define USE_OLED_I2C     // I2C OLED display (128x64)
//#define USE_TFT_SPI      // SPI TFT display (various sizes)
```

## Usage

1. Select your display type by uncommenting the appropriate #define
2. Upload the sketch to your Arduino-compatible board
3. Use the rotary encoder to tune the frequency
4. Press the band button to cycle through ham radio bands
5. Press the step button to change the tuning step size
6. Press the function button to cycle through power levels
7. Press the encoder button to toggle RF output on/off

## Controls

- **Rotary Encoder**: Turn clockwise to increase frequency, counter-clockwise to decrease
- **Encoder Button**: Toggle RF output on/off
- **Band Button**: Cycle through ham radio bands
- **Step Button**: Cycle through step sizes (10 Hz, 100 Hz, 1 kHz, 10 kHz, 100 kHz, 1 MHz)
- **Function Button**: Cycle through power levels (0-3, corresponding to -4 dBm to +5 dBm)

## Display Information

The display shows:
- Current frequency with appropriate units (kHz, MHz, or GHz)
- Current ham band
- Step size
- PLL lock status
- Power level
- RF output status (ON/OFF)
