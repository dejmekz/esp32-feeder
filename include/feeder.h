#ifndef _FEEDERSETTING_H
#define _FEEDERSETTING_H

#include <Arduino.h>
#include "DHTesp.h"

struct FeedingSettings
{
  int hour;
  int minute;
  int8_t portions;
  unsigned long duration; // Duration in milliseconds to feed
  bool enabled;
};

struct FeederSettings
{
  FeedingSettings feed01;
  FeedingSettings feed02;

  TempAndHumidity dht22Data;
};

FeedingSettings* CheckFeedingTime(FeederSettings* schedule, int hours, int minutes);
int8_t CheckIfFeedingTime(FeedingSettings* setting, int hours, int minutes);


#endif