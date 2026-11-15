#ifndef DIGITAL_CLOCK_H
#define DIGITAL_CLOCK_H

#include <time.h>

typedef struct {
    long long offset_seconds;
} Clock;

typedef struct {
    int hour;
    int minute;
    int second;
    int year;
    int month;
    int day;
} ClockTime;

void clock_init(Clock *clock);
void clock_adjust_seconds(Clock *clock, long long seconds);
void clock_reset(Clock *clock);
int clock_get_system_time(ClockTime *result);
int clock_get_display_time(const Clock *clock, ClockTime *result);

#endif
