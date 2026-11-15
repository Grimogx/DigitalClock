#ifndef DIGITAL_STOPWATCH_H
#define DIGITAL_STOPWATCH_H

#include <stdint.h>

typedef struct {
    int64_t started_at_microseconds;
    int64_t elapsed_microseconds;
    int running;
} Stopwatch;

void stopwatch_init(Stopwatch *stopwatch);
void stopwatch_start(Stopwatch *stopwatch);
void stopwatch_pause(Stopwatch *stopwatch);
void stopwatch_resume(Stopwatch *stopwatch);
void stopwatch_reset(Stopwatch *stopwatch);
int64_t stopwatch_elapsed_microseconds(const Stopwatch *stopwatch);

#endif
