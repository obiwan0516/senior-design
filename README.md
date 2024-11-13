# Arduino Sensor and Display Project

This repository contains Arduino sketches and libraries for interfacing various sensors and an LCD. Each file is designed to test or implement specific sensors or modules in an Arduino environment. Below is a description of each included file and how it contributes to the project.

---

## Files and Directories

### 1. `1602A_test`
- **Description**: This sketch tests the 1602A LCD display functionality. It initializes the LCD and displays a sample message to verify that the display works as expected.
- **Usage**: Use this file to confirm that the 1602A LCD display is functioning properly.

### 2. `HC-SR04`
- **Description**: This sketch interfaces with the HC-SR04 ultrasonic distance sensor. It measures the distance to an object in front of the sensor and outputs the reading to the Serial Monitor.
- **Usage**: Use this file to test the HC-SR04 sensor independently.

### 3. `HC_SR04_1602A`
- **Description**: This sketch combines the HC-SR04 ultrasonic distance sensor with the 1602A LCD display. It measures the distance using the sensor and displays the result on the LCD screen.
- **Usage**: Use this file to view the real-time distance measurements on the LCD.

### 4. `I2C_Scanner`
- **Description**: This sketch scans all I2C addresses to identify devices connected to the I2C bus. It can help identify the I2C addresses of connected modules such as the VL53L0X or TMAG5273 sensors.
- **Usage**: Use this file to verify the I2C addresses of your connected devices, ensuring they are correctly wired and recognized.

### 5. `libraries`
- **Description**: This directory contains required libraries for the project, including the libraries for LCD, VL53L0X (time-of-flight), and TMAG5273 (Hall effect) sensors.
- **Usage**: Ensure these libraries are installed in your Arduino IDE before compiling any sketches.

### 6. `VL53L0`
- **Description**: This sketch interfaces with the VL53L0X time-of-flight sensor, which measures the distance to an object based on laser technology.
- **Usage**: Use this file to test the VL53L0X sensor independently and view the readings in the Serial Monitor.

### 7. `VL53L0_1602A`
- **Description**: This sketch combines the VL53L0X time-of-flight sensor with the 1602A LCD display. It measures distance and displays the readings on the LCD screen.
- **Usage**: Use this file to view VL53L0X distance measurements on the LCD display.

### 8. `VL53L0_1602A_TMAG5273`
- **Description**: This sketch interfaces with the VL53L0X time-of-flight sensor, 1602A LCD display, and TMAG5273 Hall effect sensor. It collects readings from both sensors and displays them on the LCD screen.
- **Usage**: Use this file to simultaneously measure distance and magnetic field strength, displaying both results on the LCD.

---

## Setup Instructions

1. **Library Installation**:
   - Copy all necessary libraries from the `libraries` directory into the Arduino IDE `libraries` folder.

2. **Wiring**:
   - Connect the sensors and LCD according to each sketch's requirements. Ensure that devices on the I2C bus are wired to the correct SDA and SCL pins.

3. **Upload Sketches**:
   - Open each sketch in the Arduino IDE, select the correct board and COM port, and upload it to your Arduino.

4. **Testing**:
   - Run each file and check either the Serial Monitor or the 1602A LCD for sensor readings and messages.

---

## Troubleshooting

- **I2C Errors**: Run the `I2C_Scanner` sketch to ensure all I2C devices are correctly connected and addresses are detected.
- **No Display on LCD**: Run the `1602A_test` sketch to verify that the LCD is functional.
- **Incorrect Readings**: Verify sensor connections and ensure power requirements are met (e.g., 3.3V or 5V).

---

## License

This project is open-source. Feel free to use and modify the code for educational and personal projects.
