#ifndef _FEEDERSETTING_H
#define _FEEDERSETTING_H

#include <Arduino.h>
#include "DHTesp.h"

struct FeedingSettings
{
  int hour;
  int minute;
  int8_t portions;
  bool enabled;
};

struct FeederSettings
{
  FeedingSettings feed01;
  FeedingSettings feed02;

  TempAndHumidity dht22Data;
};

int8_t CheckFeedingTime(FeederSettings &schedule, int hours, int minutes);
int8_t CheckIfFeedingTime(FeedingSettings &setting, int hours, int minutes);

void setFeeder01Schedule(FeederSettings &schedule, int hour, int minute, int8_t portions, bool enabled);
void setFeeder02Schedule(FeederSettings &schedule, int hour, int minute, int8_t portions, bool enabled);

#endif