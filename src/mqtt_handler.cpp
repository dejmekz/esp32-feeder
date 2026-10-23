#include "mqtt_handler.h"
#include <WiFi.h>
#include <esp32-hal-log.h>
#include "config.h"

WiFiClient _wifiClient;
PubSubClient _mqttClient(_wifiClient);

const char* willMessageOffline = "offline";
const char* willMessageOnline = "online";

static const char* _device_name;
static const char* _user;
static const char* _password;
static const char* _mqtt_topic_will;

static unsigned long lastMqttAttemptTime = 0;
static bool firstAttemptDone = false;

// Diagnostics, published in the wifi status
static uint16_t connectCount = 0;
static uint16_t failedCount = 0;
static int lastFailedState = 0;
static unsigned long lastConnectedAt = 0;

SimpleAction _callbackConnected = nullptr;

static void log_publish_error(const char* topic, size_t payloadLen)
{
    uint16_t bufSize = _mqttClient.getBufferSize();
    size_t packetSize = payloadLen + 2 + 1 + 2 + strlen(topic); // MQTT framing overhead
    if (!_mqttClient.connected()) {
        log_e("MQTT publish failed [%s]: not connected (state=%d)", topic, _mqttClient.state());
    } else if (packetSize > bufSize) {
        log_e("MQTT publish failed [%s]: packet %d bytes exceeds buffer %d bytes", topic, packetSize, bufSize);
    } else {
        log_e("MQTT publish failed [%s]: payload=%d bytes, packet~=%d bytes", topic, payloadLen, packetSize);
    }
}

void mqtt_init_after_connect()
{
    if (_callbackConnected)
    {
        _callbackConnected();
    }
}

void mqtt_init(const char* device_name,const char* server, int port, const char* user, const char* password, const char* mqtt_topic_will)
{
    _device_name = device_name;
    _user = user;
    _password = password;
    _mqtt_topic_will = mqtt_topic_will;

    _mqttClient.setServer(server, port);
    _mqttClient.setBufferSize(512);
}

void mqtt_connect()
{
    if (_mqttClient.connected())
    {
        return;
    }

    unsigned long currentTime = millis();
    // First attempt right after WiFi is up, then every MQTT_RECONNECT_INTERVAL_MS
    if (!firstAttemptDone || currentTime - lastMqttAttemptTime >= MQTT_RECONNECT_INTERVAL_MS)
    {
        firstAttemptDone = true;
        lastMqttAttemptTime = currentTime;

        if (_mqttClient.connect(_device_name, _user, _password, _mqtt_topic_will, 1, true, willMessageOffline)) {
            connectCount++;
            lastConnectedAt = millis() / 1000;
            log_i("MQTT connected");
            mqtt_publish_online();
            mqtt_init_after_connect();
        }
        else
        {
            failedCount++;
            lastFailedState = _mqttClient.state();
            log_e("MQTT connection failed, rc=%d", lastFailedState);
        }
    }
}

bool mqtt_loop()
{
    if (!_mqttClient.loop())
    {
        mqtt_connect();
    }
    return _mqttClient.connected();  // Return actual current status
}

bool mqtt_publish_json(const char* topic, JsonDocument& doc) {
    char buffer[512];
    size_t len = serializeJson(doc, buffer);

    // Check if serialization succeeded
    if (len == 0) {
        log_e("MQTT JSON serialization failed for topic: %s", topic);
        return false;
    }

    // Don't publish truncated JSON - it would be invalid
    if (len >= sizeof(buffer)) {
        log_e("MQTT JSON too large (%d bytes), not publishing to: %s", len, topic);
        return false;
    }

    // Attempt to publish and log result
    // Retained: openHAB reads the last state/wifi/sensors values after its restart
    bool result = _mqttClient.publish(topic, (const uint8_t*)buffer, len, true);
    if (!result) {
        log_publish_error(topic, len);
    }

    return result;
}

// Retained "online" in the LWT topic. Published after connect and repeated with
// the periodic status, so a cleared retained message is restored (openHAB reads
// it after restart).
bool mqtt_publish_online()
{
    bool result = _mqttClient.publish(_mqtt_topic_will, willMessageOnline, true);
    if (!result) {
        log_publish_error(_mqtt_topic_will, strlen(willMessageOnline));
    }
    return result;
}

void mqtt_add_info(JsonDocument &doc)
{
    doc["mqtt"]["connects"] = connectCount;    // since boot
    doc["mqtt"]["failed"] = failedCount;       // failed attempts since boot
    doc["mqtt"]["lastRc"] = lastFailedState;   // state of the last failed attempt
    doc["mqtt"]["connectedAt"] = lastConnectedAt; // uptime (s) of the last connect
}

bool mqtt_publish(const char *topic, const char *payload)
{
    bool result = _mqttClient.publish(topic, payload);
    if (!result) {
        log_publish_error(topic, strlen(payload));
    }
    return result;
}

void mqtt_subscribe(const char* topic, uint8_t qos) {
    _mqttClient.subscribe(topic, qos);
}

void mqtt_set_callback(MQTT_CALLBACK_SIGNATURE, SimpleAction callbackConnected) {
    _mqttClient.setCallback(callback);
    _callbackConnected = callbackConnected;
}

void mqtt_disconnect() {
    if (_mqttClient.connected()) {
        if (!_mqttClient.publish(_mqtt_topic_will, willMessageOffline, true)) {
            log_publish_error(_mqtt_topic_will, strlen(willMessageOffline));
        }
        _mqttClient.disconnect();
        log_i("MQTT disconnected");
    }
}