/*
 * ADF4351 Controller for Raspberry Pi Pico 2
 * 
 * This sketch controls the ADF4351 wideband frequency synthesizer
 * for ham radio, SDR, and other RF experimentation applications.
 * 
 * Connections:
 * ADF4351 LE (Latch Enable) -> Pico GPIO 5
 * ADF4351 CLK (Clock) -> Pico GPIO 2
 * ADF4351 DATA (Data) -> Pico GPIO 3
 * ADF4351 CE (Chip Enable) -> Pico GPIO 4
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

// Command processing variables
String inputBuffer = "";
bool commandComplete = false;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("ADF4351 Controller for Raspberry Pi Pico 2");
  Serial.println("----------------------------------------");
  
  // Initialize ADF4351
  adf4351.begin(REF_FREQ);
  
  // Set initial frequency (100 MHz)
  adf4351.setFrequency(100000000);
  
  Serial.println("ADF4351 initialized");
  printHelp();
}

void loop() {
  // Process serial commands
  while (Serial.available()) {
    char c = Serial.read();
    
    // Process on newline
    if (c == '\n' || c == '\r') {
      if (inputBuffer.length() > 0) {
        processCommand(inputBuffer);
        inputBuffer = "";
      }
    } else {
      // Add character to buffer
      inputBuffer += c;
    }
  }
}

void processCommand(String command) {
  command.trim();
  command.toLowerCase();
  
  // Parse command
  if (command.startsWith("freq ")) {
    // Set frequency command: "freq 145000000"
    String freqStr = command.substring(5);
    uint32_t frequency = freqStr.toInt();
    
    if (frequency >= 35000000 && frequency <= 4400000000UL) {
      Serial.print("Setting frequency to: ");
      Serial.print(frequency);
      Serial.println(" Hz");
      
      if (adf4351.setFrequency(frequency)) {
        Serial.println("Frequency set successfully");
      } else {
        Serial.println("Error: Frequency out of range");
      }
    } else {
      Serial.println("Error: Frequency out of range (35 MHz to 4.4 GHz)");
    }
  } 
  else if (command.startsWith("power ")) {
    // Set power level command: "power 3"
    String powerStr = command.substring(6);
    uint8_t powerLevel = powerStr.toInt();
    
    if (powerLevel <= 3) {
      Serial.print("Setting power level to: ");
      Serial.println(powerLevel);
      adf4351.setPowerLevel(powerLevel);
      
      // Print corresponding dBm value
      int8_t dBm = -4 + (powerLevel * 3);
      Serial.print("Output power: ");
      Serial.print(dBm);
      Serial.println(" dBm");
    } else {
      Serial.println("Error: Power level must be 0-3");
      Serial.println("0: -4dBm, 1: -1dBm, 2: +2dBm, 3: +5dBm");
    }
  }
  else if (command == "on") {
    // Enable output
    Serial.println("Enabling RF output");
    adf4351.enableOutput(true);
  }
  else if (command == "off") {
    // Disable output
    Serial.println("Disabling RF output");
    adf4351.enableOutput(false);
  }
  else if (command.startsWith("phase ")) {
    // Set phase command: "phase 90"
    String phaseStr = command.substring(6);
    uint16_t phase = phaseStr.toInt();
    
    if (phase <= 4095) {
      Serial.print("Setting phase to: ");
      Serial.println(phase);
      adf4351.setPhase(phase);
    } else {
      Serial.println("Error: Phase must be 0-4095");
    }
  }
  else if (command == "lownoise") {
    // Set low noise mode
    Serial.println("Setting low noise mode");
    adf4351.setLowNoiseMode(true);
  }
  else if (command == "lowspur") {
    // Set low spur mode
    Serial.println("Setting low spur mode");
    adf4351.setLowNoiseMode(false);
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

void printStatus() {
  Serial.println("\nADF4351 Status:");
  Serial.println("----------------");
  
  // Print current frequency
  Serial.print("Frequency: ");
  Serial.print(adf4351.getFrequency());
  Serial.println(" Hz");
  
  // Print lock status
  Serial.print("PLL Lock: ");
  Serial.println(adf4351.isLocked() ? "Locked" : "Unlocked");
  
  Serial.println();
}

void printHelp() {
  Serial.println("\nAvailable Commands:");
  Serial.println("------------------");
  Serial.println("freq <Hz>    - Set frequency in Hz (35MHz to 4.4GHz)");
  Serial.println("power <0-3>  - Set output power (0:-4dBm, 1:-1dBm, 2:+2dBm, 3:+5dBm)");
  Serial.println("on           - Enable RF output");
  Serial.println("off          - Disable RF output");
  Serial.println("phase <0-4095> - Set phase value");
  Serial.println("lownoise     - Set low noise mode");
  Serial.println("lowspur      - Set low spur mode");
  Serial.println("status       - Display current status");
  Serial.println("help         - Display this help message");
  Serial.println("\nExample: freq 145000000");
  Serial.println();
}
