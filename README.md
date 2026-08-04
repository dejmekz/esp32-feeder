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

## Firmware signing

OTA firmware images are fetched over plain HTTP, so the device verifies an
RSA-4096/SHA256 signature (via esp32FOTA) before switching to a downloaded
image. This is checked independently of transport, so it also rejects a
tampered image even from a MITM on the network.

One-time setup — generate a signing keypair (never commit `priv_key.pem`;
it's already gitignored under `keys/`):
```
mkdir -p keys
openssl genrsa -out keys/priv_key.pem 4096
openssl rsa -in keys/priv_key.pem -pubout -out keys/rsa_key.pub
```
Then paste the contents of `keys/rsa_key.pub` into `include/fota_pubkey.h`
(replacing the existing `FOTA_RSA_PUB_KEY` block) and rebuild/flash the
device once so it trusts the new key.

For every release, sign the compiled firmware before uploading it to the
FOTA server:
```
pio run -e esp32-c3-devkitm-1
scripts/sign_firmware.sh .pio/build/esp32-c3-devkitm-1/firmware.bin firmware.img
```
Upload `firmware.img` (not the raw `.bin`) to the FOTA server and point
`fota.json`'s `bin`/URL fields at it, as usual for esp32FOTA.

> [!IMPORTANT]
> rename include config_tamplate.h to config.h
