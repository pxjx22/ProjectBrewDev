# ProjectBrewDev: FOSS Gaggia Classic Pro PID Controller

**⚠️ IMPORTANT DISCLAIMER: WORK IN PROGRESS - DO NOT USE! ⚠️** **This project is currently under active development and is NOT yet stable, complete, or tested for safety and reliability. The code and hardware designs shared here are experimental and intended for development and learning purposes only at this stage.** **DO NOT attempt to install or use this system on your Gaggia Classic Pro or any other espresso machine in its current state. Doing so could result in damage to your machine, create an electrical hazard, or lead to unpredictable behavior.** Project BrewDev is a DIY, open-source PID temperature controller and enhancement system specifically designed for the Gaggia Classic Pro espresso machine. It utilizes an ESP32 microcontroller to provide precise temperature control and additional features for an improved brewing experience.

## Project Goal
This project aims to design and build a custom, open-source (FOSS) PID controller and enhancement system specifically for the Gaggia Classic Pro espresso machine. The primary motivations are:

* **Enhanced Control & Consistency:** To achieve precise, stable, and user-configurable PID temperature control for both brewing and steam functions, leading to more consistent and repeatable espresso shots.
* **Learning & Customization:** To foster a deeper understanding of the machine's operation and provide a platform for custom modifications and feature additions beyond what off-the-shelf solutions might offer. This project is built with a DIY spirit, valuing hands-on learning and tailored solutions.
* **Integrated User Experience:** To move beyond basic PID functionality by incorporating a user-friendly interface with an OLED display and rotary encoder for real-time feedback and easy adjustments, along with practical features like a shot timer.
* **Open Source Contribution:** To develop and share a well-documented, FOSS solution (hardware designs, software, and documentation) on GitHub, making it accessible for other Gaggia enthusiasts to build, adapt, and contribute to.

## Current Status
Project BrewDev is currently in the "hardware pending" stage of V1 development. The root cause of major system instability has been identified as the failure of the ESP32's onboard 3.3V regulator under electrical load. The project is awaiting the installation of a robust, external 3.3V regulator to create a stable power supply for the microcontroller and its peripherals.

## Key Features (V1 Scope)
The initial V1 release of Project BrewDev aims to implement the following core features:

* PID control for brew temperature
* PID control for steam temperature (triggered by Gaggia's physical switch)
* An I2C OLED Display (1.3") to show current mode, current temp, desired temp, and a shot timer.
* A Rotary Encoder (KY-040 module) for user input to adjust temperature setpoints.
* A Shot Timer function, triggered by the Gaggia's physical brew switch.
* A "Ready" LED indicator to visually signal when the machine has reached the desired temperature.

## Roadmap & To-Do
- [x] ~~Initial component sourcing and V1 software draft.~~
- [x] ~~Diagnose and troubleshoot hardware instability and electrical noise.~~
- [x] ~~Identify ESP32 onboard regulator as the point of failure.~~
- [ ] **Install and validate external 3.3V LDO regulator fix.**
- [ ] Perform systematic hardware validation of all V1 components on a stable power rail.
- [ ] Complete PID tuning for both brew and steam modes.
- [ ] Write a detailed, step-by-step build guide with photos and clear schematics.
- [ ] Clean up, comment, and optimize the V1 firmware for public release.
- [ ] **Future:** Explore V2 features like pressure profiling, flow control, and pre-infusion.

---
## Project BrewDev: V1 Component Checklist
This list reflects the current, revised hardware specification after initial troubleshooting.

#### I. Core Platform & Development:
- { } [ESP32-WROOM-32D Development Board](https://www.aliexpress.com/item/1005006456519790.html)
- { } [ESP32 38Pin Screw Terminal Board (for easier wiring)](https://www.aliexpress.com/item/1005006026098254.html)

#### II. Sensors & Amplifiers (for V1 PID Control):
- { } [K-Type Thermocouple Probe (M4 Screw Thread)](https://www.aliexpress.com/item/1005005496786289.html) (x2, one for brew, one for steam)
- { } [MAX31855 Thermocouple Sensor Module](https://www.aliexpress.com/item/1902975189.html) (x2, one for each thermocouple)

#### III. User Interface:
- { } [DIYUSER 1.3" IIC OLED Display Module (White/BLUE, SH1106 based)](https://www.aliexpress.com/item/1005007451015054.html)
- { } [KY-040 Rotary Encoder Module](https://www.aliexpress.com/item/1005006551162496.html)
- { } [Standard LED (e.g., 3mm or 5mm, any color)](https://www.aliexpress.com/item/1005007591932915.html)
- { } [Current Limiting Resistor for LED (e.g., 220Ω - 330Ω)](https://www.aliexpress.com/item/1005006209050774.html)

#### IV. Power Supply & Control:
- { } [HLK-PM01 (5V 3W AC-DC Power Supply Module)](https://www.aliexpress.com/item/1005006072424191.html)
- { } **External 3.3V LDO Regulator (e.g., LM3940, AMS1117-3.3)**
- { } [Solid State Relay (DC-AC SSR-40DA)](https://www.aliexpress.com/item/1005005837105164.html)

#### V. Interfacing & Circuit Stability:
- { } [PC817 Optocoupler or AC 220V Optocoupler Isolation Module](https://www.aliexpress.com/item/1005007458865867.html) (x2)
- { } **Filter Capacitors:**
    - { } 1000µF Electrolytic Capacitor (for smoothing HLK-PM01 5V output)
    - { } 10µF Electrolytic Capacitor (for 5V power rail)
    - { } 0.1µF (100nF) Ceramic Capacitor (for high-frequency noise on 5V and 3.3V rails)

#### VI. Essential Prototyping & Connection:
- { } [Dupont Jumper Wires (Mix kit)](https://www.aliexpress.com/item/1005003252824475.html)
- { } Wires for mains connections (18AWG)
- { } Wires for low voltage DC connections (22AWG)
- { } [6.3mm Spade connector (Female, Male)](https://www.aliexpress.com/item/1005002765359666.html)
- { } Mounting hardware (M2/M2.5/M3 bolts, nuts, standoffs)
