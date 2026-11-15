#include "settings.h"

void settings_init(Settings *settings)
{
    if (settings != 0) {
        settings->format = CLOCK_FORMAT_12_HOUR;
        settings->theme = CLOCK_THEME_DARK;
        settings->face = CLOCK_FACE_MINIMAL_DIGITAL;
    }
}
