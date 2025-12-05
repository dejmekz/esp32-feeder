#include "wifi_handler.h"
#include <WiFi.h>
#include <esp32-hal-log.h>

static const char *_wifi_ssid = nullptr;
static const char *_wifi_password = nullptr;
static unsigned long lastWifiAttemptTime = 0;
static bool wifiConnected = false;
/*
void WiFiEvent(WiFiEvent_t event){
    log_d("[WiFi-event] event: %d\n", event);

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY: 
            log_d("WiFi interface ready");
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            log_d("Completed scan for access points");
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            log_d("WiFi client started");
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            log_d("WiFi clients stopped");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            log_d("Connected to access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            log_d("Disconnected from WiFi access point");
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            log_d("Authentication mode of access point has changed");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            log_d("Obtained IP address: %s", WiFi.localIP().toString().c_str());
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            log_d("Lost IP address and IP address is reset to 0");
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            log_d("WiFi Protected Setup (WPS): succeeded in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            log_d("WiFi Protected Setup (WPS): failed in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            log_d("WiFi Protected Setup (WPS): timeout in enrollee mode");
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            log_d("WiFi Protected Setup (WPS): pin code in enrollee mode");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            log_d("WiFi access point started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            log_d("WiFi access point  stopped");
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            log_d("Client connected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            log_d("Client disconnected");
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            log_d("Assigned IP address to client");
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            log_d("Received probe request");
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            log_d("AP IPv6 is preferred");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            log_d("STA IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            log_d("Ethernet IPv6 is preferred");
            break;
        case ARDUINO_EVENT_ETH_START:
            log_d("Ethernet started");
            break;
        case ARDUINO_EVENT_ETH_STOP:
            log_d("Ethernet stopped");
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            log_d("Ethernet connected");
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            log_d("Ethernet disconnected");
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            log_d("Obtained IP address");
            break;
        default: break;
    }}*/

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
    log_i("WiFi connected - IP address: %s", WiFi.localIP().toString().c_str());
    wifiConnected = true;
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    log_d("WiFi lost connection. Reason: %d",info.wifi_sta_disconnected.reason);
    wifiConnected = false;
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
    return isConnected && wifiConnected;
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