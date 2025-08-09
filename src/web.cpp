#include "web.h"
#include "config.h"
#include "html.h"
#include "utils.h"
#include <esp32-hal-log.h>

static uint32_t webserverRequestMillis = 0;
static uint16_t webserverTimeout = 0;

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

    if (feederSettings != nullptr)
    {
        FeedingSettings *feed01 = feederSettings->feed01;
        FeedingSettings *feed02 = feederSettings->feed02;

        char temp[6];
        snprintf(temp, 6, "%02d:%02d", feed01->hour, feed01->minute);
        doc["time01"] = String(temp);
        doc["por01"] = feed01->portions;

        snprintf(temp, 6, "%02d:%02d", feed02->hour, feed02->minute);
        doc["time02"] = String(temp);
        doc["por02"] = feed02->portions;

        TempAndHumidity *th = feederSettings->dht22Data;
        doc["temp"] = th->temperature;
        doc["hum"] = th->humidity;
    }

    doc["runtime"] = get_formated_actual_millis();

    char buffer[512];
    serializeJson(doc, buffer);
    webserver.send(200, F("application/json"), buffer);
}

void handleFeedRequest()
{
    if (callFeederCallback)
    {
        callFeederCallback(1, 2000); // Call the feeder callback with 1 portion and 2000 ms
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

    // AJAX request from main page to update readings
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

        log_i("Saving settings...");
        String buf;
        
        FeedingSettings* feed01 = feederSettings->feed01;
        FeedingSettings* feed02 = feederSettings->feed02;

        feed01->hour = webserver.arg("h01").toInt();
        feed01->minute = webserver.arg("m01").toInt();
        feed01->portions = webserver.arg("p01").toInt();
        feed02->enabled = true; // always enabled

        feed02->hour = webserver.arg("h02").toInt();
        feed02->minute = webserver.arg("m02").toInt();
        feed02->portions = webserver.arg("p02").toInt();
        feed02->enabled = true; // always enabled

        // validate feeding times
        if (feed01->hour < 0 || feed01->hour > 23 || feed01->minute < 0 || feed01->minute > 59 ||
            feed02->hour < 0 || feed02->hour > 23 || feed02->minute < 0 || feed02->minute > 59 ||
            feed01->portions < 0 || feed01->portions > 10 || feed02->portions < 0 || feed02->portions > 10)
        {
            webserver.send(400, "text/plain", "Invalid feeding time or portions");
            return;
        }

        log_i("Feeder 1: %02d:%02d, portions: %d", feed01->hour, feed01->minute, feed01->portions);
        log_i("Feeder 2: %02d:%02d, portions: %d", feed02->hour, feed02->minute, feed02->portions);

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

// change webserver timeout and reset its timer
void webserver_settimeout(uint16_t timeoutSecs)
{
    if (webserverTimeout != timeoutSecs)
    {
        log_e("Set webserver timeout to %d secs.", timeoutSecs);
        webserverTimeout = timeoutSecs;
    }
}

// reset timeout countdown
void webserver_tickle()
{
    webserverRequestMillis = millis();
}

bool webserver_stop(bool force)
{
    if (!force && (millis() - webserverRequestMillis) < (webserverTimeout * 1000))
        return false;

    if (webserverRequestMillis > 0)
    {
        webserver.stop();
        webserverRequestMillis = 0;
        log_d(": Webserver stopped [%d]", millis());
    }
    return true;
}