# Water Rehabilitation Assistance System

## Project Description
This project involves a system that supports movement rehabilitation in a water environment. It utilizes a Raspberry Pi 4 microcomputer, an MPU6050 motion sensor, and 1027 vibration motors to monitor and correct arm movement. The system provides feedback in both haptic (vibration) and visual (screen animation) forms, allowing better control over performed exercises.

## Technologies and Tools
- **Programming Language:** C
- **Libraries:** WiringPi, GTK+3, POSIX Threads
- **Hardware:** Raspberry Pi 4, MPU6050, 1027 Vibration Motors
- **Operating System:** Raspberry Pi OS

## Required Components
To ensure proper system operation, the following components are required:
- **Raspberry Pi 4** (with power supply and microSD card with Raspberry Pi OS installed)
- **MPU6050 Accelerometer and Gyroscope Module**
- **1027 Vibration Motors** (2 pieces)
- **Electrical resistors** (18Ω and 1kΩ)
- **1N4001 Diode**
- **2N2222 Transistor**
- **Prototype PCB board**
- **Connecting wires**
- **Epoxy resin for waterproofing**

## Features
- Reading and interpreting data from the MPU6050 sensor
- Controlling vibration motors based on arm tilt angle
- Graphical interface for inputting threshold values and visualizing movement
- Multithreading for smooth system operation
- Waterproofing of components using epoxy resin

## Installation and Configuration
### 1. Install Libraries
```sh
sudo apt-get install wiringpi
sudo apt install libgtk-3-dev
```
To verify the installation:
```sh
gpio -v
pkg-config --modversion gtk+-3.0
```

### 2. Compile the Code
```sh
gcc -o main main.c `pkg-config --cflags --libs gtk+-3.0` -lwiringPi -lm -pthread
```

### 3. Run the Program
```sh
./main
```

## Usage
1. After starting the program, a window will appear for entering threshold values for activating the motors.
2. Once confirmed, an animation window displaying real-time arm movement updates will open.
3. Vibration motors will activate when set threshold angles are exceeded.
4. The program can be closed using the "Exit" button.

## Potential Improvements
- Implementation of wireless communication instead of wired connections
- Integration with Arduino platform
- Extending the system for rehabilitation of other body parts
- Enhancing the graphical interface and adding new visualizations

## Author
Michał Kochanowicz
