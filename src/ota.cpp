#include "ota.h"
#include <ArduinoOTA.h>
#include <esp_task_wdt.h>
#include <esp32-hal-log.h>

static const char *_hostname = nullptr;
static const char *_password = nullptr;
static OtaStartCallback _callbackStart = nullptr;
static bool otaStarted = false;
static int lastPercent = -1;

static const char *ota_error_text(ota_error_t error)
{
    switch (error)
    {
    case OTA_AUTH_ERROR:    return "auth failed";
    case OTA_BEGIN_ERROR:   return "begin failed";
    case OTA_CONNECT_ERROR: return "connect failed";
    case OTA_RECEIVE_ERROR: return "receive failed";
    case OTA_END_ERROR:     return "end failed";
    default:                return "unknown";
    }
}

void ota_init(const char *hostname, const char *password, OtaStartCallback onStart)
{
    _hostname = hostname;
    _password = password;
    _callbackStart = onStart;
}

// MDNS.begin() inside ArduinoOTA.begin() needs a network, so it runs
// only after the first WiFi connection (and only once).
static void ota_start()
{
    ArduinoOTA.setHostname(_hostname);
    if (_password != nullptr && strlen(_password) > 0)
    {
        ArduinoOTA.setPassword(_password);
    }

    ArduinoOTA.onStart([]()
                       {
        log_i("OTA: start");
        lastPercent = -1;
        if (_callbackStart)
        {
            _callbackStart();
        } });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          {
        // The whole upload runs inside ArduinoOTA.handle(), loop() is blocked
        // meanwhile - reset the watchdog after every data block.
        esp_task_wdt_reset();

        int percent = total > 0 ? progress * 100 / total : 0;
        if (percent != lastPercent && percent % 10 == 0)
        {
            log_i("OTA: %d %%", percent);
        }
        lastPercent = percent; });

    ArduinoOTA.onEnd([]()
                     { log_i("OTA: done, restarting"); });

    ArduinoOTA.onError([](ota_error_t error)
                       { log_e("OTA: error %u (%s)", error, ota_error_text(error)); });

    ArduinoOTA.begin();
    otaStarted = true;
    log_i("OTA: ready at %s.local%s", _hostname, (_password != nullptr && strlen(_password) > 0) ? "" : " (NO PASSWORD)");
}

void ota_loop(bool wifiConnected)
{
    if (!otaStarted)
    {
        if (wifiConnected)
        {
            ota_start();
        }
        return;
    }

    ArduinoOTA.handle();
}
