# ESP32 MQTT Sound Sensor with WiFiManager

This project implements a WiFi-enabled ESP32 sound sensor that integrates with Home Assistant using MQTT. It simulates sound level readings and updates an RGB LED based on MQTT color messages from Home Assistant. Configuration is done through a captive portal on first boot.

## Overview

The ESP32 device:

- Starts a **captive portal** to collect WiFi and MQTT broker settings from the user
- Connects to the specified WiFi network
- Registers itself with the MQTT broker using a unique hardware ID and user-defined location
- Publishes simulated sound level data every 5 seconds
- Subscribes to MQTT color control messages to update the RGB LED
- Reflects noise thresholds via RGB LED color:
  - Green: Low
  - Yellow: Medium
  - Red: High

## Captive Portal Configuration

When powered on, the ESP32 creates a temporary access point:

- **SSID**: `ESP32_Setup`
- **Password**: `esp32config`

Using a phone or computer, connect to this network. A **captive portal** will automatically open where you can configure:

- **WiFi SSID** and **Password**: Credentials for the network the ESP32 should join
- **MQTT Broker IP Address**: IP address of your Home Assistant or MQTT server
- **MQTT Username** and **Password**: For brokers requiring authentication
- **ESP32 Location**: Logical name or label (e.g., "Living Room", "Garage") for identifying the device

After submitting the form, the ESP32 connects to WiFi and the MQTT broker, and begins operation.

## RGB LED Behavior

The RGB LED is controlled by color commands received via MQTT from Home Assistant. These commands reflect the current noise threshold stage.

| Threshold Stage | Color  | Description              |
|-----------------|--------|--------------------------|
| Low             | Green  | Sound level is low       |
| Medium          | Yellow | Moderate sound level     |
| High            | Red    | High sound level         |

Color messages must be sent as plain text: `"green"`, `"yellow"`, or `"red"`.

## MQTT Topics

| Purpose             | Topic Format                         |
|---------------------|--------------------------------------|
| Sound level publish | `home/sound/level`                   |
| Registration        | `home/sensor/register`               |
| Response subscribe  | `home/sensor/<hwid>/response`        |
| Color control       | `home/sensor/<hwid>/color`           |

`<hwid>` is the MAC address of the ESP32 (lowercase, without colons).

## Required Libraries

Install the following libraries via the Arduino Library Manager:

- [WiFiManager](https://github.com/tzapu/WiFiManager)
- [PubSubClient](https://github.com/knolleary/pubsubclient)
- `WiFi.h` (included with the ESP32 board support)

## Building and Uploading (Arduino IDE)

1. **Install ESP32 board support**:
   - Go to **File > Preferences**
   - Add this to **Additional Board Manager URLs**:  
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Open **Tools > Board > Boards Manager**, search for **ESP32**, and install

2. **Install required libraries** as listed above

3. **Configure board settings**:
   - Board: `ESP32 Dev Module` (or your specific variant)
   - Port: Select the correct serial port for your ESP32

4. **Upload the sketch**:
   - Click **Verify** to compile
   - Click **Upload** to flash the firmware

## Notes

- Sound levels are randomly generated for demonstration purposes and can be replaced with real sensor readings.
- The RGB LED requires correct configuration of the `RGB_BUILTIN` macro based on your hardware.
- You can test MQTT topics using tools like MQTT Explorer or the `mosquitto_pub` / `mosquitto_sub` CLI.


