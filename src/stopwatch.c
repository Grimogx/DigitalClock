#include "stopwatch.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <time.h>
#endif

static int64_t monotonic_microseconds(void)
{
#ifdef _WIN32
    LARGE_INTEGER current_time;
    LARGE_INTEGER frequency;
    int64_t seconds;
    int64_t remainder;

    if (!QueryPerformanceCounter(&current_time) ||
        !QueryPerformanceFrequency(&frequency) || frequency.QuadPart == 0) {
        return 0;
    }
    seconds = current_time.QuadPart / frequency.QuadPart;
    remainder = current_time.QuadPart % frequency.QuadPart;
    return seconds * 1000000 +
           remainder * 1000000 / frequency.QuadPart;
#else
    struct timespec current_time;

    /* A monotonic clock is not affected by changes to the system time. */
    if (clock_gettime(CLOCK_MONOTONIC, &current_time) != 0) {
        return 0;
    }

    return (int64_t)current_time.tv_sec * 1000000 + current_time.tv_nsec / 1000;
#endif
}

void stopwatch_init(Stopwatch *stopwatch)
{
    if (stopwatch != NULL) {
        stopwatch->started_at_microseconds = 0;
        stopwatch->elapsed_microseconds = 0;
        stopwatch->running = 0;
    }
}

void stopwatch_start(Stopwatch *stopwatch)
{
    if (stopwatch != NULL && !stopwatch->running) {
        stopwatch->elapsed_microseconds = 0;
        stopwatch->started_at_microseconds = monotonic_microseconds();
        stopwatch->running = 1;
    }
}

void stopwatch_pause(Stopwatch *stopwatch)
{
    if (stopwatch != NULL && stopwatch->running) {
        stopwatch->elapsed_microseconds +=
            monotonic_microseconds() - stopwatch->started_at_microseconds;
        stopwatch->running = 0;
    }
}

void stopwatch_resume(Stopwatch *stopwatch)
{
    if (stopwatch != NULL && !stopwatch->running) {
        stopwatch->started_at_microseconds = monotonic_microseconds();
        stopwatch->running = 1;
    }
}

void stopwatch_reset(Stopwatch *stopwatch)
{
    stopwatch_init(stopwatch);
}

int64_t stopwatch_elapsed_microseconds(const Stopwatch *stopwatch)
{
    if (stopwatch == NULL) {
        return 0;
    }

    if (stopwatch->running) {
        return stopwatch->elapsed_microseconds +
               monotonic_microseconds() - stopwatch->started_at_microseconds;
    }

    return stopwatch->elapsed_microseconds;
}
