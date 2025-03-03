# Jetson I2C Communication Script

## Overview
This script facilitates communication between the Jetson AGX Orin and the MSP430 microcontroller via I2C. It is designed to receive water leak detection data and trigger appropriate responses on the Jetson system.

## Features
- Establishes an I2C connection with the MSP430.
- Reads sensor data transmitted by the leak detection system.
- Logs incoming data for debugging and future integration.
- Open for expansion to support additional commands and processing logic.

## Requirements
- **Jetson AGX Orin** running Ubuntu.
- **Python 3** with the following dependencies:
  ```sh
  pip install smbus2
  ```
- **I2C enabled** on the Jetson (ensure `/dev/i2c-*` is available).

## Usage
Run the script to start listening for data:
```sh
python3 i2c_communication.py
```

## Future Development
- Implement data validation and error handling.
- Add integration with the RoboSub’s main control software.
- Expand logging capabilities for debugging and analytics.

## Contributors
- Jonathon Hughes

