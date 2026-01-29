#ifndef NVS_STORAGE_H
#define NVS_STORAGE_H

#include <Arduino.h>
#include "feeder.h"

// NVS namespace for feeder settings
#define NVS_NAMESPACE "feeder"

// Initialize NVS storage
void nvs_init();

// Feeder configuration persistence
bool nvs_save_feeder_config(FeederSettings* settings);
bool nvs_load_feeder_config(FeederSettings* settings);

// WiFi configuration persistence
bool nvs_save_wifi_config(const char* ssid, const char* password);
bool nvs_load_wifi_config(char* ssid, size_t ssid_len, char* password, size_t pass_len);

// MQTT configuration persistence
bool nvs_save_mqtt_config(const char* server, int port, const char* user, const char* password);
bool nvs_load_mqtt_config(char* server, size_t server_len, int* port, char* user, size_t user_len, char* password, size_t pass_len);

// Check if NVS has valid configuration
bool nvs_has_wifi_config();
bool nvs_has_mqtt_config();
bool nvs_has_feeder_config();

#endif
