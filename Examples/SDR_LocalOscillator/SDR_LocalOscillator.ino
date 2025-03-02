/*
 * ADF4351 SDR Local Oscillator
 * 
 * This example demonstrates how to use the ADF4351 as a local oscillator
 * for software-defined radio applications. It includes features for
 * frequency control, IF offset calculation, and band selection.
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

// SDR parameters
uint32_t targetFrequency = 145000000;  // Target frequency (145 MHz)
uint32_t ifOffset = 10700000;          // IF offset (10.7 MHz)
bool highSideInjection = true;         // High-side injection (LO > RF)

// Common SDR bands
struct SdrBand {
  const char* name;
  uint32_t startFreq;
  uint32_t endFreq;
};

// Array of common SDR bands
const SdrBand sdrBands[] = {
  {"AM Broadcast", 530000, 1700000},         // AM Broadcast
  {"Shortwave", 2300000, 26100000},          // Shortwave
  {"CB Radio", 26965000, 27405000},          // CB Radio
  {"10m Ham", 28000000, 29700000},           // 10m Amateur Radio
  {"FM Broadcast", 88000000, 108000000},     // FM Broadcast
  {"Aircraft", 118000000, 137000000},        // Aircraft Band
  {"2m Ham", 144000000, 148000000},          // 2m Amateur Radio
  {"Weather", 162400000, 162550000},         // NOAA Weather Radio
  {"70cm Ham", 420000000, 450000000},        // 70cm Amateur Radio
  {"ISM 915", 902000000, 928000000},         // ISM 915 MHz
  {"ADS-B", 1090000000, 1090000000},         // ADS-B Aircraft Tracking
  {"L-Band Satellite", 1525000000, 1559000000}, // L-Band Satellite
  {"S-Band", 2400000000UL, 2500000000UL}     // S-Band
};

const int NUM_BANDS = sizeof(sdrBands) / sizeof(sdrBands[0]);
int currentBandIndex = 6; // Default to 2m Ham

// Tuning step sizes (in Hz)
const uint32_t stepSizes[] = {100, 1000, 5000, 12500, 25000, 100000, 1000000};
const int NUM_STEPS = sizeof(stepSizes) / sizeof(stepSizes[0]);
int currentStepIndex = 2; // Default to 5 kHz steps

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("ADF4351 SDR Local Oscillator");
  Serial.println("---------------------------");
  
  // Initialize ADF4351
  adf4351.begin(REF_FREQ);
  
  // Set low spur mode for better SDR performance
  adf4351.setLowNoiseMode(false);
  
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
    setTargetFrequency(targetFrequency + stepSizes[currentStepIndex]);
  }
  else if (command == "down") {
    // Decrease frequency by current step size
    setTargetFrequency(targetFrequency - stepSizes[currentStepIndex]);
  }
  else if (command == "step") {
    // Cycle through step sizes
    currentStepIndex = (currentStepIndex + 1) % NUM_STEPS;
    Serial.print("Step size: ");
    printFrequency(stepSizes[currentStepIndex]);
    Serial.println();
  }
  else if (command.startsWith("band ")) {
    // Set band by index: "band 6"
    int bandIndex = command.substring(5).toInt();
    
    if (bandIndex >= 0 && bandIndex < NUM_BANDS) {
      currentBandIndex = bandIndex;
      setBand(currentBandIndex);
    } else {
      Serial.println("Error: Invalid band index");
      printBands();
    }
  }
  else if (command.startsWith("freq ")) {
    // Set target frequency directly: "freq 145.500"
    String freqStr = command.substring(5);
    float freqMHz = freqStr.toFloat();
    uint32_t frequency = (uint32_t)(freqMHz * 1000000);
    
    setTargetFrequency(frequency);
  }
  else if (command.startsWith("if ")) {
    // Set IF offset: "if 10.7"
    String ifStr = command.substring(3);
    float ifMHz = ifStr.toFloat();
    ifOffset = (uint32_t)(ifMHz * 1000000);
    
    Serial.print("IF Offset: ");
    printFrequency(ifOffset);
    Serial.println();
    
    // Update LO frequency with new IF offset
    updateLoFrequency();
  }
  else if (command == "injection") {
    // Toggle high/low side injection
    highSideInjection = !highSideInjection;
    
    Serial.print("Injection: ");
    Serial.println(highSideInjection ? "High Side" : "Low Side");
    
    // Update LO frequency with new injection setting
    updateLoFrequency();
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
  // Set frequency to the middle of the selected band
  uint32_t midFreq = (sdrBands[bandIndex].startFreq + sdrBands[bandIndex].endFreq) / 2;
  setTargetFrequency(midFreq);
  
  Serial.print("Band: ");
  Serial.print(sdrBands[bandIndex].name);
  Serial.print(" (");
  printFrequency(sdrBands[bandIndex].startFreq);
  Serial.print(" - ");
  printFrequency(sdrBands[bandIndex].endFreq);
  Serial.println(")");
}

void setTargetFrequency(uint32_t frequency) {
  // Check if frequency is within range for the ADF4351
  if (frequency < 35000000 - ifOffset) {
    frequency = 35000000 - ifOffset;
    Serial.println("Warning: Target frequency limited due to ADF4351 range");
  } else if (frequency > 4400000000UL - ifOffset) {
    frequency = 4400000000UL - ifOffset;
    Serial.println("Warning: Target frequency limited due to ADF4351 range");
  }
  
  // Set the target frequency
  targetFrequency = frequency;
  
  // Update the LO frequency based on target frequency and IF offset
  updateLoFrequency();
  
  // Print the target frequency
  Serial.print("Target Frequency: ");
  printFrequency(targetFrequency);
  Serial.println();
}

void updateLoFrequency() {
  // Calculate LO frequency based on target frequency, IF offset, and injection side
  uint32_t loFrequency;
  
  if (highSideInjection) {
    // High-side injection: LO = RF + IF
    loFrequency = targetFrequency + ifOffset;
  } else {
    // Low-side injection: LO = RF - IF
    loFrequency = targetFrequency - ifOffset;
  }
  
  // Set the LO frequency
  if (loFrequency >= 35000000 && loFrequency <= 4400000000UL) {
    adf4351.setFrequency(loFrequency);
    
    // Print the LO frequency
    Serial.print("LO Frequency: ");
    printFrequency(loFrequency);
    Serial.println();
  } else {
    Serial.println("Error: LO frequency out of range (35 MHz to 4.4 GHz)");
  }
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
  Serial.println("\nAvailable SDR Bands:");
  Serial.println("-------------------");
  
  for (int i = 0; i < NUM_BANDS; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.print(sdrBands[i].name);
    Serial.print(" (");
    printFrequency(sdrBands[i].startFreq);
    Serial.print(" - ");
    printFrequency(sdrBands[i].endFreq);
    Serial.println(")");
  }
  
  Serial.println();
}

void printStatus() {
  Serial.println("\nCurrent Status:");
  Serial.println("--------------");
  
  Serial.print("Band: ");
  Serial.println(sdrBands[currentBandIndex].name);
  
  Serial.print("Target Frequency: ");
  printFrequency(targetFrequency);
  Serial.println();
  
  Serial.print("IF Offset: ");
  printFrequency(ifOffset);
  Serial.println();
  
  Serial.print("Injection: ");
  Serial.println(highSideInjection ? "High Side" : "Low Side");
  
  uint32_t loFrequency = highSideInjection ? 
                         targetFrequency + ifOffset : 
                         targetFrequency - ifOffset;
  
  Serial.print("LO Frequency: ");
  printFrequency(loFrequency);
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
  Serial.println("next        - Go to next SDR band");
  Serial.println("prev        - Go to previous SDR band");
  Serial.println("up          - Increase frequency by current step size");
  Serial.println("down        - Decrease frequency by current step size");
  Serial.println("step        - Cycle through step sizes");
  Serial.println("band <idx>  - Set band by index number");
  Serial.println("freq <MHz>  - Set target frequency directly in MHz");
  Serial.println("if <MHz>    - Set IF offset in MHz");
  Serial.println("injection   - Toggle between high/low side injection");
  Serial.println("bands       - List all available SDR bands");
  Serial.println("status      - Display current status");
  Serial.println("help        - Display this help message");
  Serial.println("\nExample: freq 145.500");
  Serial.println();
}
