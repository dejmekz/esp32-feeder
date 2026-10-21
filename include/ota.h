#ifndef _OTA_H
#define _OTA_H

typedef void (*OtaStartCallback)();

// Firmware update over WiFi (ArduinoOTA), pushed from PlatformIO: pio run -e ota -t upload
// onStart is called before the upload starts (stop motor, go offline in MQTT).
void ota_init(const char *hostname, const char *password, OtaStartCallback onStart);

// Call on every loop pass; the whole upload runs inside.
// Starts the OTA service (and mDNS) after the first WiFi connection.
void ota_loop(bool wifiConnected);

#endif
