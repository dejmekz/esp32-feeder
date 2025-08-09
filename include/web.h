#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "DHTesp.h"
#include "feeder.h"

// guard againt invalid remote configuration values
#define WEBSERVER_TIMEOUT_MIN_SECS 60
#define WEBSERVER_TIMEOUT_MAX_SECS 300

typedef void (*FeederCallback)(int8_t portions, unsigned long feedingTime);

extern WebServer webserver;

void webserver_start(FeederSettings* setting);

void webserver_set_callback_feeder(FeederCallback callback);

bool webserver_stop();

#endif