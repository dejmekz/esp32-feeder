# ESP32 feeder

An automatic pet feeder that delivers food twice a day at a specific time.

build_flag:
USING_MOTOR - switch between use a motor shield (L298N) or servo MG995 360°

## Hardware components

* ESP32 development board
* L298N / servo MG995 360°
* 5V/1A power supply
* dupont jumper wires and connectors
* RED Led
* BLUE Led
* Button

## Websites

Inspiration for website creation.

[Irrigation Automation (ESP32/MQTT)](https://github.com/lrswss/esp32-irrigation-automation)

## Firmware upload

To compile the firmware for the ESP32 controller just download [Visual
Studio Code](https://code.visualstudio.com/) and install the [PlatformIO
add-on](https://platformio.org/install/ide?install=vscode). Open the project
directory and adjust the settings in `include/config.h` to your needs.

## Firmware update over WiFi (OTA)

Further firmware updates are pushed over WiFi with ArduinoOTA, no update
server is needed. The first firmware with OTA must be uploaded over USB.

1. Set `OTA_PASSWORD` in `include/config.h`.
2. Copy `ota.secret.ini.example` as `ota.secret.ini` (not in git) and set
   the same password and the device host name. The device name is printed
   at startup (`Device name: feeder-XXXXXXXX`), the host is
   `feeder-XXXXXXXX.local` (or use the IP address).
3. Upload: `pio run -e ota -t upload`

The motor stops and the device goes offline in MQTT when the upload starts.
The reason of the last restart is published as `reset` in the
`feeder/<device>/wifi` topic (e.g. `brownout`, `task watchdog`).

> [!IMPORTANT]
> rename include config_tamplate.h to config.h
