#include "nvs_storage.h"
#include <Preferences.h>
#include <esp32-hal-log.h>

static Preferences preferences;

void nvs_init()
{
    log_i("NVS storage initialized");
}

bool nvs_save_feeder_config(FeederSettings* settings)
{
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        log_e("Failed to open NVS namespace for writing");
        return false;
    }

    // Save feed01 settings
    preferences.putInt("f1_hour", settings->feed01.hour);
    preferences.putInt("f1_minute", settings->feed01.minute);
    preferences.putChar("f1_portions", settings->feed01.portions);
    preferences.putULong("f1_duration", settings->feed01.duration);

    // Save feed02 settings
    preferences.putInt("f2_hour", settings->feed02.hour);
    preferences.putInt("f2_minute", settings->feed02.minute);
    preferences.putChar("f2_portions", settings->feed02.portions);
    preferences.putULong("f2_duration", settings->feed02.duration);

    // Mark config as valid
    preferences.putBool("feeder_valid", true);

    preferences.end();
    log_i("Feeder config saved to NVS");
    return true;
}

bool nvs_load_feeder_config(FeederSettings* settings)
{
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        log_e("Failed to open NVS namespace for reading");
        return false;
    }

    // Check if config is valid
    if (!preferences.getBool("feeder_valid", false)) {
        preferences.end();
        log_i("No valid feeder config in NVS");
        return false;
    }

    // Load feed01 settings
    settings->feed01.hour = preferences.getInt("f1_hour", settings->feed01.hour);
    settings->feed01.minute = preferences.getInt("f1_minute", settings->feed01.minute);
    settings->feed01.portions = preferences.getChar("f1_portions", settings->feed01.portions);
    settings->feed01.duration = preferences.getULong("f1_duration", settings->feed01.duration);
    settings->feed01.enabled = true;

    // Load feed02 settings
    settings->feed02.hour = preferences.getInt("f2_hour", settings->feed02.hour);
    settings->feed02.minute = preferences.getInt("f2_minute", settings->feed02.minute);
    settings->feed02.portions = preferences.getChar("f2_portions", settings->feed02.portions);
    settings->feed02.duration = preferences.getULong("f2_duration", settings->feed02.duration);
    settings->feed02.enabled = true;

    preferences.end();
    log_i("Feeder config loaded from NVS: feed01=%02d:%02d (%d portions), feed02=%02d:%02d (%d portions)",
          settings->feed01.hour, settings->feed01.minute, settings->feed01.portions,
          settings->feed02.hour, settings->feed02.minute, settings->feed02.portions);
    return true;
}

bool nvs_save_wifi_config(const char* ssid, const char* password)
{
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        log_e("Failed to open NVS namespace for writing");
        return false;
    }

    preferences.putString("wifi_ssid", ssid);
    preferences.putString("wifi_pass", password);
    preferences.putBool("wifi_valid", true);

    preferences.end();
    log_i("WiFi config saved to NVS");
    return true;
}

bool nvs_load_wifi_config(char* ssid, size_t ssid_len, char* password, size_t pass_len)
{
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        log_e("Failed to open NVS namespace for reading");
        return false;
    }

    if (!preferences.getBool("wifi_valid", false)) {
        preferences.end();
        log_i("No valid WiFi config in NVS");
        return false;
    }

    String stored_ssid = preferences.getString("wifi_ssid", "");
    String stored_pass = preferences.getString("wifi_pass", "");

    if (stored_ssid.length() == 0) {
        preferences.end();
        return false;
    }

    strncpy(ssid, stored_ssid.c_str(), ssid_len - 1);
    ssid[ssid_len - 1] = '\0';
    strncpy(password, stored_pass.c_str(), pass_len - 1);
    password[pass_len - 1] = '\0';

    preferences.end();
    log_i("WiFi config loaded from NVS: SSID=%s", ssid);
    return true;
}

bool nvs_save_mqtt_config(const char* server, int port, const char* user, const char* password)
{
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        log_e("Failed to open NVS namespace for writing");
        return false;
    }

    preferences.putString("mqtt_server", server);
    preferences.putInt("mqtt_port", port);
    preferences.putString("mqtt_user", user);
    preferences.putString("mqtt_pass", password);
    preferences.putBool("mqtt_valid", true);

    preferences.end();
    log_i("MQTT config saved to NVS");
    return true;
}

bool nvs_load_mqtt_config(char* server, size_t server_len, int* port, char* user, size_t user_len, char* password, size_t pass_len)
{
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        log_e("Failed to open NVS namespace for reading");
        return false;
    }

    if (!preferences.getBool("mqtt_valid", false)) {
        preferences.end();
        log_i("No valid MQTT config in NVS");
        return false;
    }

    String stored_server = preferences.getString("mqtt_server", "");
    String stored_user = preferences.getString("mqtt_user", "");
    String stored_pass = preferences.getString("mqtt_pass", "");
    int stored_port = preferences.getInt("mqtt_port", 1883);

    if (stored_server.length() == 0) {
        preferences.end();
        return false;
    }

    strncpy(server, stored_server.c_str(), server_len - 1);
    server[server_len - 1] = '\0';
    *port = stored_port;
    strncpy(user, stored_user.c_str(), user_len - 1);
    user[user_len - 1] = '\0';
    strncpy(password, stored_pass.c_str(), pass_len - 1);
    password[pass_len - 1] = '\0';

    preferences.end();
    log_i("MQTT config loaded from NVS: server=%s, port=%d", server, *port);
    return true;
}

bool nvs_has_wifi_config()
{
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        return false;
    }
    bool valid = preferences.getBool("wifi_valid", false);
    preferences.end();
    return valid;
}

bool nvs_has_mqtt_config()
{
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        return false;
    }
    bool valid = preferences.getBool("mqtt_valid", false);
    preferences.end();
    return valid;
}

bool nvs_has_feeder_config()
{
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        return false;
    }
    bool valid = preferences.getBool("feeder_valid", false);
    preferences.end();
    return valid;
}
