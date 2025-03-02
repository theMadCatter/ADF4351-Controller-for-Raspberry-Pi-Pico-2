/*
 * ADF4351 VFO Interface
 * 
 * This example implements a VFO (Variable Frequency Oscillator) interface
 * for the ADF4351 frequency synthesizer using:
 * - Rotary encoder for frequency tuning with debouncing
 * - Support for multiple display types (LCD, OLED, TFT)
 * - Push buttons for band and step size selection
 * 
 * Created: March 2025
 */

#include "ADF4351.h"
#include <Wire.h>

// Uncomment only ONE of these display options
#define USE_LCD_I2C      // I2C LCD display (16x2 or 20x4)
//#define USE_OLED_I2C     // I2C OLED display (128x64)
//#define USE_TFT_SPI      // SPI TFT display (various sizes)

// Include appropriate display library based on selection
#ifdef USE_LCD_I2C
  #include <LiquidCrystal_I2C.h>  // I2C LCD library
  LiquidCrystal_I2C lcd(0x27, 20, 4); // Set the LCD address to 0x27 for a 20 chars and 4 line display
#endif

#ifdef USE_OLED_I2C
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  #define SCREEN_WIDTH 128 // OLED display width, in pixels
  #define SCREEN_HEIGHT 64 // OLED display height, in pixels
  #define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

#ifdef USE_TFT_SPI
  #include <Adafruit_GFX.h>
  #include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
  #define TFT_CS        17    // TFT chip select pin
  #define TFT_RST       16    // TFT reset pin
  #define TFT_DC        15    // TFT data/command pin
  Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#endif

// Encoder library
#include <Encoder.h>

// Pin definitions for ADF4351
#define ADF4351_LE_PIN   5  // Latch Enable Pin
#define ADF4351_CLK_PIN  2  // Clock Pin
#define ADF4351_DATA_PIN 3  // Data Pin
#define ADF4351_CE_PIN   4  // Chip Enable Pin

// Pin definitions for rotary encoder
#define ENCODER_PIN_A    6  // Encoder pin A
#define ENCODER_PIN_B    7  // Encoder pin B
#define ENCODER_BUTTON   8  // Encoder push button

// Pin definitions for additional buttons
#define BAND_BUTTON      9  // Band selection button
#define STEP_BUTTON      10 // Step size selection button
#define FUNC_BUTTON      11 // Function button

// Reference frequency (Hz)
const uint32_t REF_FREQ = 25000000; // 25 MHz reference

// Create ADF4351 instance
ADF4351 adf4351(ADF4351_LE_PIN, ADF4351_CLK_PIN, ADF4351_DATA_PIN, ADF4351_CE_PIN);

// Create Encoder instance
Encoder tuningKnob(ENCODER_PIN_A, ENCODER_PIN_B);

// Ham band presets (in Hz)
struct BandPreset {
  const char* name;
  uint32_t frequency;
};

// Array of ham band presets
const BandPreset hamBands[] = {
  {"160m", 1900000},     // 160 meters (1.8-2.0 MHz)
  {"80m", 3700000},      // 80 meters (3.5-4.0 MHz)
  {"60m", 5357000},      // 60 meters (5.3-5.4 MHz)
  {"40m", 7100000},      // 40 meters (7.0-7.3 MHz)
  {"30m", 10125000},     // 30 meters (10.1-10.15 MHz)
  {"20m", 14200000},     // 20 meters (14.0-14.35 MHz)
  {"17m", 18100000},     // 17 meters (18.068-18.168 MHz)
  {"15m", 21200000},     // 15 meters (21.0-21.45 MHz)
  {"12m", 24930000},     // 12 meters (24.89-24.99 MHz)
  {"10m", 28400000},     // 10 meters (28.0-29.7 MHz)
  {"6m", 50150000},      // 6 meters (50-54 MHz)
  {"2m", 145000000},     // 2 meters (144-148 MHz)
  {"70cm", 435000000},   // 70 centimeters (420-450 MHz)
  {"33cm", 915000000},   // 33 centimeters (902-928 MHz)
  {"23cm", 1270000000},  // 23 centimeters (1240-1300 MHz)
  {"13cm", 2400000000UL} // 13 centimeters (2300-2450 MHz)
};

const int NUM_BANDS = sizeof(hamBands) / sizeof(hamBands[0]);
int currentBandIndex = 11; // Default to 2m band

// Tuning step sizes (in Hz)
const uint32_t stepSizes[] = {10, 100, 1000, 10000, 100000, 1000000};
const char* stepLabels[] = {"10 Hz", "100 Hz", "1 kHz", "10 kHz", "100 kHz", "1 MHz"};
const int NUM_STEPS = sizeof(stepSizes) / sizeof(stepSizes[0]);
int currentStepIndex = 2; // Default to 1 kHz steps

// Current frequency
uint32_t currentFrequency = 145000000; // Start at 145 MHz

// Button debouncing
const unsigned long DEBOUNCE_DELAY = 50; // Debounce time in milliseconds
unsigned long lastBandButtonTime = 0;
unsigned long lastStepButtonTime = 0;
unsigned long lastFuncButtonTime = 0;
unsigned long lastEncoderButtonTime = 0;
bool bandButtonState = HIGH;
bool stepButtonState = HIGH;
bool funcButtonState = HIGH;
bool encoderButtonState = HIGH;

// Encoder previous position
long oldEncoderPosition = 0;

// Display update timing
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 100; // Update display every 100ms

// RF output state
bool rfOutputEnabled = true;

// Power level
uint8_t powerLevel = 3; // Default to +5dBm

// Function prototypes
void initDisplay();
void updateDisplay();
void handleEncoder();
void handleButtons();
void setBand(int bandIndex);
void setFrequency(uint32_t frequency);
void formatFrequency(uint32_t frequency, char* buffer);
void displayStatusLine();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize display
  initDisplay();
  
  // Initialize button pins
  pinMode(ENCODER_BUTTON, INPUT_PULLUP);
  pinMode(BAND_BUTTON, INPUT_PULLUP);
  pinMode(STEP_BUTTON, INPUT_PULLUP);
  pinMode(FUNC_BUTTON, INPUT_PULLUP);
  
  // Initialize ADF4351
  adf4351.begin(REF_FREQ);
  
  // Set initial frequency
  adf4351.setFrequency(currentFrequency);
  
  // Reset encoder position
  tuningKnob.write(0);
  
  // Show initial display
  updateDisplay();
  
  Serial.println("ADF4351 VFO Interface initialized");
}

void loop() {
  // Handle encoder rotation
  handleEncoder();
  
  // Handle button presses
  handleButtons();
  
  // Update display periodically
  unsigned long currentMillis = millis();
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    lastDisplayUpdate = currentMillis;
  }
}

void initDisplay() {
#ifdef USE_LCD_I2C
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ADF4351 VFO");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
#endif

#ifdef USE_OLED_I2C
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ADF4351 VFO"));
  display.println(F("Initializing..."));
  display.display();
#endif

#ifdef USE_TFT_SPI
  tft.initR(INITR_BLACKTAB); // Initialize ST7735S chip
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(0, 0);
  tft.println("ADF4351 VFO");
  tft.println("Initializing...");
#endif
}

void handleEncoder() {
  // Read encoder position
  long newPosition = tuningKnob.read() / 4; // Divide by 4 for smoother tuning
  
  // Check if position has changed
  if (newPosition != oldEncoderPosition) {
    // Calculate frequency change
    long change = newPosition - oldEncoderPosition;
    uint32_t frequencyChange = change * stepSizes[currentStepIndex];
    
    // Update frequency
    if (change > 0) {
      setFrequency(currentFrequency + frequencyChange);
    } else {
      // Prevent underflow
      if (currentFrequency >= (uint32_t)(-frequencyChange)) {
        setFrequency(currentFrequency + frequencyChange);
      } else {
        setFrequency(35000000); // Minimum frequency
      }
    }
    
    // Update old position
    oldEncoderPosition = newPosition;
  }
}

void handleButtons() {
  // Read current button states
  bool bandButtonReading = digitalRead(BAND_BUTTON);
  bool stepButtonReading = digitalRead(STEP_BUTTON);
  bool funcButtonReading = digitalRead(FUNC_BUTTON);
  bool encoderButtonReading = digitalRead(ENCODER_BUTTON);
  
  unsigned long currentMillis = millis();
  
  // Band button debouncing and handling
  if (bandButtonReading != bandButtonState) {
    lastBandButtonTime = currentMillis;
  }
  
  if ((currentMillis - lastBandButtonTime) > DEBOUNCE_DELAY) {
    if (bandButtonReading != bandButtonState) {
      bandButtonState = bandButtonReading;
      
      if (bandButtonState == LOW) {
        // Band button pressed, go to next band
        currentBandIndex = (currentBandIndex + 1) % NUM_BANDS;
        setBand(currentBandIndex);
      }
    }
  }
  
  // Step button debouncing and handling
  if (stepButtonReading != stepButtonState) {
    lastStepButtonTime = currentMillis;
  }
  
  if ((currentMillis - lastStepButtonTime) > DEBOUNCE_DELAY) {
    if (stepButtonReading != stepButtonState) {
      stepButtonState = stepButtonReading;
      
      if (stepButtonState == LOW) {
        // Step button pressed, cycle through step sizes
        currentStepIndex = (currentStepIndex + 1) % NUM_STEPS;
        updateDisplay();
      }
    }
  }
  
  // Function button debouncing and handling
  if (funcButtonReading != funcButtonState) {
    lastFuncButtonTime = currentMillis;
  }
  
  if ((currentMillis - lastFuncButtonTime) > DEBOUNCE_DELAY) {
    if (funcButtonReading != funcButtonState) {
      funcButtonState = funcButtonReading;
      
      if (funcButtonState == LOW) {
        // Function button pressed, cycle through power levels
        powerLevel = (powerLevel + 1) % 4;
        adf4351.setPowerLevel(powerLevel);
        updateDisplay();
      }
    }
  }
  
  // Encoder button debouncing and handling
  if (encoderButtonReading != encoderButtonState) {
    lastEncoderButtonTime = currentMillis;
  }
  
  if ((currentMillis - lastEncoderButtonTime) > DEBOUNCE_DELAY) {
    if (encoderButtonReading != encoderButtonState) {
      encoderButtonState = encoderButtonReading;
      
      if (encoderButtonState == LOW) {
        // Encoder button pressed, toggle RF output
        rfOutputEnabled = !rfOutputEnabled;
        adf4351.enableOutput(rfOutputEnabled);
        updateDisplay();
      }
    }
  }
}

void setBand(int bandIndex) {
  // Set frequency to the selected band
  setFrequency(hamBands[bandIndex].frequency);
  updateDisplay();
}

void setFrequency(uint32_t frequency) {
  // Check if frequency is within range
  if (frequency < 35000000) {
    frequency = 35000000;
  } else if (frequency > 4400000000UL) {
    frequency = 4400000000UL;
  }
  
  // Set the frequency
  if (adf4351.setFrequency(frequency)) {
    currentFrequency = frequency;
  }
}

void updateDisplay() {
  char freqBuffer[21];
  formatFrequency(currentFrequency, freqBuffer);
  
#ifdef USE_LCD_I2C
  // Update LCD display
  lcd.clear();
  
  // Line 1: Frequency
  lcd.setCursor(0, 0);
  lcd.print(freqBuffer);
  
  // Line 2: Band
  lcd.setCursor(0, 1);
  lcd.print("Band: ");
  lcd.print(hamBands[currentBandIndex].name);
  
  // Line 3: Step size and lock status
  lcd.setCursor(0, 2);
  lcd.print("Step: ");
  lcd.print(stepLabels[currentStepIndex]);
  lcd.setCursor(14, 2);
  lcd.print(adf4351.isLocked() ? "LOCK" : "UNLK");
  
  // Line 4: Power and RF output status
  lcd.setCursor(0, 3);
  lcd.print("Pwr:");
  lcd.print(powerLevel);
  lcd.print(" RF:");
  lcd.print(rfOutputEnabled ? "ON " : "OFF");
#endif

#ifdef USE_OLED_I2C
  // Update OLED display
  display.clearDisplay();
  
  // Frequency display (larger text)
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(freqBuffer);
  
  // Other information (smaller text)
  display.setTextSize(1);
  
  // Band
  display.setCursor(0, 20);
  display.print("Band: ");
  display.println(hamBands[currentBandIndex].name);
  
  // Step size
  display.setCursor(0, 30);
  display.print("Step: ");
  display.println(stepLabels[currentStepIndex]);
  
  // Lock status
  display.setCursor(0, 40);
  display.print("Lock: ");
  display.println(adf4351.isLocked() ? "YES" : "NO");
  
  // Power and RF output
  display.setCursor(0, 50);
  display.print("Pwr:");
  display.print(powerLevel);
  display.print(" RF:");
  display.println(rfOutputEnabled ? "ON" : "OFF");
  
  display.display();
#endif

#ifdef USE_TFT_SPI
  // Update TFT display
  tft.fillScreen(ST7735_BLACK);
  
  // Frequency display (larger text)
  tft.setTextSize(2);
  tft.setTextColor(ST7735_YELLOW);
  tft.setCursor(0, 0);
  tft.println(freqBuffer);
  
  // Other information (smaller text)
  tft.setTextSize(1);
  tft.setTextColor(ST7735_WHITE);
  
  // Band
  tft.setCursor(0, 25);
  tft.print("Band: ");
  tft.println(hamBands[currentBandIndex].name);
  
  // Step size
  tft.setCursor(0, 35);
  tft.print("Step: ");
  tft.println(stepLabels[currentStepIndex]);
  
  // Lock status
  tft.setCursor(0, 45);
  tft.print("Lock: ");
  tft.setTextColor(adf4351.isLocked() ? ST7735_GREEN : ST7735_RED);
  tft.println(adf4351.isLocked() ? "YES" : "NO");
  
  // Power level
  tft.setCursor(0, 55);
  tft.setTextColor(ST7735_WHITE);
  tft.print("Power: ");
  tft.print(powerLevel);
  tft.print(" (");
  tft.print(-4 + (powerLevel * 3));
  tft.println(" dBm)");
  
  // RF output status
  tft.setCursor(0, 65);
  tft.print("RF Output: ");
  tft.setTextColor(rfOutputEnabled ? ST7735_GREEN : ST7735_RED);
  tft.println(rfOutputEnabled ? "ON" : "OFF");
#endif
}

void formatFrequency(uint32_t frequency, char* buffer) {
  // Format frequency with appropriate units and spacing
  if (frequency < 1000000) {
    // Less than 1 MHz, display in kHz
    sprintf(buffer, "Freq: %7.3f kHz", frequency / 1000.0);
  } else if (frequency < 1000000000UL) {
    // Less than 1 GHz, display in MHz
    sprintf(buffer, "Freq: %9.6f MHz", frequency / 1000000.0);
  } else {
    // 1 GHz or more, display in GHz
    sprintf(buffer, "Freq: %6.6f GHz", frequency / 1000000000.0);
  }
}
