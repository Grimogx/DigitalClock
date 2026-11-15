#include "clock.h"

#include <stddef.h>

static int convert_time(time_t timestamp, ClockTime *result)
{
    struct tm local_time;

    if (result == NULL) {
        return 0;
    }
#ifdef _WIN32
    if (localtime_s(&local_time, &timestamp) != 0) {
        return 0;
    }
#else
    if (localtime_r(&timestamp, &local_time) == NULL) {
        return 0;
    }
#endif

    result->hour = local_time.tm_hour;
    result->minute = local_time.tm_min;
    result->second = local_time.tm_sec;
    result->year = local_time.tm_year + 1900;
    result->month = local_time.tm_mon + 1;
    result->day = local_time.tm_mday;
    return 1;
}

void clock_init(Clock *clock)
{
    if (clock != NULL) {
        clock->offset_seconds = 0;
    }
}

void clock_adjust_seconds(Clock *clock, long long seconds)
{
    if (clock != NULL) {
        clock->offset_seconds += seconds;
    }
}

void clock_reset(Clock *clock)
{
    if (clock != NULL) {
        clock->offset_seconds = 0;
    }
}

int clock_get_system_time(ClockTime *result)
{
    return convert_time(time(NULL), result);
}

int clock_get_display_time(const Clock *clock, ClockTime *result)
{
    time_t timestamp = time(NULL);

    if (clock != NULL) {
        timestamp += (time_t)clock->offset_seconds;
    }

    return convert_time(timestamp, result);
}
