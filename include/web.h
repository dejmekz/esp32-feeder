#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "DHTesp.h"
#include "feeder.h"

typedef void (*FeederCallback)(int8_t portions, unsigned long feedingTime);

extern WebServer webserver;

void webserver_start(FeederSettings* setting);

void webserver_set_callback_feeder(FeederCallback callback);

#endif