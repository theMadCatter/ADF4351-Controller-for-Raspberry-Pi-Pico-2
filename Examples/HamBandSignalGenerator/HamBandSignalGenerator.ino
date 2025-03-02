/*
 * ADF4351 Ham Band Signal Generator
 * 
 * This example demonstrates how to use the ADF4351 as a signal generator
 * for ham radio bands. It includes presets for common ham bands and
 * allows for fine-tuning of the frequency.
 * 
 * Created: March 2025
 */

#include "ADF4351.h"

// Pin definitions
#define ADF4351_LE_PIN   5  // Latch Enable Pin
#define ADF4351_CLK_PIN  2  // Clock Pin
#define ADF4351_DATA_PIN 3  // Data Pin
#define ADF4351_CE_PIN   4  // Chip Enable Pin

// Reference frequency (Hz)
const uint32_t REF_FREQ = 25000000; // 25 MHz reference

// Create ADF4351 instance
ADF4351 adf4351(ADF4351_LE_PIN, ADF4351_CLK_PIN, ADF4351_DATA_PIN, ADF4351_CE_PIN);

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
const int NUM_STEPS = sizeof(stepSizes) / sizeof(stepSizes[0]);
int currentStepIndex = 2; // Default to 1 kHz steps

// Current frequency
uint32_t currentFrequency = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("ADF4351 Ham Band Signal Generator");
  Serial.println("--------------------------------");
  
  // Initialize ADF4351
  adf4351.begin(REF_FREQ);
  
  // Set initial band
  setBand(currentBandIndex);
  
  Serial.println("ADF4351 initialized");
  printHelp();
}

void loop() {
  // Process serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    processCommand(command);
  }
}

void processCommand(String command) {
  if (command == "next") {
    // Next band
    currentBandIndex = (currentBandIndex + 1) % NUM_BANDS;
    setBand(currentBandIndex);
  }
  else if (command == "prev") {
    // Previous band
    currentBandIndex = (currentBandIndex - 1 + NUM_BANDS) % NUM_BANDS;
    setBand(currentBandIndex);
  }
  else if (command == "up") {
    // Increase frequency by current step size
    setFrequency(currentFrequency + stepSizes[currentStepIndex]);
  }
  else if (command == "down") {
    // Decrease frequency by current step size
    setFrequency(currentFrequency - stepSizes[currentStepIndex]);
  }
  else if (command == "step") {
    // Cycle through step sizes
    currentStepIndex = (currentStepIndex + 1) % NUM_STEPS;
    Serial.print("Step size: ");
    printFrequency(stepSizes[currentStepIndex]);
    Serial.println();
  }
  else if (command.startsWith("band ")) {
    // Set band by name: "band 2m"
    String bandName = command.substring(5);
    
    for (int i = 0; i < NUM_BANDS; i++) {
      if (bandName == hamBands[i].name) {
        currentBandIndex = i;
        setBand(currentBandIndex);
        return;
      }
    }
    
    Serial.println("Error: Unknown band name");
    printBands();
  }
  else if (command.startsWith("freq ")) {
    // Set frequency directly: "freq 145.500"
    String freqStr = command.substring(5);
    float freqMHz = freqStr.toFloat();
    uint32_t frequency = (uint32_t)(freqMHz * 1000000);
    
    if (frequency >= 35000000 && frequency <= 4400000000UL) {
      setFrequency(frequency);
    } else {
      Serial.println("Error: Frequency out of range (35 MHz to 4.4 GHz)");
    }
  }
  else if (command == "power") {
    // Cycle through power levels
    static uint8_t powerLevel = 3;
    powerLevel = (powerLevel + 1) % 4;
    
    adf4351.setPowerLevel(powerLevel);
    
    Serial.print("Power level: ");
    Serial.print(powerLevel);
    Serial.print(" (");
    Serial.print(-4 + (powerLevel * 3));
    Serial.println(" dBm)");
  }
  else if (command == "bands") {
    // List all bands
    printBands();
  }
  else if (command == "status") {
    // Print current status
    printStatus();
  }
  else if (command == "help") {
    // Print help
    printHelp();
  }
  else {
    // Unknown command
    Serial.println("Unknown command. Type 'help' for available commands.");
  }
}

void setBand(int bandIndex) {
  // Set frequency to the selected band
  setFrequency(hamBands[bandIndex].frequency);
  
  Serial.print("Band: ");
  Serial.print(hamBands[bandIndex].name);
  Serial.print(" (");
  printFrequency(hamBands[bandIndex].frequency);
  Serial.println(")");
}

void setFrequency(uint32_t frequency) {
  // Check if frequency is within range
  if (frequency < 35000000) {
    frequency = 35000000;
    Serial.println("Frequency limited to 35 MHz minimum");
  } else if (frequency > 4400000000UL) {
    frequency = 4400000000UL;
    Serial.println("Frequency limited to 4.4 GHz maximum");
  }
  
  // Set the frequency
  adf4351.setFrequency(frequency);
  currentFrequency = frequency;
  
  // Print the frequency
  Serial.print("Frequency: ");
  printFrequency(frequency);
  Serial.println();
}

void printFrequency(uint32_t frequency) {
  // Print frequency in appropriate units
  if (frequency < 1000000) {
    // Less than 1 MHz, print in kHz
    Serial.print(frequency / 1000.0, 3);
    Serial.print(" kHz");
  } else if (frequency < 1000000000UL) {
    // Less than 1 GHz, print in MHz
    Serial.print(frequency / 1000000.0, 6);
    Serial.print(" MHz");
  } else {
    // 1 GHz or more, print in GHz
    Serial.print(frequency / 1000000000.0, 6);
    Serial.print(" GHz");
  }
}

void printBands() {
  Serial.println("\nAvailable Ham Bands:");
  Serial.println("-------------------");
  
  for (int i = 0; i < NUM_BANDS; i++) {
    Serial.print(hamBands[i].name);
    Serial.print(": ");
    printFrequency(hamBands[i].frequency);
    Serial.println();
  }
  
  Serial.println();
}

void printStatus() {
  Serial.println("\nCurrent Status:");
  Serial.println("--------------");
  
  Serial.print("Band: ");
  Serial.println(hamBands[currentBandIndex].name);
  
  Serial.print("Frequency: ");
  printFrequency(currentFrequency);
  Serial.println();
  
  Serial.print("Step Size: ");
  printFrequency(stepSizes[currentStepIndex]);
  Serial.println();
  
  Serial.print("PLL Lock: ");
  Serial.println(adf4351.isLocked() ? "Locked" : "Unlocked");
  
  Serial.println();
}

void printHelp() {
  Serial.println("\nAvailable Commands:");
  Serial.println("------------------");
  Serial.println("next        - Go to next ham band");
  Serial.println("prev        - Go to previous ham band");
  Serial.println("up          - Increase frequency by current step size");
  Serial.println("down        - Decrease frequency by current step size");
  Serial.println("step        - Cycle through step sizes");
  Serial.println("band <name> - Set band by name (e.g., 'band 2m')");
  Serial.println("freq <MHz>  - Set frequency directly in MHz");
  Serial.println("power       - Cycle through power levels");
  Serial.println("bands       - List all available ham bands");
  Serial.println("status      - Display current status");
  Serial.println("help        - Display this help message");
  Serial.println("\nExample: freq 145.500");
  Serial.println();
}
