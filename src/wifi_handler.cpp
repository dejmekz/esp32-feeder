#include "wifi_handler.h"
#include <WiFi.h>
#include <esp32-hal-log.h>
#include "config.h"

// Lower TX power = lower current peaks while transmitting. At full power
// (19.5 dBm) TX peaks combined with motor/servo start or flash writes
// (FOTA) can cause a brownout reset. Override in config.h if needed.
#ifndef WIFI_TX_POWER
#define WIFI_TX_POWER WIFI_POWER_11dBm
#endif

// External flag from main.cpp - reset on disconnect to force NTP resync
extern volatile bool clockSynced;

static const char *_wifi_ssid = nullptr;
static const char *_wifi_password = nullptr;
static unsigned long wifiDisconnectedSince = 0;
// Written from the WiFi event task, read from loop()
static volatile bool wifiConnected = false;

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
    log_i("WiFi connected - IP address: %s", WiFi.localIP().toString().c_str());
    wifiConnected = true;
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    log_d("WiFi lost connection. Reason: %d", info.wifi_sta_disconnected.reason);
    wifiConnected = false;
    clockSynced = false;  // Force NTP resync on reconnect
}

void wifi_init(const char *device_name, const char *ssid, const char *password)
{
    _wifi_ssid = ssid;
    _wifi_password = password;
    WiFi.setHostname(device_name); // Set a hostname for the ESP32
    WiFi.disconnect(true, true);   // Disconnect from any network and erase old config

    // Examples of different ways to register wifi events
    //WiFi.onEvent(WiFiEvent);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.persistent(false);        // don't write credentials to flash on every begin()
    WiFi.mode(WIFI_STA); // Set WiFi mode to Station
    WiFi.setTxPower(WIFI_TX_POWER);
    // Mains powered: without modem sleep the device reliably answers
    // web requests and MQTT keepalive.
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    wl_status_t status = WiFi.begin(_wifi_ssid, _wifi_password);
    log_i("Connecting to WiFi SSID: %s, status: %d", _wifi_ssid, status);
}

// Common dropouts are handled by the library's auto reconnect. Restart the
// connection manually only when not connected for WIFI_RECONNECT_INTERVAL_MS.
// The radio stays on, so TX power and sleep settings are kept.
bool wifi_loop()
{
    bool isConnected = WiFi.status() == WL_CONNECTED;
    if (isConnected)
    {
        wifiDisconnectedSince = 0;
        return wifiConnected;
    }

    log_v("WiFi not connected - status: %d", WiFi.status());
    unsigned long currentTime = millis();
    if (wifiDisconnectedSince == 0)
    {
        wifiDisconnectedSince = currentTime;
    }
    else if (currentTime - wifiDisconnectedSince >= WIFI_RECONNECT_INTERVAL_MS)
    {
        wifiDisconnectedSince = currentTime;
        log_i("WiFi not connected for %lu s, reconnecting...", WIFI_RECONNECT_INTERVAL_MS / 1000);
        WiFi.disconnect();
        WiFi.begin(_wifi_ssid, _wifi_password);
    }
    return false;
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