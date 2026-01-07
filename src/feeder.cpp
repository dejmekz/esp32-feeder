#include "feeder.h"

int8_t CheckIfFeedingTime(FeedingSettings* setting, int hours, int minutes)
{
    if (setting->enabled && setting->hour == hours && setting->minute == minutes)
    {
        setting->enabled = false; // Disable after feeding
        return setting->portions;
    }
    return 0;
}


FeedingSettings* CheckFeedingTime(FeederSettings* schedule, int hours, int minutes)
{
    int8_t portions = CheckIfFeedingTime(&schedule->feed01, hours, minutes);
    if (portions > 0)
    {
        return &schedule->feed01;
    }

    portions = CheckIfFeedingTime(&schedule->feed02, hours, minutes);
    if (portions > 0)
    {
        return &schedule->feed02;
    }

    return nullptr;
}

void setFeeder01Schedule(FeederSettings* schedule, int hour, int minute, int8_t portions, bool enabled)
{
    schedule->feed01.hour = hour;
    schedule->feed01.minute = minute;
    schedule->feed01.portions = portions;
    schedule->feed01.enabled = enabled;
}

void setFeeder02Schedule(FeederSettings* schedule, int hour, int minute, int8_t portions, bool enabled)
{
    schedule->feed02.hour = hour;
    schedule->feed02.minute = minute;
    schedule->feed02.portions = portions;
    schedule->feed02.enabled = enabled;
}