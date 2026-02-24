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


#endif
