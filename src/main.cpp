#include <esp_task_wdt.h>
#include <Arduino.h>
#include <esp32-hal-log.h>
#include <esp32fota.h>
#include <freertos/semphr.h>

#include "DHTesp.h"

#include "config.h"
#include "wifi_handler.h"
#include "mqtt_handler.h"
#include "LedPin.h"
#include "MotorControl.h"
#include "web.h"
#include "feeder.h"
#include "Button.h"
#include "nvs_storage.h"

FeederSettings feederSchedule = {
    {FEEDING_HOUR_01, FEEDING_MINUTE_01, FEEDING_PORTIONS_01, FEEDING_DURATION, true},
    {FEEDING_HOUR_02, FEEDING_MINUTE_02, FEEDING_PORTIONS_02, FEEDING_DURATION, true},
    {0.0, 0.0}};

// Mutex for thread-safe access to feederSchedule
SemaphoreHandle_t scheduleMutex = NULL;

char device_name[16]; // 15 + 1 - delka řetězce + 1 :)
char mqtt_topic_sensors[31];
char mqtt_topic_will[27];
char mqtt_topic_cmnd[28];
char mqtt_topic_conf[28];
char mqtt_topic_feed[28];
char mqtt_topic_wifi[28];
char mqtt_topic_state[29];

bool is_wifi_connected = false;
bool is_mqtt_connected = false;

int previous_minutes = -1;

esp32FOTA esp32_FOTA(FOTA_FIRMWARE_TYPE, FIRMWARE_VERSION, false, true);

#ifdef USING_MOTOR
MotorControl motor(MOTOR_PIN_1, MOTOR_PIN_2);
#else
MotorControl motor(SERVO_PIN);
#endif

DHTesp dht;
LedPin LedRed(LED_RED_PIN, true);
LedPin LedBlue(LED_BLUE_PIN, true);
Button ButtonFeed(BUTTON_PIN, 100);

bool clockSynced = false;
unsigned long lastLoopCheckMillis = 0;

void mqtt_message_handler(char *topic, byte *message, unsigned int length);
void mqtt_handle_command(const String& message);
void mqtt_handle_config(const String& message);
void mqtt_setup_after_connect();
void synchronize_clock_from_ntp();
void setup_variables();
bool validateFeedingConfig(int hour, int minute, int8_t portions);

void publish_wifi_status(int minutes, const String& timestampMsg);
void publish_sensor_data(int minutes, const String& timestampMsg);
void checkIfExistNewFirmware(int minutes);

void mid_night_reset(int hours, int minutes);

void start_feeding(int8_t portions, unsigned long feedingTime);
void mqtt_publish_feeding_status(bool feeding);

void setup()
{
  esp_task_wdt_init(30, true); // 30 second timeout
  esp_task_wdt_add(NULL);

  // Initialize mutex for thread-safe schedule access
  scheduleMutex = xSemaphoreCreateMutex();
  if (scheduleMutex == NULL) {
    log_e("Failed to create mutex!");
  }

  setup_variables();

  // Initialize NVS and load saved configuration
  nvs_init();
  if (xSemaphoreTake(scheduleMutex, portMAX_DELAY) == pdTRUE) {
    nvs_load_feeder_config(&feederSchedule);
    xSemaphoreGive(scheduleMutex);
  }

  wifi_init(device_name, WIFI_SSID, WIFI_PASSWORD);
  mqtt_init(device_name, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASSWORD, mqtt_topic_will);
  mqtt_set_callback(mqtt_message_handler, mqtt_setup_after_connect);

  esp32_FOTA.setManifestURL(FOTA_MANIFEST_URL);
  esp32_FOTA.printConfig();

  motor.init();
  motor.onMotorChange(mqtt_publish_feeding_status);

  LedBlue.off();
  LedRed.off();

  // Serial.begin(115200);
  log_i("Starting Feeder...");
  dht.setup(DHT22_PIN, DHTesp::DHT22);

  webserver_start(&feederSchedule);
  webserver_set_callback_feeder(start_feeding);
}

void loop()
{
  esp_task_wdt_reset();

  webserver.handleClient();
  LedRed.blink();
  is_wifi_connected = wifi_loop();

  if (is_wifi_connected)
  {
    is_mqtt_connected = mqtt_loop();
  }
  else
  {
    is_mqtt_connected = false;
  }

  motor.loop();
  const bool motorRunning = motor.isRunning();

  if (motorRunning)
  {
    LedBlue.blink();
  }
  else
  {
    if (is_mqtt_connected)
    {
      LedBlue.on();
    }
    else
    {
      LedBlue.off();
    }
  }

  unsigned long currentMillis = millis();

  // Periodic check interval
  if (currentMillis - lastLoopCheckMillis >= LOOP_CHECK_INTERVAL_MS)
  {
    lastLoopCheckMillis = currentMillis;

    synchronize_clock_from_ntp();

    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100))
    {
      int minutes = timeinfo.tm_min;
      int hours = timeinfo.tm_hour;

      char timestampMsg[20];
      snprintf(timestampMsg, 20, "%02d.%02d.%04d %02d:%02d:%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
               hours, minutes, timeinfo.tm_sec);

      log_i("Current time: %s", timestampMsg);

      if (previous_minutes != minutes)
      {
        previous_minutes = minutes;

        int8_t portions_to_feed = 0;
        unsigned long duration_to_feed = 0;

        if (xSemaphoreTake(scheduleMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          mid_night_reset(hours, minutes);

          FeedingSettings *feedingSetting = CheckFeedingTime(&feederSchedule, hours, minutes);
          if (feedingSetting != nullptr)
          {
            log_i("Feeding time detected: %d portions at %02d:%02d for %d ms", feedingSetting->portions, feedingSetting->hour, feedingSetting->minute, feedingSetting->duration);
            portions_to_feed = feedingSetting->portions;
            duration_to_feed = feedingSetting->duration;
          }
          xSemaphoreGive(scheduleMutex);
        } else {
          log_e("Failed to acquire mutex for schedule check");
        }

        if (portions_to_feed > 0) {
          start_feeding(portions_to_feed, duration_to_feed);
        }

        checkIfExistNewFirmware(minutes);

        if (is_mqtt_connected)
        {
          publish_wifi_status(minutes, timestampMsg);
          publish_sensor_data(minutes, timestampMsg);
        }
      }
    }
    else
    {
      log_w("Failed to obtain time from ESP32");
    }

    log_i("Wifi: %d, MQTT: %d", is_wifi_connected, is_mqtt_connected);
  }

  uint8_t buttState = ButtonFeed.read();

  if (buttState == LOW && !motor.isRunning())
  {
    log_i("Button pressed, starting feeding...");
    start_feeding(1, FEEDING_DURATION); // Start feeding with 1 portion
  }

  vTaskDelay(100 / portTICK_PERIOD_MS); // Delay to avoid blocking the loop
  // This allows other tasks to run and prevents the loop from running too fast
}

void mqtt_setup_after_connect()
{
  log_i("MQTT connection established, setting up subscriptions...");
  mqtt_subscribe(mqtt_topic_cmnd, 0);
  mqtt_subscribe(mqtt_topic_conf, 0);
}

void synchronize_clock_from_ntp()
{
  if (clockSynced || !is_wifi_connected)
  {
    return;
  }

  configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", NTP_SERVER, NTP_FALLBACK_SERVER);

  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    clockSynced = true;
    log_i("Clock synced with NTP server");
    log_i("Current time: %s", asctime(&timeinfo));
  }
}

// Validate feeding configuration values
bool validateFeedingConfig(int hour, int minute, int8_t portions)
{
  return hour >= 0 && hour <= 23 &&
         minute >= 0 && minute <= 59 &&
         portions >= 1 && portions <= MAX_PORTIONS;
}

// Handle MQTT command messages (cmnd topic)
void mqtt_handle_command(const String& message)
{
  String cmd = message;
  cmd.toLowerCase();

  if (cmd == "on")
  {
    log_i("Received 'on' command");
    motor.start(1, FEEDING_DURATION);
  }
  else if (cmd == "off")
  {
    log_i("Received 'off' command");
    motor.stop();
  }
  else if (cmd.startsWith("feed "))
  {
    int portions = cmd.substring(5).toInt();
    if (portions >= 1 && portions <= MAX_PORTIONS) {
      log_i("Received 'feed' command with %d portions", portions);
      start_feeding(portions, FEEDING_DURATION);
    } else {
      log_e("Invalid portions value: %d (must be 1-%d)", portions, MAX_PORTIONS);
    }
  }
  else
  {
    log_e("Unknown command received: %s", cmd.c_str());
  }
}

// Handle MQTT configuration messages (conf topic)
void mqtt_handle_config(const String& message)
{
  log_i("Processing config update");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error)
  {
    log_e("JSON parse error: %s", error.c_str());
    return;
  }

  bool configUpdated = false;

  if (xSemaphoreTake(scheduleMutex, pdMS_TO_TICKS(100)) != pdTRUE) {
    log_e("Failed to acquire mutex for config update");
    return;
  }

  if (doc["feed01"].is<JsonObject>())
  {
    int hour = doc["feed01"]["hour"] | feederSchedule.feed01.hour;
    int minute = doc["feed01"]["minute"] | feederSchedule.feed01.minute;
    int8_t portions = doc["feed01"]["portions"] | feederSchedule.feed01.portions;

    if (validateFeedingConfig(hour, minute, portions))
    {
      feederSchedule.feed01.hour = hour;
      feederSchedule.feed01.minute = minute;
      feederSchedule.feed01.portions = portions;
      feederSchedule.feed01.enabled = true;
      configUpdated = true;
      log_i("feed01 updated: %02d:%02d, %d portions", hour, minute, portions);
    }
    else
    {
      log_e("feed01 validation failed: hour=%d, minute=%d, portions=%d", hour, minute, portions);
    }
  }

  if (doc["feed02"].is<JsonObject>())
  {
    int hour = doc["feed02"]["hour"] | feederSchedule.feed02.hour;
    int minute = doc["feed02"]["minute"] | feederSchedule.feed02.minute;
    int8_t portions = doc["feed02"]["portions"] | feederSchedule.feed02.portions;

    if (validateFeedingConfig(hour, minute, portions))
    {
      feederSchedule.feed02.hour = hour;
      feederSchedule.feed02.minute = minute;
      feederSchedule.feed02.portions = portions;
      feederSchedule.feed02.enabled = true;
      configUpdated = true;
      log_i("feed02 updated: %02d:%02d, %d portions", hour, minute, portions);
    }
    else
    {
      log_e("feed02 validation failed: hour=%d, minute=%d, portions=%d", hour, minute, portions);
    }
  }

  if (configUpdated)
  {
    nvs_save_feeder_config(&feederSchedule);
    log_i("Feeder schedule updated and saved to NVS");
  }
  else
  {
    log_w("No valid configuration updates applied.");
  }

  xSemaphoreGive(scheduleMutex);
}

void mqtt_message_handler(char *topic, byte *message, unsigned int length)
{
  log_i("Message arrived on topic: %s, length: %d", topic, length);

  // Protect against oversized messages
  if (length > MAX_MQTT_MESSAGE_SIZE) {
    log_e("MQTT message too large: %d bytes (max: %d)", length, MAX_MQTT_MESSAGE_SIZE);
    return;
  }

  String messageTemp((char*)message, length);

  log_i("Message: %s", messageTemp.c_str());

  if (strcmp(topic, mqtt_topic_cmnd) == 0)
  {
    mqtt_handle_command(messageTemp);
  }
  else if (strcmp(topic, mqtt_topic_conf) == 0)
  {
    mqtt_handle_config(messageTemp);
  }

  log_i("MQTT message processing complete.");
}

void setup_variables()
{
  uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address (6 bytes).
  uint32_t deviceId = (uint32_t)(chipid >> 16); // Use upper 4 bytes of MAC

  int written = snprintf(device_name, sizeof(device_name), "feeder-%08X", deviceId);
  if (written >= sizeof(device_name))
  {
    log_e("device_name buffer overflow! Required: %d, Available: %d", written + 1, sizeof(device_name));
  }

  // Build MQTT topics with overflow checking
  written = snprintf(mqtt_topic_sensors, sizeof(mqtt_topic_sensors), "feeder/%s/sensors", device_name);
  if (written >= sizeof(mqtt_topic_sensors))
  {
    log_e("mqtt_topic_sensors overflow! Required: %d, Available: %d", written + 1, sizeof(mqtt_topic_sensors));
  }

  written = snprintf(mqtt_topic_will, sizeof(mqtt_topic_will), "feeder/%s/LWT", device_name);
  if (written >= sizeof(mqtt_topic_will))
  {
    log_e("mqtt_topic_will overflow! Required: %d, Available: %d", written + 1, sizeof(mqtt_topic_will));
  }

  written = snprintf(mqtt_topic_cmnd, sizeof(mqtt_topic_cmnd), "feeder/%s/cmnd", device_name);
  if (written >= sizeof(mqtt_topic_cmnd))
  {
    log_e("mqtt_topic_cmnd overflow! Required: %d, Available: %d", written + 1, sizeof(mqtt_topic_cmnd));
  }

  written = snprintf(mqtt_topic_conf, sizeof(mqtt_topic_conf), "feeder/%s/conf", device_name);
  if (written >= sizeof(mqtt_topic_conf))
  {
    log_e("mqtt_topic_conf overflow! Required: %d, Available: %d", written + 1, sizeof(mqtt_topic_conf));
  }

  written = snprintf(mqtt_topic_feed, sizeof(mqtt_topic_feed), "feeder/%s/feed", device_name);
  if (written >= sizeof(mqtt_topic_feed))
  {
    log_e("mqtt_topic_feed overflow! Required: %d, Available: %d", written + 1, sizeof(mqtt_topic_feed));
  }

  written = snprintf(mqtt_topic_wifi, sizeof(mqtt_topic_wifi), "feeder/%s/wifi", device_name);
  if (written >= sizeof(mqtt_topic_wifi))
  {
    log_e("mqtt_topic_wifi overflow! Required: %d, Available: %d", written + 1, sizeof(mqtt_topic_wifi));
  }

  written = snprintf(mqtt_topic_state, sizeof(mqtt_topic_state), "feeder/%s/state", device_name);
  if (written >= sizeof(mqtt_topic_state))
  {
    log_e("mqtt_topic_state overflow! Required: %d, Available: %d", written + 1, sizeof(mqtt_topic_state));
  }

  log_i("Device name: %s", device_name);
}

void publish_wifi_status(int minutes, const String& timestampMsg)
{
  if ((minutes % 10 == 0))
  {
    JsonDocument doc;
    doc["uptime"] = millis() / 1000;
    doc["time"] = timestampMsg;

    doc["wifi"]["name"] = device_name;

    wifi_add_info(doc); // Add WiFi info to the JSON document
    mqtt_publish_json(mqtt_topic_wifi, doc);

    doc.clear(); // Clear the document for the next use

    doc["time"] = timestampMsg;
    doc["name"] = device_name;
    doc["firmware"] = FIRMWARE_VERSION;
    doc["type"] = FOTA_FIRMWARE_TYPE;

    // Read feeder schedule with mutex protection
    if (xSemaphoreTake(scheduleMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      doc["feed01"]["hour"] = feederSchedule.feed01.hour;
      doc["feed01"]["minute"] = feederSchedule.feed01.minute;
      doc["feed01"]["portions"] = feederSchedule.feed01.portions;
      doc["feed01"]["duration"] = feederSchedule.feed01.duration;

      doc["feed02"]["hour"] = feederSchedule.feed02.hour;
      doc["feed02"]["minute"] = feederSchedule.feed02.minute;
      doc["feed02"]["portions"] = feederSchedule.feed02.portions;
      doc["feed02"]["duration"] = feederSchedule.feed02.duration;
      xSemaphoreGive(scheduleMutex);
    }

    mqtt_publish_json(mqtt_topic_state, doc);
  }
}

void publish_sensor_data(int minutes, const String& timestampMsg)
{
  // Every 5 minutes, publish the status
  if ((minutes % 5 == 0))
  {
    TempAndHumidity newValues = dht.getTempAndHumidity();

    JsonDocument doc;
    doc["uptime"] = millis() / 1000;
    doc["time"] = timestampMsg;

    if (dht.getStatus() != 0)
    {
      log_e("DHT22 error status: %s", dht.getStatusString());
      doc["DHT22"]["error"] = dht.getStatusString();
      // Use last known good values if available
      if (xSemaphoreTake(scheduleMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        doc["DHT22"]["temperature"] = feederSchedule.dht22Data.temperature;
        doc["DHT22"]["humidity"] = feederSchedule.dht22Data.humidity;
        xSemaphoreGive(scheduleMutex);
      }
    }
    else
    {
      // Update stored values with mutex protection
      if (xSemaphoreTake(scheduleMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        feederSchedule.dht22Data = newValues;
        xSemaphoreGive(scheduleMutex);
      }
      doc["DHT22"]["temperature"] = newValues.temperature;
      doc["DHT22"]["humidity"] = newValues.humidity;
    }

    mqtt_publish_json(mqtt_topic_sensors, doc);
  }
}

void start_feeding(int8_t portions, unsigned long feedingTime)
{
  if (motor.isRunning() || portions <= 0)
  {
    log_d("No feeding scheduled or already running.");
    return;
  }

  log_i("Starting feeding process...");
  motor.start(portions, feedingTime);
}

void mid_night_reset(int hours, int minutes)
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
  if (!is_mqtt_connected)
    return;

  const char* state = feeding ? "ON" : "OFF";
  mqtt_publish(mqtt_topic_feed, state);
}

void checkIfExistNewFirmware(int minutes)
{
  if (minutes % 10 != 0 || !is_wifi_connected)
  {
    return;
  }

  esp32_FOTA.handle(); // Check for updates
}