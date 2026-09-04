# Smart Bedsore Detection System

An IoT-based medical monitoring system built on the ESP32 that prevents pressure ulcers (bedsores) by continuously tracking patient contact pressure, duration, and skin temperature.

## Overview
The Smart Bedsore Detection System is designed to alert caregivers before pressure ulcers can form on bedridden patients. It utilizes Force Sensitive Resistors (FSRs) to measure pressure across multiple contact points and a BME280 sensor to monitor skin temperature. Real-time data is displayed locally on an OLED screen and can be monitored remotely via the Blynk IoT platform.

## Features
- **Continuous Pressure Monitoring:** Reads data from 4 FSRs to calculate average force and pressure.
- **Temperature Tracking:** Monitors skin temperature using a BME280 sensor.
- **Risk Assessment:** Categorizes patient risk into LOW, MODERATE, HIGH, and CRITICAL levels based on pressure magnitude and duration.
- **Local Alerts:** Visual (Red LED) and Audio (Buzzer) warnings based on the severity of the risk.
- **Remote Alerts:** Sends critical push notifications to caregivers using Blynk.
- **Real-Time Display:** Shows current pressure, force, temperature, risk score, and status on an I2C OLED display.

## Hardware Components
- 1x ESP32 Microcontroller
- 4x Force Sensitive Resistors (FSRs)
- 1x BME280 (Temperature/Humidity/Pressure Sensor)
- 1x OLED Display (SSD1306, 128x64, I2C)
- 1x Red LED
- 1x Active Buzzer
- Resistors (for FSR voltage dividers and LED)
- Jumper Wires & Breadboard

## Pin Configuration
| Component | ESP32 Pin |
|-----------|-----------|
| FSR 1     | GPIO 34   |
| FSR 2     | GPIO 35   |
| FSR 3     | GPIO 32   |
| FSR 4     | GPIO 33   |
| OLED SDA  | GPIO 18   |
| OLED SCL  | GPIO 19   |
| BME SDA   | GPIO 21   |
| BME SCL   | GPIO 22   |
| Red LED   | GPIO 25   |
| Buzzer    | GPIO 26   |

## Risk Levels & Alert Logic
- **LOW:** Safe pressure levels (< 32 mmHg). No alerts.
- **MODERATE:** Sustained pressure (32-40 mmHg) for over 15 minutes. Triggers a visual LED warning.
- **HIGH:** High pressure (40-60 mmHg) for over 5 minutes. Triggers an LED and a slow beep buzzer alarm.
- **CRITICAL:** Dangerous pressure (> 60 mmHg) or prolonged moderate pressure (> 60 minutes). Triggers a rapid buzzer alarm and sends a critical push notification to caregivers via Blynk.

## Software Dependencies
Ensure you have the following libraries installed in your Arduino IDE:
- `Blynk` by Volodymyr Shymanskyy
- `Adafruit BME280 Library` by Adafruit
- `Adafruit SSD1306` by Adafruit
- `Adafruit GFX Library` by Adafruit

## Setup and Installation
1. Install the [Arduino IDE](https://www.arduino.cc/en/software) and add [ESP32 board support](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
2. Install the required libraries via the Arduino Library Manager.
3. Open `real_bedsore_condition.ino`.
4. Update the WiFi credentials:
   ```cpp
   char ssid[] = "your_wifi_ssid";
   char pass[] = "your_wifi_password";
   ```
5. Update your Blynk Authentication Token:
   ```cpp
   char auth[] = "your_blynk_auth_token";
   ```
6. Connect the ESP32 to your computer and upload the code.
7. Set up your Blynk dashboard using the Virtual Pins:
   - `V0`: Temperature (°C)
   - `V1`: Risk Level (String)
   - `V2`: Display Risk Score (%)
   - `V3`: Average Pressure (mmHg)
   - `V4`: Average Force (N)
