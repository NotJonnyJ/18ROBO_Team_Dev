# RoboSub Leak Detector

## Overview
The RoboSub Leak Detector is a senior capstone project developed for Montana State University's RoboCats robotics team. The project aims to prevent catastrophic damage to the RoboSub's internal electronics by detecting water leaks early and triggering appropriate responses.

## Project Motivation
During previous RoboNation RoboSub competitions, the RoboCats' autonomous submarine suffered from water leaks, leading to costly damage to critical components. This project was developed to mitigate such risks by implementing a reliable water leak detection and alert system.

## Features
- **Multi-Level Water Detection**: Detects different levels of water accumulation using continuity and optical prism-type sensors.
- **Visual Alert System**: Uses high-power LEDs to provide clear underwater visibility of the alert status.
- **Microcontroller Integration**: Interfaces with an MSP430FR2355 microcontroller to process sensor data and trigger responses.
- **Failsafe Mechanism**: Initiates an automatic shutdown when critical water levels are detected.
- **Low Power Consumption**: Optimized to operate efficiently within the RoboSub's power constraints.

## System Design
The system is built with the following components:
- **MSP430FR2355 Microcontroller** for data processing and communication.
- **Continuity Water Sensors** to detect initial water presence.
- **Optical Prism Water Sensors** for precise water level detection.
- **High-Power LED Indicator** (XMLDCL-00-R250-00C5AAA02) for real-time visual status.
- **I2C Communication** to interface with the RoboSub's onboard Linux-based Jetson PC.

## Repository Structure
```
├── app/
│   ├── main.c                # Main firmware source code for the MSP430 microcontroller
```

## Installation & Setup
1. **Clone the repository**:
   ```sh
   git clone https://github.com/NotJonnyJ/18ROBO_Team_Dev.git
   cd 18ROBO_Team_Dev
   ```
2. **Open with Code Composer Studio v20+** 
   ```sh
   - open CCS v20 or later
   - open the project file located in /app directory
   - build and upload the project using CCS to the MSP430 Dev board to confirm connection

   ```
3. **Connect sensors** and test water detection functionality.
4. **Monitor output** through I2C communication to verify system status.

## Usage
- When water is detected at **hazard levels**, the LED will flash yellow.
- If water exceeds the **critical level**, the LED turns red, and an emergency shutdown is initiated.
- The system logs data and sends alerts to the Jetson PC via I2C.

## Contributors
- **Nicholas Gill** – PCB Development, Voltage Regulation, Integration
- **Jonathon Hughes** – Microcontroller Development, I2C Communication
- **Logan Graham** – Verification, Visual Alert Subsystem
- **Justin Westwood** – Fabrication, Hazard & Critical Level Subsystems


## Acknowledgments
Special thanks to **Dr. Bradley Whitaker** and the Montana State University RoboCats team for their guidance and support.
