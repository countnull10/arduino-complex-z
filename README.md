# Analog Joystick Tilt Coordinates & Complex Numbers

[![Linux Day 2026](https://img.shields.io/badge/Event-Linux%20Day%202026-blue.svg)](https://github.com/countnull10)
[![GTK3](https://img.shields.io/badge/GUI-GTK3%20%2F%20Cairo-green.svg)](https://www.gtk.org/)
[![Arduino](https://img.shields.io/badge/Hardware-Arduino-orange.svg)](https://www.arduino.cc/)

A demonstrative and educational project created for **Linux Day 2026** (in collaboration with the Palermo Linux Club "Free Circle"). It combines **Arduino**, trigonometric formulas (complex numbers and angle calculations), and a desktop graphical interface written in **C using GTK3 and Cairo**.

---

## 📌 Overview

The system consists of two main components:
1. **Arduino Firmware:** Reads raw values from an analog joystick, maps them onto a Gauss plane ($X$ and $Y$ coordinates), calculates the corresponding complex number $z$, and computes the angle in degrees using `atan2`. Data is displayed on a 16x2 LCD screen and transmitted in real-time via the serial port.
2. **Desktop Application (GTK3):** Listens to the serial stream from Arduino and renders in real-time—using Cairo graphics—the joystick's position inside a circle, along with a projected point on the perimeter.

---

## 🛠️ Hardware Requirements

* **Arduino UNO**
* **16x2 LCD Display** (Hitachi HD44780 driver compatible)
* **Analog Joystick Module**
* **10K Potentiometer / Resistor** (for LCD contrast adjustment)
* Jumper wires and breadboard

### Wiring Diagram (Arduino)

| LCD Pin / Component | Arduino Pin / Connection |
| :--- | :--- |
| **LCD RS** | Digital Pin 12 |
| **LCD Enable (E)** | Digital Pin 11 |
| **LCD D4** | Digital Pin 5 |
| **LCD D5** | Digital Pin 4 |
| **LCD D6** | Digital Pin 3 |
| **LCD D7** | Digital Pin 2 |
| **LCD R/W, VSS** | Ground (GND) |
| **LCD VCC** | 5V |
| **LCD VO (Pin 3)** | 10K Resistor / Potentiometer Wiper |
| **AnalogStick GND** | GND |
| **AnalogStick +5V** | 5V |
| **AnalogStick VRx** | Analog Pin A0 |
| **AnalogStick VRy** | Analog Pin A1 |

---

## 📁 Repository Structure

* `./pc-app/joystick_gui.c` - Main source code for the GTK3 graphical interface, window event handling (e.g., full-screen toggle with `F11`), and Cairo-based rendering.
* `./pc-app/serial.h` - Custom header library for serial port configuration (`termios`) and data parsing (`/dev/ttyACM0`).
* `./arduino/joystick_lcd/joystick_lcd.ino` - Arduino sketch handling the LCD output and serial telemetry transmission at `9600 bps`.

---

## 🎨 Logos & Customization

The graphical interface includes spaces for custom logos (such as event or creator branding) loaded via an `./assets/` directory. 
* Since the `assets` folder is omitted from this repository, **if you want to display custom logos, you must manually create an `assets/` folder in the project root** and place your image files inside it (matching the filenames expected by the source code, e.g., `logo-free-circle.png`, `logo-linux-day.png`, `logo-count-null2.png`), or modify the paths in `joystick_gui.c`.

---

## 🚀 Compilation & Installation (Linux)

### Prerequisites
Make sure you have a C compiler (`gcc`), `make`, and the **GTK+ 3 development libraries** installed on your system.

On Debian/Ubuntu/Mint:
```bash
sudo apt update
sudo apt install build-essential libgtk-3-dev pkg-config
```

### Building and Running
Flash the joystick_lcd.ino sketch onto your Arduino UNO and connect it to your PC (default serial port: /dev/ttyACM0).
Compile the GTK3 desktop application:
`gcc ./pc-app/joystick_gui.c -o joystick_gui $(pkg-config --cflags --libs gtk+-3.0) -lm`

Execute the binary:
`./joystick_gui`

## 🤝 Contributing
Contributions, issues, and feature requests are welcome! Feel free to open an issue or pull request.
