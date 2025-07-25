
#include <Arduino.h>
#include <esp32-hal-log.h>
#include <esp32fota.h>

#include "DHTesp.h"

#include "config.h"
#include "Esp32Mqtt.h"
#include "LedPin.h"
#include "MotorControl.h"
#include "web.h"
#include "feeder.h"
#include "Button.h"

FeederSettings feederSchedule = {{6, 30, 2, true}, {15, 0, 2, true}};

char DeviceName[16]; // 15 + 1 - delka řetězce + 1 :)
char mqtt_topic_sensors[31];
char mqtt_topic_will[27];
char mqtt_topic_cmnd[28];
char mqtt_topic_conf[28];
char mqtt_topic_feed[28];
char mqtt_topic_wifi[28];
char mqtt_topic_state[29];

esp32FOTA esp32_FOTA("esp32-feeder-motor", FIRMWARE_VERSION, false, true);

MotorControl motor(MOTOR_PIN_1, MOTOR_PIN_2);
Esp32Mqtt espMqtt(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, DeviceName);

DHTesp dht;
LedPin LedRed(LED_RED_PIN, true);
LedPin LedBlue(LED_BLUE_PIN, true);
Button ButtonFeed(BUTTON_PIN);

bool clockSynced = false;
bool mqttSynced = false;
unsigned long lastLoopCheckMillis = 0;

// Ensure webserver_start is declared as a function
extern void webserver_start(FeederSettings *settings);

void mqttCallback(char *topic, byte *message, unsigned int length);
void setupMqttAfterConnect();
void syncWithNTP();
void setupVariables();

void publishWifiStatus(int minutes, String timestampMsg);
void publishSensorData(int minutes, String timestampMsg);
void checkIfExistNewFirmware(int hours, int minutes);

void midNightReset(int hours, int minutes);

void startFeeding(int8_t portions);
void mqtt_publish_feeding_status(bool feeding);

void setup()
{
  esp32_FOTA.setManifestURL(FOTA_MANIFEST_URL);
  esp32_FOTA.printConfig();

  motor.init();
  motor.onMotorChange(mqtt_publish_feeding_status);

  LedBlue.off();
  LedRed.off();

  setupVariables();
  // Serial.begin(115200);
  log_i("Starting Feeder...");
  dht.setup(DHT22_PIN, DHTesp::DHT22);
  espMqtt.setup(mqtt_topic_will, mqttCallback);

  webserver_start(&feederSchedule);
  webserver_set_callback_feeder(startFeeding);
}

void loop()
{
  webserver.handleClient();
  LedRed.blink();
  espMqtt.loop(); // Zpracování MQTT zpráv
  motor.loop();

  syncWithNTP();
  setupMqttAfterConnect();

  if (motor.isRunning())
  {
    LedBlue.blink();
  }
  else
  {
    if (espMqtt.isWifiConnected())
    {
      LedBlue.on();
    }
    else
    {
      mqttSynced = false; // Reset MQTT sync state if WiFi is not connected
      LedBlue.off();
    }
  }

  unsigned long currentMillis = millis();

  // 30s interval check
  if (currentMillis - lastLoopCheckMillis >= 30000)
  {
    lastLoopCheckMillis = currentMillis;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
      return;

    int minutes = timeinfo.tm_min;
    int hours = timeinfo.tm_hour;

    char timestampMsg[20];
    snprintf(timestampMsg, 20, "%02d.%02d.%04d %02d:%02d:%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
             hours, minutes, timeinfo.tm_sec);

    log_i("Current time: %s", timestampMsg);

    midNightReset(hours, minutes);
    startFeeding(CheckFeedingTime(feederSchedule, hours, minutes));

    checkIfExistNewFirmware(hours, minutes);

    if (espMqtt.isMqttConnected())
    {
      publishWifiStatus(minutes, timestampMsg);
      publishSensorData(minutes, timestampMsg);

      if (minutes == 0) // Every hour
      {
        mqtt_publish_feeding_status(motor.isRunning());
      }
    }

    log_i("Wifi connected: %s", espMqtt.isWifiConnected() ? "Yes" : "No");
    log_i("MQTT connected: %s", espMqtt.isMqttConnected() ? "Yes" : "No");
  }

  uint8_t buttState = ButtonFeed.read();

  if (buttState == LOW)
  {
    log_i("Button pressed, starting feeding...");
    startFeeding(1); // Start feeding with 1 portion
  }

  vTaskDelay(100 / portTICK_PERIOD_MS); // Delay to avoid blocking the loop
  // This allows other tasks to run and prevents the loop from running too fast
}

void setupMqttAfterConnect()
{
  if (mqttSynced || !espMqtt.isMqttConnected())
  {
    return;
  }

  log_i("MQTT connection established, setting up subscriptions...");
  espMqtt.subscribe(mqtt_topic_cmnd);
  espMqtt.subscribe(mqtt_topic_conf);
  mqttSynced = true;
}

void syncWithNTP()
{
  if (clockSynced || espMqtt.isWifiConnected() == false)
  {
    return;
  }

  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", NTP_SERVER);
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    clockSynced = true;
    log_i("Clock synced with NTP server");
    log_i("Current time: %s", asctime(&timeinfo));
  }
}

void mqttCallback(char *topic, byte *message, unsigned int length)
{
  log_i("Message arrived on topic: %s", topic);
  String messageTemp;
  for (int i = 0; i < length; i++)
  {
    messageTemp += (char)message[i];
  }

  log_i("Message: %s", messageTemp.c_str());

  messageTemp.toLowerCase();

  if (strcmp(topic, mqtt_topic_cmnd) == 0)
  {
    // Check the payload
    if (messageTemp == "on")
    {
      log_i("Received 'on' command");
      motor.start(1); // Start feeding with 1 portion
      // Add your code to handle the 'on' command here
    }
    else if (messageTemp == "off")
    {
      log_i("Received 'off' command");
      // Add your code to handle the 'off' command here
      motor.stop(); // Stop feeding
    }
    else if (messageTemp.startsWith("feed "))
    {
      int portions = messageTemp.substring(5).toInt();
      log_i("Received 'feed' command with %d portions", portions);
      startFeeding(portions); // Start feeding with specified portions
    }
    else
    {
      log_e("Unknown command received");
    }
  }

  if (strcmp(topic, mqtt_topic_conf) == 0)
  {
    log_i("Received 'conf' command");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, messageTemp);
    if (!error)
    {
      if (doc["feed01"].is<JsonObject>())
      {
        feederSchedule.feed01.hour = doc["feed01"]["hour"] | feederSchedule.feed01.hour;
        feederSchedule.feed01.minute = doc["feed01"]["minute"] | feederSchedule.feed01.minute;
        feederSchedule.feed01.portions = doc["feed01"]["portions"] | feederSchedule.feed01.portions;
        feederSchedule.feed01.enabled = true; // Default to true if not specified
      }
      else
      {
        log_w("No feed01 configuration found in JSON.");
      }
      if (doc["feed02"].is<JsonObject>())
      {
        feederSchedule.feed02.hour = doc["feed02"]["hour"] | feederSchedule.feed02.hour;
        feederSchedule.feed02.minute = doc["feed02"]["minute"] | feederSchedule.feed02.minute;
        feederSchedule.feed02.portions = doc["feed02"]["portions"] | feederSchedule.feed02.portions;
        feederSchedule.feed02.enabled = true; // Default to true if not specified
      }
      else
      {
        log_w("No feed02 configuration found in JSON.");
      }
      log_i("Feeder schedule updated from JSON config.");
    }
    else
    {
      log_e("JSON parse error: %s", error.c_str());
    }
  }

  log_i("MQTT message processing complete.");
}

void setupVariables()
{
  uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
  uint32_t deviceId = 0;
  for (int i = 0; i < 25; i = i + 8)
  {
    deviceId |= ((chipid >> (40 - i)) & 0xff) << i;
  }

  snprintf(DeviceName, sizeof(DeviceName), "feeder_%08X", deviceId);
  snprintf(mqtt_topic_sensors, sizeof(mqtt_topic_sensors), "feeder/%s/sensors", DeviceName);
  snprintf(mqtt_topic_will, sizeof(mqtt_topic_will), "feeder/%s/LWT", DeviceName);
  snprintf(mqtt_topic_cmnd, sizeof(mqtt_topic_cmnd), "feeder/%s/cmnd", DeviceName);
  snprintf(mqtt_topic_conf, sizeof(mqtt_topic_conf), "feeder/%s/conf", DeviceName);
  snprintf(mqtt_topic_feed, sizeof(mqtt_topic_feed), "feeder/%s/feed", DeviceName);
  snprintf(mqtt_topic_wifi, sizeof(mqtt_topic_wifi), "feeder/%s/wifi", DeviceName);
  snprintf(mqtt_topic_state, sizeof(mqtt_topic_state), "feeder/%s/state", DeviceName);
  snprintf(DeviceName, sizeof(DeviceName), "feeder-%08X", deviceId);
}

void publishWifiStatus(int minutes, String timestampMsg)
{
  if ((minutes % 10 == 0))
  {
    JsonDocument doc;
    doc["uptime"] = millis() / 1000;
    doc["time"] = timestampMsg;

    espMqtt.addWifiInfo(doc); // Add WiFi info to the JSON document
    espMqtt.publishJson(mqtt_topic_wifi, doc);

    doc.clear(); // Clear the document for the next use

    doc["time"] = timestampMsg;

    doc["chip"]["revision"] = ESP.getChipRevision();
    doc["chip"]["model"] = ESP.getChipModel();
    doc["chip"]["cores"] = ESP.getChipCores();

    doc["heap"]["total"] = ESP.getHeapSize();
    doc["heap"]["free"] = ESP.getFreeHeap();
    doc["heap"]["minFree"] = ESP.getMinFreeHeap();
    doc["heap"]["maxAlloc"] = ESP.getMaxAllocHeap();

    doc["psram"]["size"] = ESP.getPsramSize();
    doc["psram"]["available"] = ESP.getFreePsram();
    doc["psram"]["minFree"] = ESP.getMinFreePsram();
    doc["psram"]["maxAlloc"] = ESP.getMaxAllocPsram();

    espMqtt.publishJson(mqtt_topic_state, doc);
  }
}

void publishSensorData(int minutes, String timestampMsg)
{
  // Every 5 minutes, publish the status
  if ((minutes % 5 == 0))
  {
    String msg = "";

    TempAndHumidity newValues = dht.getTempAndHumidity();

    if (dht.getStatus() != 0)
    {
      msg = "ERR";
      log_e("DHT22 error status: %s", dht.getStatusString());
    }
    else
    {
      msg = "OK";
      feederSchedule.dht22Data = newValues;
    }

    TempAndHumidity dhtData = feederSchedule.dht22Data;

    JsonDocument doc;
    doc["uptime"] = millis() / 1000;
    doc["time"] = timestampMsg;

    doc["DHT22"]["temperature"] = dhtData.temperature;
    doc["DHT22"]["humidity"] = dhtData.humidity;
    doc["DHT22"]["msg"] = "OK";

    // serializeJson(doc, Serial);
    // Serial.println();

    espMqtt.publishJson(mqtt_topic_sensors, doc);
  }
}

void startFeeding(int8_t portions)
{
  if (motor.isRunning() || portions <= 0)
  {
    log_d("No feeding scheduled or already running.");
    return;
  }

  log_i("Starting feeding process...");
  motor.start(portions);
}

void midNightReset(int hours, int minutes)
{
  if (hours != 0 || minutes != 0)
  {
    return; // Only reset at midnight
  }

  // Reset the feeder schedule at midnight
  feederSchedule.feed01.enabled = true;
  feederSchedule.feed02.enabled = true;
  motor.reset();
  log_i("Feeder schedule reset at midnight.");
}

void mqtt_publish_feeding_status(bool feeding)
{
  if (!espMqtt.isMqttConnected())
    return;

  char state[4];
  state[0] = 'O';
  state[3] = '\0';

  if (feeding)
  {
    state[1] = 'N';
    state[2] = '\0';
  }
  else
  {
    state[1] = 'F';
    state[2] = 'F';
  }

  espMqtt.publish(mqtt_topic_feed, state);
}

void checkIfExistNewFirmware(int hours, int minutes)
{
  //if (hours != 0 || !espMqtt.isWifiConnected())
  if (!espMqtt.isWifiConnected())
  {
    return;
  }

  if (minutes % 10 != 0)
  {
    return;
  }

  esp32_FOTA.handle(); // Check for updates
}