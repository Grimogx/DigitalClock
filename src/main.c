#include "clock.h"
#include "gui.h"
#include "settings.h"
#include "stopwatch.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance,
                   LPSTR command_line, int show_command)
{
    Clock clock;
    Stopwatch stopwatch;
    Settings settings;

    (void)previous_instance;
    (void)command_line;
    clock_init(&clock);
    stopwatch_init(&stopwatch);
    settings_init(&settings);

    return gui_run(instance, show_command, &clock, &stopwatch, &settings);
}
