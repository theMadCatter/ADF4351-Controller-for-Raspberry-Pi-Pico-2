# ADF4351 Controller for Raspberry Pi Pico 2

This project provides a controller for the ADF4351 wideband frequency synthesizer using the Raspberry Pi Pico 2 microcontroller. It's designed for ham radio, SDR (Software Defined Radio), and other RF experimentation applications.

## Features

- Control the ADF4351 frequency synthesizer via SPI
- Set frequencies from 35 MHz to 4.4 GHz
- Serial interface for easy frequency control
- Optimized for Raspberry Pi Pico 2 but adaptable to other microcontrollers

## Hardware Requirements

- Raspberry Pi Pico 2
- ADF4351 module or breakout board
- 25 MHz reference oscillator (default on most boards, can be adjusted in code)
- Connecting wires
- Optional: Amplifier, filters, and antenna for RF output

## Wiring Connections

| ADF4351 Pin | Raspberry Pi Pico 2 Pin |
|-------------|-------------------------|
| LE (Latch Enable) | GPIO 5 |
| CLK (Clock) | GPIO 2 |
| DATA (Data) | GPIO 3 |
| CE (Chip Enable) | GPIO 4 |
| VCC | 3.3V |
| GND | GND |

## Software Setup

1. Install the Arduino IDE
2. Add Raspberry Pi Pico 2 board support to Arduino IDE
   - In Arduino IDE, go to File > Preferences
   - Add this URL to the "Additional Boards Manager URLs" field:
     `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
   - Go to Tools > Board > Boards Manager
   - Search for "Raspberry Pi Pico" and install the package
3. Select "Raspberry Pi Pico" from the Tools > Board menu
4. Upload the sketch to your Pico

## Usage

1. Connect your hardware according to the wiring diagram
2. Upload the sketch to the Raspberry Pi Pico 2
3. Open the Serial Monitor at 115200 baud
4. Enter the desired frequency in Hz (e.g., 145000000 for 145 MHz)
5. The ADF4351 will be programmed to output the requested frequency

## Advanced Usage

The project includes several example sketches to demonstrate different use cases:

### Frequency Sweep
A utility for sweeping through a range of frequencies, useful for testing filters and RF components.

### Ham Band Signal Generator
A preset-based signal generator for common amateur radio bands.

### SDR Local Oscillator
A stable local oscillator for software-defined radio applications.

### VFO Interface
A complete Variable Frequency Oscillator interface with:
- Rotary encoder for frequency tuning with debouncing
- Support for multiple display types (LCD, OLED, TFT)
- Band selection and step size control
- RF output and power level control

## Repository Structure

```
ADF4351_Controller/
├── ADF4351.cpp                # Core library implementation
├── ADF4351.h                  # Library header file
├── ADF4351_Controller.ino     # Main controller sketch
├── README.md                  # This file
└── Examples/                  # Example applications
    ├── FrequencySweep/        # Frequency sweep utility
    ├── HamBandSignalGenerator/# Ham band signal generator
    ├── SDR_LocalOscillator/   # SDR local oscillator
    └── VFO_Interface/         # VFO interface with display support
```

## Contributing

Contributions to improve the project are welcome. Please feel free to fork the repository, make changes, and submit pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Thanks to Analog Devices for the ADF4351 documentation
- The Raspberry Pi Foundation for the Pico 2 microcontroller
- All contributors to the Arduino libraries used in this project
