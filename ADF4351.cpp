/*
 * ADF4351.cpp - Library for controlling the ADF4351 wideband frequency synthesizer
 * 
 * Implementation file for the ADF4351 library.
 * 
 * Created: March 2025
 */

#include "ADF4351.h"

// Constructor
ADF4351::ADF4351(uint8_t le_pin, uint8_t clk_pin, uint8_t data_pin, uint8_t ce_pin) {
  _le_pin = le_pin;
  _clk_pin = clk_pin;
  _data_pin = data_pin;
  _ce_pin = ce_pin;
  
  _frequency = 0;
  _powerLevel = 3;  // Default to +5dBm
  _outputEnabled = true;
  _lowNoiseMode = true;
  
  // Initialize registers to 0
  for (int i = 0; i < 6; i++) {
    _registers[i] = 0;
  }
}

// Initialize the ADF4351
void ADF4351::begin(uint32_t refFreq) {
  // Store reference frequency
  _refFreq = refFreq;
  
  // Set up pins
  pinMode(_le_pin, OUTPUT);
  pinMode(_clk_pin, OUTPUT);
  pinMode(_data_pin, OUTPUT);
  pinMode(_ce_pin, OUTPUT);
  
  // Set initial pin states
  digitalWrite(_le_pin, HIGH);
  digitalWrite(_clk_pin, LOW);
  digitalWrite(_data_pin, LOW);
  digitalWrite(_ce_pin, HIGH); // Enable the chip
  
  // Initial register values
  
  // Register 0: RF Output Frequency Setting
  _registers[0] = 0x00580000; // INT=88, FRAC=0
  
  // Register 1: Phase value, Modulus value
  _registers[1] = 0x08008011; // Phase=1, Prescaler=8/9, MOD=1
  
  // Register 2: Low-noise and low-spur modes
  _registers[2] = 0x00004E42; // Low-noise mode, MUXOUT=Digital Lock Detect, R=1
  
  // Register 3: Output power, charge pump settings
  _registers[3] = 0x000004B3; // Output power = +5dBm
  
  // Register 4: VCO band selection, RF divider
  _registers[4] = 0x00800024; // RF Divider=16, Band Select Clock Divider=40
  
  // Register 5: LD pin mode
  _registers[5] = 0x00580005; // Lock Detect Pin Mode = Digital Lock Detect
  
  // Write all registers (in reverse order 5 to 0)
  for (int i = 5; i >= 0; i--) {
    writeRegister(_registers[i]);
  }
  
  // Set a default frequency (100 MHz)
  setFrequency(100000000);
}

// Set output frequency in Hz
bool ADF4351::setFrequency(uint32_t frequency) {
  // Check if frequency is within range
  if (frequency < 35000000 || frequency > 4400000000UL) {
    return false;
  }
  
  _frequency = frequency;
  updateRegisters();
  
  // Write all registers (in reverse order 5 to 0)
  for (int i = 5; i >= 0; i--) {
    writeRegister(_registers[i]);
  }
  
  return true;
}

// Set output power level (0-3)
void ADF4351::setPowerLevel(uint8_t level) {
  if (level > 3) level = 3;
  
  _powerLevel = level;
  
  // Update Register 3 with new power level
  // Clear the power level bits (bits 3-4)
  _registers[3] &= ~(0x03 << 3);
  // Set the new power level
  _registers[3] |= (_powerLevel << 3);
  
  // Write Register 3
  writeRegister(_registers[3]);
}

// Enable/disable output
void ADF4351::enableOutput(bool enable) {
  _outputEnabled = enable;
  
  // Update Register 4 with output enable/disable
  if (enable) {
    // Clear the RF output enable bit (bit 5)
    _registers[4] &= ~(1 << 5);
  } else {
    // Set the RF output enable bit (bit 5) to disable output
    _registers[4] |= (1 << 5);
  }
  
  // Write Register 4
  writeRegister(_registers[4]);
}

// Set phase value (0-4095)
void ADF4351::setPhase(uint16_t phase) {
  if (phase > 4095) phase = 4095;
  
  // Update Register 1 with new phase value
  // Clear the phase value bits (bits 15-26)
  _registers[1] &= ~(0xFFF << 15);
  // Set the new phase value
  _registers[1] |= (phase << 15);
  
  // Write Register 1
  writeRegister(_registers[1]);
}

// Set low noise or low spur mode
void ADF4351::setLowNoiseMode(bool lowNoise) {
  _lowNoiseMode = lowNoise;
  
  // Update Register 2 with low noise/spur mode
  if (lowNoise) {
    // Set the low noise mode bit (bit 21)
    _registers[2] |= (1 << 21);
    // Clear the low spur mode bit (bit 20)
    _registers[2] &= ~(1 << 20);
  } else {
    // Clear the low noise mode bit (bit 21)
    _registers[2] &= ~(1 << 21);
    // Set the low spur mode bit (bit 20)
    _registers[2] |= (1 << 20);
  }
  
  // Write Register 2
  writeRegister(_registers[2]);
}

// Get current frequency
uint32_t ADF4351::getFrequency() {
  return _frequency;
}

// Get lock status (assuming MUXOUT is set to digital lock detect)
bool ADF4351::isLocked() {
  // This would require an additional pin to read the MUXOUT pin
  // For now, we'll just return true
  return true;
}

// Private method to write a register value to the ADF4351
void ADF4351::writeRegister(uint32_t value) {
  // Pull LE low to begin the transfer
  digitalWrite(_le_pin, LOW);
  
  // Send 32 bits, MSB first
  for (int i = 31; i >= 0; i--) {
    // Set DATA pin based on bit value
    digitalWrite(_data_pin, (value >> i) & 0x01);
    
    // Pulse the clock
    digitalWrite(_clk_pin, HIGH);
    delayMicroseconds(1);
    digitalWrite(_clk_pin, LOW);
    delayMicroseconds(1);
  }
  
  // Pull LE high to latch the data
  digitalWrite(_le_pin, HIGH);
  delayMicroseconds(1);
}

// Private method to update register values based on current settings
void ADF4351::updateRegisters() {
  // Calculate RF divider
  uint8_t divider = calculateRFDivider(_frequency);
  uint8_t rfDivider = 1 << divider;
  
  // Calculate VCO frequency
  uint32_t vcoFreq = _frequency * rfDivider;
  
  // Calculate PFD frequency (Phase Frequency Detector)
  // For simplicity, we'll use R=1 (Reference Divider)
  uint32_t refDivider = 1;
  uint32_t pfdFreq = _refFreq / refDivider;
  
  // Calculate INT and FRAC values
  // INT is the integer division of VCO frequency by PFD frequency
  // FRAC is the fractional part, represented as FRAC/MOD
  uint16_t mod = 1000; // Modulus value
  uint16_t intValue = vcoFreq / pfdFreq;
  uint32_t fracValue = (uint32_t)((((double)vcoFreq / (double)pfdFreq) - intValue) * mod);
  
  // Update registers with calculated values
  
  // Register 0: INT, FRAC
  _registers[0] = (intValue << 15) | (fracValue << 3) | 0;
  
  // Register 1: Phase, Prescaler, MOD
  _registers[1] = (0 << 15) | (1 << 27) | (mod << 3) | 1;
  
  // Register 2: Low-noise mode, MUXOUT, R-counter
  _registers[2] = (_lowNoiseMode ? (1 << 21) : (1 << 20)) | (6 << 9) | (refDivider << 14) | 2;
  
  // Register 3: Output power, charge pump settings
  _registers[3] = (_powerLevel << 3) | (3 << 10) | 3;
  
  // Register 4: RF divider, band select clock divider, VCO settings
  uint8_t bandSelectClockDiv = 200; // Suitable for most applications
  _registers[4] = (divider << 20) | (_outputEnabled ? 0 : (1 << 5)) | (bandSelectClockDiv << 12) | 4;
  
  // Register 5: LD pin mode
  _registers[5] = (1 << 22) | 5; // Digital lock detect
}

// Calculate RF divider value based on frequency
uint8_t ADF4351::calculateRFDivider(uint32_t frequency) {
  if (frequency < 68750000) {
    return 6; // Divide by 64
  } else if (frequency < 137500000) {
    return 5; // Divide by 32
  } else if (frequency < 275000000) {
    return 4; // Divide by 16
  } else if (frequency < 550000000) {
    return 3; // Divide by 8
  } else if (frequency < 1100000000) {
    return 2; // Divide by 4
  } else if (frequency < 2200000000UL) {
    return 1; // Divide by 2
  } else {
    return 0; // Divide by 1
  }
}
