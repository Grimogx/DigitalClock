#ifndef DIGITAL_SETTINGS_H
#define DIGITAL_SETTINGS_H

typedef enum {
    CLOCK_FORMAT_12_HOUR,
    CLOCK_FORMAT_24_HOUR
} ClockFormat;

typedef enum {
    CLOCK_THEME_LIGHT,
    CLOCK_THEME_DARK
} ClockTheme;

typedef enum {
    CLOCK_FACE_MINIMAL_DIGITAL,
    CLOCK_FACE_CLASSIC_DIGITAL,
    CLOCK_FACE_ANALOG,
    CLOCK_FACE_ROMAN_ANALOG,
    CLOCK_FACE_MINIMAL_ANALOG,
    CLOCK_FACE_CREATIVE
} ClockFace;

typedef struct {
    ClockFormat format;
    ClockTheme theme;
    ClockFace face;
} Settings;

void settings_init(Settings *settings);

#endif
