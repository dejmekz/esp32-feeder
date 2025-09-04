#include "wifi_handler.h"
#include <WiFi.h>
#include <esp32-hal-log.h>

static const char *_wifi_ssid = nullptr;
static const char *_wifi_password = nullptr;
static unsigned long lastWifiAttemptTime = 0;

void wifi_init(const char *device_name, const char *ssid, const char *password)
{
    _wifi_ssid = ssid;
    _wifi_password = password;
    WiFi.setHostname(device_name); // Set a hostname for the ESP32
    WiFi.disconnect(true, true);   // Disconnect from any network and erase old config
    wifi_connect(true);
}

void wifi_connect(bool rst)
{
    WiFi.disconnect(true, rst);
    delay(1000);                   // short delay to avoid overloading
    WiFi.mode(WIFI_STA); // Set WiFi mode to Station
    wl_status_t status = WiFi.begin(_wifi_ssid, _wifi_password);
    log_i("Connecting to WiFi SSID: %s, status: %d", _wifi_ssid, status);
    delay(50); // short delay to avoid overloading
}

bool wifi_loop()
{
    bool isConnected = WiFi.status() == WL_CONNECTED;
    if (!isConnected)
    {
        log_d("WiFi not connected - status: %d", WiFi.status());
        unsigned long currentTime = millis();
        if (currentTime - lastWifiAttemptTime >= 60000)
        { // Attempt to reconnect every 60 seconds
            lastWifiAttemptTime = currentTime;
            log_i("Reconnecting to WiFi...");
            wifi_connect(false);
        }
    }
    return isConnected;
}

bool wifi_is_connected()
{
    return WiFi.status() == WL_CONNECTED;
}

void wifi_add_info(JsonDocument &doc)
{
    doc["wifi"]["ssid"] = WiFi.SSID();
    doc["wifi"]["bssid"] = WiFi.BSSIDstr();
    doc["wifi"]["rssi"] = WiFi.RSSI();
    doc["wifi"]["channel"] = WiFi.channel();
    doc["wifi"]["ip"] = WiFi.localIP();
    doc["wifi"]["mac"] = WiFi.macAddress();
}