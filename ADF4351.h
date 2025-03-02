/*
 * ADF4351.h - Library for controlling the ADF4351 wideband frequency synthesizer
 * 
 * This library provides a simple interface for controlling the ADF4351 chip
 * with the Raspberry Pi Pico 2 or other Arduino-compatible microcontrollers.
 * 
 * Created: March 2025
 */

#ifndef ADF4351_H
#define ADF4351_H

#include <Arduino.h>

class ADF4351 {
  public:
    // Constructor
    ADF4351(uint8_t le_pin, uint8_t clk_pin, uint8_t data_pin, uint8_t ce_pin);
    
    // Initialize the ADF4351
    void begin(uint32_t refFreq = 25000000);
    
    // Set output frequency in Hz
    bool setFrequency(uint32_t frequency);
    
    // Set output power level (0-3)
    // 0: -4dBm, 1: -1dBm, 2: +2dBm, 3: +5dBm
    void setPowerLevel(uint8_t level);
    
    // Enable/disable output
    void enableOutput(bool enable);
    
    // Set phase value (0-4095)
    void setPhase(uint16_t phase);
    
    // Set low noise or low spur mode
    // true = low noise mode, false = low spur mode
    void setLowNoiseMode(bool lowNoise);
    
    // Get current frequency
    uint32_t getFrequency();
    
    // Get lock status
    bool isLocked();
    
  private:
    // Pin definitions
    uint8_t _le_pin;   // Latch Enable Pin
    uint8_t _clk_pin;  // Clock Pin
    uint8_t _data_pin; // Data Pin
    uint8_t _ce_pin;   // Chip Enable Pin
    
    // Current settings
    uint32_t _frequency;    // Current frequency in Hz
    uint32_t _refFreq;      // Reference frequency in Hz
    uint8_t _powerLevel;    // Output power level (0-3)
    bool _outputEnabled;    // Output state
    bool _lowNoiseMode;     // Low noise mode state
    
    // Register values
    uint32_t _registers[6]; // 6 registers, 32 bits each
    
    // Private methods
    void writeRegister(uint32_t value);
    void updateRegisters();
    uint8_t calculateRFDivider(uint32_t frequency);
};

#endif
