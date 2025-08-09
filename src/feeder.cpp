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
    if (CheckIfFeedingTime(schedule->feed01, hours, minutes) > 0)
    {
        return schedule->feed01;
    }

    if (CheckIfFeedingTime(schedule->feed02, hours, minutes) > 0)
    {
        return schedule->feed02;
    }

    return nullptr; // No feeding scheduled
}

void setFeeder01Schedule(FeederSettings* schedule, int hour, int minute, int8_t portions, bool enabled)
{
    FeedingSettings* feed01 = schedule->feed01;
    if (feed01 == nullptr)
    {        feed01 = new FeedingSettings();
        schedule->feed01 = feed01;
    }
    feed01->hour = hour;
    feed01->minute = minute;
    feed01->portions = portions;
    feed01->enabled = enabled;
}

void setFeeder02Schedule(FeederSettings* schedule, int hour, int minute, int8_t portions, bool enabled)
{
    FeedingSettings* feed02 = schedule->feed02;
    if (feed02 == nullptr)
    {
        feed02 = new FeedingSettings();
        schedule->feed02 = feed02;
    }
    feed02->hour = hour;
    feed02->minute = minute;
    feed02->portions = portions;
    feed02->enabled = enabled;
}