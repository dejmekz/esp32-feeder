#include "web.h"
#include "config.h"
#include "html.h"
#include "utils.h"
#include "nvs_storage.h"
#include <esp32-hal-log.h>
#include <freertos/semphr.h>

// External mutex from main.cpp for thread-safe feederSettings access
extern SemaphoreHandle_t scheduleMutex;

WebServer webserver(80);
FeederCallback callFeederCallback = nullptr;
FeederSettings *feederSettings = nullptr;

static String get_formated_actual_millis()
{
    unsigned long currentMillis = millis();       // unsigned long
    unsigned long seconds = currentMillis / 1000; // unsigned long
    unsigned long minutes = seconds / 60;         // unsigned long
    unsigned long hours = minutes / 60;           // unsigned long
    unsigned long days = hours / 24;              // unsigned long
    seconds %= 60;
    minutes %= 60;
    hours %= 24;

    char temp[15];
    snprintf(temp, 15, "%02dd %02d:%02d:%02d", days, hours, minutes, seconds);

    return String(temp);
}

// pass sensor readings, system status to web ui as JSON
static void updateUI()
{
    JsonDocument doc;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
        char temp[12];
        snprintf(temp, 6, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        doc["time"] = String(temp);

        snprintf(temp, 12, "%02d.%02d.%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
        doc["date"] = String(temp);
    }
    else
    {
        doc["time"] = "00:00:00";
        doc["date"] = "01.01.1970";
    }

    // Read feeder settings with mutex protection
    if (feederSettings != nullptr && scheduleMutex != NULL)
    {
        if (xSemaphoreTake(scheduleMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            char temp[6];
            snprintf(temp, 6, "%02d:%02d", feederSettings->feed01.hour, feederSettings->feed01.minute);
            doc["time01"] = String(temp);
            doc["por01"] = feederSettings->feed01.portions;
            doc["dur01"] = feederSettings->feed01.duration;

            snprintf(temp, 6, "%02d:%02d", feederSettings->feed02.hour, feederSettings->feed02.minute);
            doc["time02"] = String(temp);
            doc["por02"] = feederSettings->feed02.portions;
            doc["dur02"] = feederSettings->feed02.duration;

            doc["temp"] = feederSettings->dht22Data.temperature;
            doc["hum"] = feederSettings->dht22Data.humidity;
            xSemaphoreGive(scheduleMutex);
        }
    }

    doc["runtime"] = get_formated_actual_millis();

    char buffer[512];
    size_t len = serializeJson(doc, buffer);
    if (len >= sizeof(buffer)) {
        webserver.send(500, F("text/plain"), "Response too large");
        return;
    }
    webserver.send(200, F("application/json"), buffer);
}

void handleFeedRequest()
{
    if (callFeederCallback)
    {
        callFeederCallback(1, FEEDING_DURATION);
        webserver.send(200, F("text/plain"), "Feeding started");
    }
    else
    {
        webserver.send(500, F("text/plain"), "Feeder callback not set");
    }
}

void webserver_set_callback_feeder(FeederCallback callback)
{
    callFeederCallback = callback;
}

void webserver_start(FeederSettings *setting)
{
    feederSettings = setting;

    // register web server handlers

    // send main page
    webserver.on("/", HTTP_GET, []()
                 {
        String html = FPSTR(HEADER_html);
        html += FPSTR(ROOT_html);
        html += FPSTR(FOOTER_html);
        html.replace("__FIRMWARE__", String(FIRMWARE_VERSION));
        html.replace("__BUILD__", String(__DATE__)+" "+String(__TIME__));
        webserver.send(200, "text/html", html); });

    // AJAX request from main page to update readings
    webserver.on("/ui", HTTP_GET, updateUI);

    // Manual feed trigger from web UI
    webserver.on("/feed", HTTP_GET, handleFeedRequest);

    // show irrgation settings
    webserver.on("/config", HTTP_GET, []()
                 {
        String html;
        html += FPSTR(HEADER_html);
        html += FPSTR(CONFIG_html);
        html += FPSTR(FOOTER_html);
        html.replace("__FIRMWARE__", String(FIRMWARE_VERSION));
        html.replace("__BUILD__", String(__DATE__) + " " + String(__TIME__));

        webserver.send(200, "text/html", html); });

    // save main settings to NVS
    webserver.on("/config", HTTP_POST, []()
                 {
        log_i("Saving settings from web...");

        // Parse values first (before taking mutex)
        int h1 = webserver.arg("h01").toInt();
        int m1 = webserver.arg("m01").toInt();
        int8_t p1 = webserver.arg("p01").toInt();
        int h2 = webserver.arg("h02").toInt();
        int m2 = webserver.arg("m02").toInt();
        int8_t p2 = webserver.arg("p02").toInt();

        // Validate feeding times
        if (h1 < 0 || h1 > 23 || m1 < 0 || m1 > 59 ||
            h2 < 0 || h2 > 23 || m2 < 0 || m2 > 59 ||
            p1 < 1 || p1 > MAX_PORTIONS || p2 < 1 || p2 > MAX_PORTIONS)
        {
            webserver.send(400, "text/plain", "Invalid feeding time or portions");
            return;
        }

        // Update with mutex protection
        if (scheduleMutex != NULL && xSemaphoreTake(scheduleMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            feederSettings->feed01.hour = h1;
            feederSettings->feed01.minute = m1;
            feederSettings->feed01.portions = p1;
            feederSettings->feed01.enabled = true;

            feederSettings->feed02.hour = h2;
            feederSettings->feed02.minute = m2;
            feederSettings->feed02.portions = p2;
            feederSettings->feed02.enabled = true;

            // Save to NVS for persistence
            nvs_save_feeder_config(feederSettings);

            xSemaphoreGive(scheduleMutex);
        } else {
            webserver.send(500, "text/plain", "Failed to acquire lock");
            return;
        }

        log_i("Feeder 1: %02d:%02d, portions: %d", h1, m1, p1);
        log_i("Feeder 2: %02d:%02d, portions: %d", h2, m2, p2);

        webserver.sendHeader("Location", "/config?saved=1", true);
        webserver.send(302, "text/plain", ""); });

    // soft reboot (short deep sleep, RTC memory is preserved)
    webserver.on("/restart", HTTP_GET, []()
                 {
                     webserver.send(200, "text/plain", "OK");
                     restartSystem(); });

    // triggers ESP.restart() thus RTC memory is lost
    webserver.on("/reset", HTTP_GET, []()
                 {
                     webserver.send(200, "text/plain", "OK");
                     resetSystem(); });

    webserver.onNotFound([]()
                         {
        String html;

        // send main page
        if (webserver.uri().endsWith("/")) {  
            html += HEADER_html;
            html += ROOT_html;
            webserver.send(200, "text/html", html);

        // send log file(s)
        } else { 
            webserver.send(404, "text/plain", "Error 404: file not found");
        } });

    webserver.begin();
    log_i(": Webserver started. [%d]", millis());
}

