# ProjectBrewDev: FOSS Gaggia Classic Pro PID Controller
Project BrewDev is a DIY, open-source PID temperature controller and enhancement system specifically designed for the Gaggia Classic Pro espresso machine. It utilizes an ESP32 microcontroller to provide precise temperature control and additional features for an improved brewing experience.

## Project Goal
This project aims to design and build a custom, open-source (FOSS) PID controller and enhancement system specifically for the Gaggia Classic Pro espresso machine. [cite: 1] The primary motivations are:

* **Enhanced Control & Consistency:** To achieve precise, stable, and user-configurable PID temperature control for both brewing and steam functions, leading to more consistent and repeatable espresso shots.
* **Learning & Customization:** To foster a deeper understanding of the machine's operation and provide a platform for custom modifications and feature additions beyond what off-the-shelf solutions might offer. This project is built with a DIY spirit, valuing hands-on learning and tailored solutions.
* **Integrated User Experience:** To move beyond basic PID functionality by incorporating a user-friendly interface with an OLED display and rotary encoder for real-time feedback and easy adjustments, along with practical features like a shot timer.
* **Open Source Contribution:** To develop and share a well-documented, FOSS solution (hardware designs, software, and documentation) on GitHub, making it accessible for other Gaggia enthusiasts to build, adapt, and contribute to.

## Current Status

Project BrewDev is currently in the V1 development phase. The initial software for V1 has been drafted and compiles, component selection is complete, and the wiring/hardware assembly phase is commencing. This README and other documentation are being actively developed alongside the build.


## Key Features (V1 Scope)

The initial V1 release of Project BrewDev aims to implement the following core features:

* PID control for brew temperature
* PID control for steam temperature (triggered by Gaggia's physical switch)
* An I2C OLED Display (1.3") to show current mode, current temp, desired temp, and a shot timer.
* A Rotary Encoder (KY-040 module) for user input
    * Adjusting brew and steam setpoint temperatures.
* A Shot Timer function, triggered by the Gaggia's physical brew switch, which resets when the brew switch is turned off.
* A "Ready" LED indicator to visually signal when the machine has reached the desired temperature.
