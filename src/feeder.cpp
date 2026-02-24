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
