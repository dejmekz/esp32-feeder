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

