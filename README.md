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

For further firmware updates is used library esp32FOTA.

## Useful libraries
[esp32FOTA library for Arduino](https://github.com/chrisjoyce911/esp32FOTA)

> [!IMPORTANT]
> rename include config_tamplate.h to config.h
