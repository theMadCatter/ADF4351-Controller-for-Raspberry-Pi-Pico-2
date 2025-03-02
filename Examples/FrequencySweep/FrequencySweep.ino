/*
 * ADF4351 Frequency Sweep Example
 * 
 * This example demonstrates how to perform a frequency sweep with the ADF4351
 * which is useful for testing filters, antennas, and other RF components.
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

// Sweep parameters
uint32_t startFreq = 100000000;  // 100 MHz
uint32_t stopFreq = 200000000;   // 200 MHz
uint32_t stepSize = 1000000;     // 1 MHz
uint32_t dwellTime = 100;        // 100 ms per frequency

// Sweep state
uint32_t currentFreq = 0;
bool sweepRunning = false;
unsigned long lastStepTime = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println("ADF4351 Frequency Sweep Example");
  Serial.println("-------------------------------");
  
  // Initialize ADF4351
  adf4351.begin(REF_FREQ);
  
  // Set initial frequency
  adf4351.setFrequency(startFreq);
  currentFreq = startFreq;
  
  Serial.println("ADF4351 initialized");
  printHelp();
}

void loop() {
  // Process serial commands
  processSerial();
  
  // Handle frequency sweep if running
  if (sweepRunning) {
    unsigned long currentTime = millis();
    
    // Check if it's time to step to the next frequency
    if (currentTime - lastStepTime >= dwellTime) {
      // Move to next frequency
      currentFreq += stepSize;
      
      // Check if we've reached the end of the sweep
      if (currentFreq > stopFreq) {
        currentFreq = startFreq; // Restart sweep
        Serial.println("Sweep cycle complete, restarting");
      }
      
      // Set the new frequency
      adf4351.setFrequency(currentFreq);
      
      // Print current frequency (only every 10 steps to avoid flooding serial)
      if ((currentFreq - startFreq) % (stepSize * 10) == 0) {
        Serial.print("Frequency: ");
        Serial.print(currentFreq / 1000000.0, 3);
        Serial.println(" MHz");
      }
      
      // Update last step time
      lastStepTime = currentTime;
    }
  }
}

void processSerial() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    if (command == "start") {
      // Start the sweep
      sweepRunning = true;
      currentFreq = startFreq;
      adf4351.setFrequency(currentFreq);
      lastStepTime = millis();
      Serial.println("Sweep started");
      
      // Print sweep parameters
      printSweepParams();
    }
    else if (command == "stop") {
      // Stop the sweep
      sweepRunning = false;
      Serial.println("Sweep stopped");
    }
    else if (command.startsWith("start ")) {
      // Set start frequency: "start 100"
      String freqStr = command.substring(6);
      uint32_t freq = (uint32_t)(freqStr.toFloat() * 1000000);
      
      if (freq >= 35000000 && freq <= 4400000000UL && freq < stopFreq) {
        startFreq = freq;
        currentFreq = startFreq;
        Serial.print("Start frequency set to: ");
        Serial.print(startFreq / 1000000.0, 3);
        Serial.println(" MHz");
      } else {
        Serial.println("Error: Invalid start frequency");
      }
    }
    else if (command.startsWith("stop ")) {
      // Set stop frequency: "stop 200"
      String freqStr = command.substring(5);
      uint32_t freq = (uint32_t)(freqStr.toFloat() * 1000000);
      
      if (freq >= 35000000 && freq <= 4400000000UL && freq > startFreq) {
        stopFreq = freq;
        Serial.print("Stop frequency set to: ");
        Serial.print(stopFreq / 1000000.0, 3);
        Serial.println(" MHz");
      } else {
        Serial.println("Error: Invalid stop frequency");
      }
    }
    else if (command.startsWith("step ")) {
      // Set step size: "step 1"
      String stepStr = command.substring(5);
      uint32_t step = (uint32_t)(stepStr.toFloat() * 1000000);
      
      if (step > 0 && step <= (stopFreq - startFreq)) {
        stepSize = step;
        Serial.print("Step size set to: ");
        Serial.print(stepSize / 1000000.0, 3);
        Serial.println(" MHz");
      } else {
        Serial.println("Error: Invalid step size");
      }
    }
    else if (command.startsWith("dwell ")) {
      // Set dwell time: "dwell 100"
      String dwellStr = command.substring(6);
      uint32_t dwell = dwellStr.toInt();
      
      if (dwell > 0 && dwell <= 10000) {
        dwellTime = dwell;
        Serial.print("Dwell time set to: ");
        Serial.print(dwellTime);
        Serial.println(" ms");
      } else {
        Serial.println("Error: Invalid dwell time (1-10000 ms)");
      }
    }
    else if (command == "params") {
      // Print sweep parameters
      printSweepParams();
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
}

void printSweepParams() {
  Serial.println("\nSweep Parameters:");
  Serial.println("-----------------");
  Serial.print("Start Frequency: ");
  Serial.print(startFreq / 1000000.0, 3);
  Serial.println(" MHz");
  
  Serial.print("Stop Frequency: ");
  Serial.print(stopFreq / 1000000.0, 3);
  Serial.println(" MHz");
  
  Serial.print("Step Size: ");
  Serial.print(stepSize / 1000000.0, 3);
  Serial.println(" MHz");
  
  Serial.print("Dwell Time: ");
  Serial.print(dwellTime);
  Serial.println(" ms");
  
  Serial.print("Total Steps: ");
  Serial.println((stopFreq - startFreq) / stepSize + 1);
  
  Serial.print("Sweep Time: ");
  Serial.print(((stopFreq - startFreq) / stepSize + 1) * dwellTime / 1000.0, 2);
  Serial.println(" seconds");
  
  Serial.println();
}

void printHelp() {
  Serial.println("\nAvailable Commands:");
  Serial.println("------------------");
  Serial.println("start       - Start frequency sweep");
  Serial.println("stop        - Stop frequency sweep");
  Serial.println("start <MHz> - Set start frequency in MHz");
  Serial.println("stop <MHz>  - Set stop frequency in MHz");
  Serial.println("step <MHz>  - Set step size in MHz");
  Serial.println("dwell <ms>  - Set dwell time in milliseconds");
  Serial.println("params      - Display current sweep parameters");
  Serial.println("help        - Display this help message");
  Serial.println("\nExample: start 100");
  Serial.println();
}
