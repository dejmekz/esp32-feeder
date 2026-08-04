#include <unity.h>
#include "feeder.h"

// Pull in the real implementation directly: PlatformIO only wires src/
// into device build environments, not native test builds, so this is
// how the native suite exercises the actual production logic.
#include "../../src/feeder.cpp"

void setUp(void) {}
void tearDown(void) {}

static FeedingSettings makeSetting(int hour, int minute, int8_t portions, bool enabled)
{
    FeedingSettings s;
    s.hour = hour;
    s.minute = minute;
    s.portions = portions;
    s.duration = 10000;
    s.enabled = enabled;
    return s;
}

void test_check_if_feeding_time_matches(void)
{
    FeedingSettings s = makeSetting(6, 30, 3, true);
    int8_t portions = CheckIfFeedingTime(&s, 6, 30);

    TEST_ASSERT_EQUAL_INT8(3, portions);
    TEST_ASSERT_FALSE(s.enabled); // disabled after firing, so it won't refeed this minute
}

void test_check_if_feeding_time_no_match(void)
{
    FeedingSettings s = makeSetting(6, 30, 3, true);
    int8_t portions = CheckIfFeedingTime(&s, 6, 31);

    TEST_ASSERT_EQUAL_INT8(0, portions);
    TEST_ASSERT_TRUE(s.enabled);
}

void test_check_if_feeding_time_already_fired_today(void)
{
    FeedingSettings s = makeSetting(6, 30, 3, false); // already fired earlier today
    int8_t portions = CheckIfFeedingTime(&s, 6, 30);

    TEST_ASSERT_EQUAL_INT8(0, portions);
}

void test_check_feeding_time_returns_feed01_when_it_matches(void)
{
    FeederSettings schedule{};
    schedule.feed01 = makeSetting(6, 30, 3, true);
    schedule.feed02 = makeSetting(15, 0, 4, true);

    FeedingSettings *result = CheckFeedingTime(&schedule, 6, 30);

    TEST_ASSERT_EQUAL_PTR(&schedule.feed01, result);
    TEST_ASSERT_FALSE(schedule.feed01.enabled);
    TEST_ASSERT_TRUE(schedule.feed02.enabled);
}

void test_check_feeding_time_returns_feed02_when_it_matches(void)
{
    FeederSettings schedule{};
    schedule.feed01 = makeSetting(6, 30, 3, true);
    schedule.feed02 = makeSetting(15, 0, 4, true);

    FeedingSettings *result = CheckFeedingTime(&schedule, 15, 0);

    TEST_ASSERT_EQUAL_PTR(&schedule.feed02, result);
    TEST_ASSERT_FALSE(schedule.feed02.enabled);
    TEST_ASSERT_TRUE(schedule.feed01.enabled);
}

void test_check_feeding_time_returns_null_when_nothing_matches(void)
{
    FeederSettings schedule{};
    schedule.feed01 = makeSetting(6, 30, 3, true);
    schedule.feed02 = makeSetting(15, 0, 4, true);

    FeedingSettings *result = CheckFeedingTime(&schedule, 12, 0);

    TEST_ASSERT_NULL(result);
}

void test_check_feeding_time_feed01_takes_priority_on_same_slot(void)
{
    FeederSettings schedule{};
    schedule.feed01 = makeSetting(6, 30, 3, true);
    schedule.feed02 = makeSetting(6, 30, 4, true);

    FeedingSettings *result = CheckFeedingTime(&schedule, 6, 30);

    TEST_ASSERT_EQUAL_PTR(&schedule.feed01, result);
    TEST_ASSERT_TRUE(schedule.feed02.enabled); // never evaluated once feed01 matched
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_check_if_feeding_time_matches);
    RUN_TEST(test_check_if_feeding_time_no_match);
    RUN_TEST(test_check_if_feeding_time_already_fired_today);
    RUN_TEST(test_check_feeding_time_returns_feed01_when_it_matches);
    RUN_TEST(test_check_feeding_time_returns_feed02_when_it_matches);
    RUN_TEST(test_check_feeding_time_returns_null_when_nothing_matches);
    RUN_TEST(test_check_feeding_time_feed01_takes_priority_on_same_slot);
    return UNITY_END();
}
