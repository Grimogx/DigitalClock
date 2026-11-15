#ifndef DIGITAL_GUI_H
#define DIGITAL_GUI_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "clock.h"
#include "settings.h"
#include "stopwatch.h"

int gui_run(HINSTANCE instance, int show_command, Clock *clock,
            Stopwatch *stopwatch, Settings *settings);

#endif
