#include "gui.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define WINDOW_CLASS "DigitalClockWindow"
#define CLOCK_TIMER_ID 1
#define STOPWATCH_TIMER_ID 2

enum {
    ID_NAV_CLOCK = 100,
    ID_NAV_STOPWATCH,
    ID_NAV_SETTINGS,
    ID_CLOCK_PANEL,
    ID_TIME_LABEL,
    ID_AMPM_LABEL,
    ID_DATE_LABEL,
    ID_SYSTEM_LABEL,
    ID_OFFSET_LABEL,
    ID_FACE_DISPLAY,
    ID_ADJUST_HOUR_DOWN,
    ID_ADJUST_HOUR_UP,
    ID_ADJUST_MINUTE_DOWN,
    ID_ADJUST_MINUTE_UP,
    ID_ADJUST_SECOND_DOWN,
    ID_ADJUST_SECOND_UP,
    ID_STOPWATCH_PANEL,
    ID_STOPWATCH_LABEL,
    ID_STOPWATCH_DESCRIPTION,
    ID_STOPWATCH_START,
    ID_STOPWATCH_PAUSE,
    ID_STOPWATCH_RESET,
    ID_SETTINGS_PANEL,
    ID_FORMAT_LABEL,
    ID_FORMAT_COMBO,
    ID_THEME_LABEL,
    ID_THEME_COMBO,
    ID_FACE_LABEL,
    ID_FACE_COMBO,
    ID_RESET_TIME,
    ID_SETTINGS_NOTE
};

typedef enum {
    PAGE_CLOCK,
    PAGE_STOPWATCH,
    PAGE_SETTINGS
} Page;

typedef struct {
    Clock *clock;
    Stopwatch *stopwatch;
    Settings *settings;
    HWND window;
    HWND title_label;
    HWND subtitle_label;
    HWND nav_clock;
    HWND nav_stopwatch;
    HWND nav_settings;
    HWND clock_panel;
    HWND time_label;
    HWND ampm_label;
    HWND date_label;
    HWND system_label;
    HWND offset_label;
    HWND face_display;
    HWND adjust_buttons[6];
    HWND stopwatch_panel;
    HWND stopwatch_label;
    HWND stopwatch_description;
    HWND stopwatch_start;
    HWND stopwatch_pause;
    HWND stopwatch_reset;
    HWND settings_panel;
    HWND format_label;
    HWND format_combo;
    HWND theme_label;
    HWND theme_combo;
    HWND face_label;
    HWND face_combo;
    HWND reset_time;
    HWND settings_note;
    HFONT normal_font;
    HFONT small_font;
    HFONT title_font;
    HFONT clock_font;
    HFONT stopwatch_font;
    HBRUSH window_brush;
    HBRUSH card_brush;
    Page page;
} GuiState;

static const char *face_names[] = {
    "Minimal digital",
    "Classic digital",
    "Analog clock",
    "Roman numeral analog",
    "Minimal analog",
    "Creative digital"
};

static COLORREF window_color(const GuiState *state)
{
    return state->settings->theme == CLOCK_THEME_DARK
               ? RGB(17, 21, 33)
               : RGB(243, 245, 249);
}

static COLORREF card_color(const GuiState *state)
{
    return state->settings->theme == CLOCK_THEME_DARK
               ? RGB(26, 33, 48)
               : RGB(255, 255, 255);
}

static COLORREF primary_text_color(const GuiState *state)
{
    return state->settings->theme == CLOCK_THEME_DARK
               ? RGB(244, 246, 251)
               : RGB(23, 32, 51);
}

static COLORREF muted_text_color(const GuiState *state)
{
    return state->settings->theme == CLOCK_THEME_DARK
               ? RGB(157, 168, 189)
               : RGB(99, 112, 137);
}

static COLORREF accent_color(const GuiState *state)
{
    if (state->settings->theme == CLOCK_THEME_DARK) {
        return state->settings->face == CLOCK_FACE_CLASSIC_DIGITAL
                   ? RGB(123, 200, 255)
                   : RGB(151, 157, 255);
    }

    return state->settings->face == CLOCK_FACE_CLASSIC_DIGITAL
               ? RGB(23, 105, 170)
               : RGB(91, 101, 232);
}

static COLORREF clock_time_color(const GuiState *state)
{
    if (state->settings->face == CLOCK_FACE_CLASSIC_DIGITAL) {
        return state->settings->theme == CLOCK_THEME_DARK
                   ? RGB(123, 200, 255)
                   : RGB(23, 105, 170);
    }
    if (state->settings->face == CLOCK_FACE_CREATIVE) {
        return state->settings->theme == CLOCK_THEME_DARK
                   ? RGB(182, 160, 255)
                   : RGB(103, 65, 217);
    }
    return primary_text_color(state);
}

static void set_control_font(HWND control, HFONT font)
{
    SendMessageA(control, WM_SETFONT, (WPARAM)font, TRUE);
}

static HWND create_control(GuiState *state, const char *class_name,
                           const char *text, DWORD style, int control_id)
{
    HWND control = CreateWindowExA(
        0, class_name, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10,
        state->window, (HMENU)(INT_PTR)control_id,
        (HINSTANCE)GetWindowLongPtrA(state->window, GWLP_HINSTANCE), NULL);

    if (control == NULL) {
        return NULL;
    }

    set_control_font(control, state->normal_font);
    return control;
}

static HWND create_label(GuiState *state, const char *text, int control_id,
                         DWORD style)
{
    return create_control(state, "STATIC", text, style, control_id);
}

static HWND create_button(GuiState *state, const char *text, int control_id)
{
    return create_control(state, "BUTTON", text,
                          BS_OWNERDRAW | WS_TABSTOP, control_id);
}

static void place_control(HWND control, int x, int y, int width, int height)
{
    if (control != NULL) {
        MoveWindow(control, x, y, width, height, TRUE);
    }
}

static void show_control(HWND control, BOOL show)
{
    if (control != NULL) {
        ShowWindow(control, show ? SW_SHOW : SW_HIDE);
    }
}

static int update_theme_brushes(GuiState *state)
{
    if (state->window_brush != NULL) {
        DeleteObject(state->window_brush);
    }
    if (state->card_brush != NULL) {
        DeleteObject(state->card_brush);
    }
    state->window_brush = CreateSolidBrush(window_color(state));
    state->card_brush = CreateSolidBrush(card_color(state));
    return state->window_brush != NULL && state->card_brush != NULL;
}

static void release_graphics_resources(GuiState *state)
{
    if (state->normal_font != NULL) {
        DeleteObject(state->normal_font);
        state->normal_font = NULL;
    }
    if (state->small_font != NULL) {
        DeleteObject(state->small_font);
        state->small_font = NULL;
    }
    if (state->title_font != NULL) {
        DeleteObject(state->title_font);
        state->title_font = NULL;
    }
    if (state->clock_font != NULL) {
        DeleteObject(state->clock_font);
        state->clock_font = NULL;
    }
    if (state->stopwatch_font != NULL) {
        DeleteObject(state->stopwatch_font);
        state->stopwatch_font = NULL;
    }
    if (state->window_brush != NULL) {
        DeleteObject(state->window_brush);
        state->window_brush = NULL;
    }
    if (state->card_brush != NULL) {
        DeleteObject(state->card_brush);
        state->card_brush = NULL;
    }
}

static int is_analog_face(const GuiState *state)
{
    return state->settings->face == CLOCK_FACE_ANALOG ||
           state->settings->face == CLOCK_FACE_ROMAN_ANALOG ||
           state->settings->face == CLOCK_FACE_MINIMAL_ANALOG;
}

static void update_display(GuiState *state)
{
    ClockTime display_time;
    ClockTime system_time;
    char text[96];
    char time_text[32];
    char period[4] = "";
    long long offset = state->clock->offset_seconds;
    unsigned long long magnitude;
    int hour;

    if (offset < 0) {
        magnitude = (unsigned long long)(-(offset + 1)) + 1;
    } else {
        magnitude = (unsigned long long)offset;
    }

    if (clock_get_display_time(state->clock, &display_time)) {
        hour = display_time.hour;
        if (state->settings->format == CLOCK_FORMAT_12_HOUR) {
            snprintf(period, sizeof(period), "%s",
                     display_time.hour >= 12 ? "PM" : "AM");
            hour %= 12;
            if (hour == 0) {
                hour = 12;
            }
        }

        snprintf(time_text, sizeof(time_text), "%02d:%02d:%02d", hour,
                 display_time.minute, display_time.second);
        SetWindowTextA(state->time_label, time_text);
        SetWindowTextA(state->ampm_label, period);
        snprintf(text, sizeof(text), "%04d-%02d-%02d", display_time.year,
                 display_time.month, display_time.day);
        SetWindowTextA(state->date_label, text);
    } else {
        SetWindowTextA(state->time_label, "--:--:--");
        SetWindowTextA(state->ampm_label, "");
        SetWindowTextA(state->date_label, "Time unavailable");
    }

    if (clock_get_system_time(&system_time)) {
        snprintf(text, sizeof(text), "Actual system time: %02d:%02d:%02d",
                 system_time.hour, system_time.minute, system_time.second);
        SetWindowTextA(state->system_label, text);
    } else {
        SetWindowTextA(state->system_label,
                       "Actual system time unavailable");
    }

    snprintf(text, sizeof(text),
             "Application adjustment: %s%02llu:%02llu:%02llu",
             offset < 0 ? "-" : "+", magnitude / 3600,
             (magnitude / 60) % 60, magnitude % 60);
    SetWindowTextA(state->offset_label, text);

    InvalidateRect(state->face_display, NULL, TRUE);
}

static void update_stopwatch(GuiState *state)
{
    int64_t elapsed = stopwatch_elapsed_microseconds(state->stopwatch);
    long long centiseconds = (long long)(elapsed / 10000);
    long long hours = centiseconds / 360000;
    long long minutes = (centiseconds / 6000) % 60;
    long long seconds = (centiseconds / 100) % 60;
    long long fraction = centiseconds % 100;
    char text[40];

    if (hours > 0) {
        snprintf(text, sizeof(text), "%02lld:%02lld:%02lld.%02lld",
                 hours, minutes, seconds, fraction);
    } else {
        snprintf(text, sizeof(text), "%02lld:%02lld.%02lld",
                 minutes, seconds, fraction);
    }
    SetWindowTextA(state->stopwatch_label, text);
    SetWindowTextA(state->stopwatch_pause,
                   state->stopwatch->running
                       ? "Pause"
                       : (elapsed > 0 ? "Resume" : "Pause"));
    EnableWindow(state->stopwatch_pause,
                 state->stopwatch->running || elapsed > 0);
}

static void update_page_visibility(GuiState *state)
{
    BOOL show_clock = state->page == PAGE_CLOCK;
    BOOL show_stopwatch = state->page == PAGE_STOPWATCH;
    BOOL show_settings = state->page == PAGE_SETTINGS;
    int i;

    show_control(state->clock_panel, show_clock);
    show_control(state->time_label, show_clock && !is_analog_face(state));
    show_control(state->ampm_label, show_clock && !is_analog_face(state));
    show_control(state->date_label, show_clock);
    show_control(state->system_label, show_clock);
    show_control(state->offset_label, show_clock);
    show_control(state->face_display, show_clock && is_analog_face(state));
    for (i = 0; i < 6; ++i) {
        show_control(state->adjust_buttons[i], show_clock);
    }

    show_control(state->stopwatch_panel, show_stopwatch);
    show_control(state->stopwatch_label, show_stopwatch);
    show_control(state->stopwatch_description, show_stopwatch);
    show_control(state->stopwatch_start, show_stopwatch);
    show_control(state->stopwatch_pause, show_stopwatch);
    show_control(state->stopwatch_reset, show_stopwatch);

    show_control(state->settings_panel, show_settings);
    show_control(state->format_label, show_settings);
    show_control(state->format_combo, show_settings);
    show_control(state->theme_label, show_settings);
    show_control(state->theme_combo, show_settings);
    show_control(state->face_label, show_settings);
    show_control(state->face_combo, show_settings);
    show_control(state->reset_time, show_settings);
    show_control(state->settings_note, show_settings);

    RedrawWindow(state->window, NULL, NULL,
                 RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

static void draw_analog_face(GuiState *state, DRAWITEMSTRUCT *item)
{
    static const char *roman_numerals[] = {
        "I", "II", "III", "IV", "V", "VI",
        "VII", "VIII", "IX", "X", "XI", "XII"
    };
    const double full_turn = 6.283185307179586;
    RECT rect = item->rcItem;
    HDC dc = item->hDC;
    HBRUSH face_brush = CreateSolidBrush(card_color(state));
    HPEN outline_pen = CreatePen(
        PS_SOLID, 2,
        state->settings->theme == CLOCK_THEME_DARK
            ? RGB(78, 93, 120)
            : RGB(211, 220, 234));
    HGDIOBJ old_brush = SelectObject(dc, face_brush);
    HGDIOBJ old_pen = SelectObject(dc, outline_pen);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int radius = (width < height ? width : height) / 2 - 7;
    int center_x = (rect.left + rect.right) / 2;
    int center_y = (rect.top + rect.bottom) / 2;
    ClockTime now;
    int i;

    SetBkMode(dc, TRANSPARENT);
    Ellipse(dc, center_x - radius, center_y - radius,
            center_x + radius, center_y + radius);

    for (i = 0; i < 60; ++i) {
        double angle = i * full_turn / 60.0;
        int inner_radius = radius - (i % 5 == 0 ? 13 : 6);
        int outer_radius = radius - 3;
        HPEN tick_pen = CreatePen(
            PS_SOLID, i % 5 == 0 ? 2 : 1,
            muted_text_color(state));
        HGDIOBJ previous_pen = SelectObject(dc, tick_pen);

        MoveToEx(dc,
                 center_x + (int)(sin(angle) * inner_radius),
                 center_y - (int)(cos(angle) * inner_radius), NULL);
        LineTo(dc,
               center_x + (int)(sin(angle) * outer_radius),
               center_y - (int)(cos(angle) * outer_radius));
        SelectObject(dc, previous_pen);
        DeleteObject(tick_pen);
    }

    if (state->settings->face != CLOCK_FACE_MINIMAL_ANALOG) {
        HFONT number_font = CreateFontA(
            -17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        HGDIOBJ previous_font = SelectObject(dc, number_font);
        SetTextColor(dc, primary_text_color(state));

        for (i = 1; i <= 12; ++i) {
            char number[8];
            RECT number_rect;
            double angle = i * full_turn / 12.0;
            int text_x = center_x + (int)(sin(angle) * radius * 0.73);
            int text_y = center_y - (int)(cos(angle) * radius * 0.73);

            if (state->settings->face == CLOCK_FACE_ROMAN_ANALOG) {
                snprintf(number, sizeof(number), "%s", roman_numerals[i - 1]);
            } else {
                snprintf(number, sizeof(number), "%d", i);
            }

            number_rect.left = text_x - 17;
            number_rect.right = text_x + 17;
            number_rect.top = text_y - 10;
            number_rect.bottom = text_y + 10;
            DrawTextA(dc, number, -1, &number_rect,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        SelectObject(dc, previous_font);
        DeleteObject(number_font);
    }

    if (clock_get_display_time(state->clock, &now)) {
        double hour_angle =
            ((now.hour % 12) + now.minute / 60.0) * full_turn / 12.0;
        double minute_angle =
            (now.minute + now.second / 60.0) * full_turn / 60.0;
        double second_angle = now.second * full_turn / 60.0;
        HPEN hands[3];
        HGDIOBJ previous_pen;

        hands[0] = CreatePen(PS_SOLID, 5, primary_text_color(state));
        hands[1] = CreatePen(PS_SOLID, 3, primary_text_color(state));
        hands[2] = CreatePen(PS_SOLID, 1, accent_color(state));

        previous_pen = SelectObject(dc, hands[0]);
        MoveToEx(dc, center_x, center_y, NULL);
        LineTo(dc, center_x + (int)(sin(hour_angle) * radius * 0.45),
               center_y - (int)(cos(hour_angle) * radius * 0.45));

        SelectObject(dc, hands[1]);
        MoveToEx(dc, center_x, center_y, NULL);
        LineTo(dc, center_x + (int)(sin(minute_angle) * radius * 0.66),
               center_y - (int)(cos(minute_angle) * radius * 0.66));

        SelectObject(dc, hands[2]);
        MoveToEx(dc, center_x, center_y, NULL);
        LineTo(dc, center_x + (int)(sin(second_angle) * radius * 0.76),
               center_y - (int)(cos(second_angle) * radius * 0.76));

        SelectObject(dc, previous_pen);
        DeleteObject(hands[0]);
        DeleteObject(hands[1]);
        DeleteObject(hands[2]);
    }

    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);
    DeleteObject(outline_pen);
    DeleteObject(face_brush);
}

static void draw_panel(DRAWITEMSTRUCT *item)
{
    GuiState *state = (GuiState *)GetWindowLongPtrA(item->hwndItem, GWLP_USERDATA);
    RECT rect = item->rcItem;
    HBRUSH brush;
    HPEN pen;
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;

    if (state == NULL) {
        return;
    }

    brush = CreateSolidBrush(card_color(state));
    pen = CreatePen(
        PS_SOLID, 1,
        state->settings->theme == CLOCK_THEME_DARK
            ? RGB(42, 52, 72)
            : RGB(225, 230, 239));
    old_brush = SelectObject(item->hDC, brush);
    old_pen = SelectObject(item->hDC, pen);
    RoundRect(item->hDC, rect.left, rect.top, rect.right, rect.bottom, 18, 18);
    SelectObject(item->hDC, old_pen);
    SelectObject(item->hDC, old_brush);
    DeleteObject(pen);
    DeleteObject(brush);
}

static void draw_button(GuiState *state, DRAWITEMSTRUCT *item)
{
    RECT rect = item->rcItem;
    char text[96];
    int id = (int)item->CtlID;
    int primary = id == ID_STOPWATCH_START || id == ID_RESET_TIME ||
                  (id >= ID_NAV_CLOCK && id <= ID_NAV_SETTINGS &&
                   ((id == ID_NAV_CLOCK && state->page == PAGE_CLOCK) ||
                    (id == ID_NAV_STOPWATCH &&
                     state->page == PAGE_STOPWATCH) ||
                    (id == ID_NAV_SETTINGS &&
                     state->page == PAGE_SETTINGS)));
    COLORREF fill;
    COLORREF text_color;
    HBRUSH brush;
    HPEN pen;
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;
    HGDIOBJ old_font;

    if (primary) {
        fill = RGB(91, 101, 232);
        text_color = RGB(255, 255, 255);
    } else {
        fill = state->settings->theme == CLOCK_THEME_DARK
                   ? RGB(37, 46, 64)
                   : RGB(237, 241, 247);
        text_color = primary_text_color(state);
    }

    if (item->itemState & ODS_DISABLED) {
        text_color = muted_text_color(state);
    }
    if (item->itemState & ODS_SELECTED) {
        fill = state->settings->theme == CLOCK_THEME_DARK
                   ? RGB(53, 65, 89)
                   : RGB(218, 227, 240);
    }

    brush = CreateSolidBrush(fill);
    pen = CreatePen(PS_SOLID, 1, fill);
    old_brush = SelectObject(item->hDC, brush);
    old_pen = SelectObject(item->hDC, pen);
    RoundRect(item->hDC, rect.left + 1, rect.top + 1,
              rect.right - 1, rect.bottom - 1, 10, 10);
    SelectObject(item->hDC, old_pen);
    SelectObject(item->hDC, old_brush);

    GetWindowTextA(item->hwndItem, text, (int)sizeof(text));
    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, text_color);
    old_font = SelectObject(item->hDC, state->normal_font);
    DrawTextA(item->hDC, text, -1, &rect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(item->hDC, old_font);

    if (item->itemState & ODS_FOCUS) {
        InflateRect(&rect, -4, -4);
        DrawFocusRect(item->hDC, &rect);
    }

    DeleteObject(pen);
    DeleteObject(brush);
}

static void layout_controls(GuiState *state)
{
    RECT client;
    int width;
    int height;
    int margin = 26;
    int card_top = 124;
    int card_width;
    int card_bottom;
    int center;
    int i;
    const int button_width = 100;
    const int button_gap = 8;
    const int adjustment_width =
        button_width * 6 + button_gap * 5;

    GetClientRect(state->window, &client);
    width = client.right - client.left;
    height = client.bottom - client.top;
    center = width / 2;
    card_width = width - margin * 2;
    card_bottom = card_top + 370;

    place_control(state->title_label, margin, 18, width - margin * 2, 30);
    place_control(state->subtitle_label, margin, 49, width - margin * 2, 20);
    place_control(state->nav_clock, center - 190, 80, 120, 34);
    place_control(state->nav_stopwatch, center - 60, 80, 120, 34);
    place_control(state->nav_settings, center + 70, 80, 120, 34);

    place_control(state->clock_panel, margin, card_top, card_width, 370);
    place_control(state->time_label, margin + 30, card_top + 75,
                  card_width - 60, 80);
    place_control(state->ampm_label, margin + 30, card_top + 150,
                  card_width - 60, 27);
    place_control(state->date_label, margin + 30, card_top + 187,
                  card_width - 60, 28);
    place_control(state->system_label, margin + 30, card_top + 260,
                  card_width - 60, 23);
    place_control(state->offset_label, margin + 30, card_top + 288,
                  card_width - 60, 23);
    place_control(state->face_display, center - 112, card_top + 20, 224, 224);
    if (is_analog_face(state)) {
        place_control(state->date_label, margin + 30, card_top + 247,
                      card_width - 60, 24);
        place_control(state->system_label, margin + 30, card_top + 276,
                      card_width - 60, 22);
        place_control(state->offset_label, margin + 30, card_top + 303,
                      card_width - 60, 22);
    }

    for (i = 0; i < 6; ++i) {
        int x = center - adjustment_width / 2 +
                i * (button_width + button_gap);
        place_control(state->adjust_buttons[i], x, card_bottom + 16,
                      button_width, 38);
    }

    place_control(state->stopwatch_panel, margin, card_top, card_width, 370);
    place_control(state->stopwatch_label, margin + 30, card_top + 100,
                  card_width - 60, 78);
    place_control(state->stopwatch_description, margin + 30, card_top + 205,
                  card_width - 60, 25);
    place_control(state->stopwatch_start, center - 158, card_top + 270,
                  100, 42);
    place_control(state->stopwatch_pause, center - 50, card_top + 270,
                  100, 42);
    place_control(state->stopwatch_reset, center + 58, card_top + 270,
                  100, 42);

    place_control(state->settings_panel, margin, card_top, card_width, 440);
    place_control(state->format_label, margin + 52, card_top + 58, 155, 28);
    place_control(state->format_combo, margin + 225, card_top + 54, 220, 34);
    place_control(state->theme_label, margin + 52, card_top + 112, 155, 28);
    place_control(state->theme_combo, margin + 225, card_top + 108, 220, 34);
    place_control(state->face_label, margin + 52, card_top + 166, 155, 28);
    place_control(state->face_combo, margin + 225, card_top + 162, 260, 34);
    place_control(state->reset_time, margin + 52, card_top + 258, 230, 40);
    place_control(state->settings_note, margin + 52, card_top + 330,
                  card_width - 104, 45);

    if (height < card_top + 465) {
        int smaller_bottom = height - 58;
        for (i = 0; i < 6; ++i) {
            int x = center - adjustment_width / 2 +
                    i * (button_width + button_gap);
            place_control(state->adjust_buttons[i], x, smaller_bottom,
                          button_width, 38);
        }
    }
}

static int create_controls(GuiState *state)
{
    int i;
    static const char *adjust_labels[] = {
        "- 1 hour", "+ 1 hour", "- 1 min",
        "+ 1 min", "- 1 sec", "+ 1 sec"
    };
    static const char *format_names[] = {"12-hour", "24-hour"};
    static const char *theme_names[] = {"Light", "Dark"};

    state->title_label = create_label(state, "DIGITAL CLOCK", 0,
                                      SS_CENTER | SS_CENTERIMAGE);
    state->subtitle_label = create_label(state, "A focused time companion", 0,
                                         SS_CENTER | SS_CENTERIMAGE);
    state->nav_clock = create_button(state, "Clock", ID_NAV_CLOCK);
    state->nav_stopwatch = create_button(state, "Stopwatch", ID_NAV_STOPWATCH);
    state->nav_settings = create_button(state, "Settings", ID_NAV_SETTINGS);

    state->clock_panel = create_control(state, "STATIC", "",
                                        SS_OWNERDRAW, ID_CLOCK_PANEL);
    state->time_label = create_label(state, "--:--:--", ID_TIME_LABEL,
                                     SS_CENTER | SS_CENTERIMAGE);
    state->ampm_label = create_label(state, "AM", ID_AMPM_LABEL,
                                     SS_CENTER | SS_CENTERIMAGE);
    state->date_label = create_label(state, "", ID_DATE_LABEL,
                                     SS_CENTER | SS_CENTERIMAGE);
    state->system_label = create_label(state, "", ID_SYSTEM_LABEL,
                                       SS_CENTER | SS_CENTERIMAGE);
    state->offset_label = create_label(state, "", ID_OFFSET_LABEL,
                                       SS_CENTER | SS_CENTERIMAGE);
    state->face_display = create_control(state, "STATIC", "",
                                         SS_OWNERDRAW, ID_FACE_DISPLAY);

    for (i = 0; i < 6; ++i) {
        state->adjust_buttons[i] =
            create_button(state, adjust_labels[i], ID_ADJUST_HOUR_DOWN + i);
    }

    state->stopwatch_panel = create_control(state, "STATIC", "",
                                            SS_OWNERDRAW, ID_STOPWATCH_PANEL);
    state->stopwatch_label = create_label(state, "00:00.00",
                                          ID_STOPWATCH_LABEL,
                                          SS_CENTER | SS_CENTERIMAGE);
    state->stopwatch_description =
        create_label(state,
                     "Measured with a monotonic timer for reliable elapsed time.",
                     ID_STOPWATCH_DESCRIPTION,
                     SS_CENTER | SS_CENTERIMAGE);
    state->stopwatch_start =
        create_button(state, "Start", ID_STOPWATCH_START);
    state->stopwatch_pause =
        create_button(state, "Pause", ID_STOPWATCH_PAUSE);
    state->stopwatch_reset =
        create_button(state, "Reset", ID_STOPWATCH_RESET);

    state->settings_panel = create_control(state, "STATIC", "",
                                           SS_OWNERDRAW, ID_SETTINGS_PANEL);
    state->format_label = create_label(state, "Time format", ID_FORMAT_LABEL,
                                       SS_LEFT | SS_CENTERIMAGE);
    state->theme_label = create_label(state, "Appearance", ID_THEME_LABEL,
                                      SS_LEFT | SS_CENTERIMAGE);
    state->face_label = create_label(state, "Clock face", ID_FACE_LABEL,
                                     SS_LEFT | SS_CENTERIMAGE);
    state->format_combo = create_control(
        state, "COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        ID_FORMAT_COMBO);
    state->theme_combo = create_control(
        state, "COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        ID_THEME_COMBO);
    state->face_combo = create_control(
        state, "COMBOBOX", "", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        ID_FACE_COMBO);
    state->reset_time =
        create_button(state, "Reset to system time", ID_RESET_TIME);
    state->settings_note =
        create_label(state,
                     "Adjustments only affect this app. Your Windows system "
                     "clock is never changed.",
                     ID_SETTINGS_NOTE,
                     SS_LEFT | SS_WORDELLIPSIS);

    if (state->title_label == NULL || state->subtitle_label == NULL ||
        state->nav_clock == NULL || state->nav_stopwatch == NULL ||
        state->nav_settings == NULL || state->clock_panel == NULL ||
        state->time_label == NULL || state->ampm_label == NULL ||
        state->date_label == NULL || state->system_label == NULL ||
        state->offset_label == NULL || state->face_display == NULL ||
        state->stopwatch_panel == NULL || state->stopwatch_label == NULL ||
        state->stopwatch_description == NULL ||
        state->stopwatch_start == NULL || state->stopwatch_pause == NULL ||
        state->stopwatch_reset == NULL || state->settings_panel == NULL ||
        state->format_label == NULL || state->format_combo == NULL ||
        state->theme_label == NULL || state->theme_combo == NULL ||
        state->face_label == NULL || state->face_combo == NULL ||
        state->reset_time == NULL || state->settings_note == NULL) {
        return 0;
    }
    for (i = 0; i < 6; ++i) {
        if (state->adjust_buttons[i] == NULL) {
            return 0;
        }
    }

    for (i = 0; i < 2; ++i) {
        SendMessageA(state->format_combo, CB_ADDSTRING, 0,
                     (LPARAM)format_names[i]);
        SendMessageA(state->theme_combo, CB_ADDSTRING, 0,
                     (LPARAM)theme_names[i]);
    }
    for (i = 0; i < (int)(sizeof(face_names) / sizeof(face_names[0])); ++i) {
        SendMessageA(state->face_combo, CB_ADDSTRING, 0,
                     (LPARAM)face_names[i]);
    }

    SendMessageA(state->format_combo, CB_SETCURSEL,
                 (WPARAM)state->settings->format, 0);
    SendMessageA(state->theme_combo, CB_SETCURSEL,
                 (WPARAM)state->settings->theme, 0);
    SendMessageA(state->face_combo, CB_SETCURSEL,
                 (WPARAM)state->settings->face, 0);

    set_control_font(state->title_label, state->title_font);
    set_control_font(state->subtitle_label, state->small_font);
    set_control_font(state->time_label, state->clock_font);
    set_control_font(state->ampm_label, state->normal_font);
    set_control_font(state->date_label, state->normal_font);
    set_control_font(state->system_label, state->small_font);
    set_control_font(state->offset_label, state->small_font);
    set_control_font(state->stopwatch_label, state->stopwatch_font);
    set_control_font(state->stopwatch_description, state->small_font);
    set_control_font(state->settings_note, state->small_font);

    for (i = 0; i < 6; ++i) {
        set_control_font(state->adjust_buttons[i], state->small_font);
    }
    set_control_font(state->nav_clock, state->normal_font);
    set_control_font(state->nav_stopwatch, state->normal_font);
    set_control_font(state->nav_settings, state->normal_font);
    return 1;
}

static void apply_setting(GuiState *state, int control_id, HWND source)
{
    int selected = (int)SendMessageA(source, CB_GETCURSEL, 0, 0);

    if (selected == CB_ERR) {
        return;
    }

    if (control_id == ID_FORMAT_COMBO &&
        selected >= CLOCK_FORMAT_12_HOUR &&
        selected <= CLOCK_FORMAT_24_HOUR) {
        state->settings->format = (ClockFormat)selected;
        update_display(state);
    } else if (control_id == ID_THEME_COMBO &&
               selected >= CLOCK_THEME_LIGHT &&
               selected <= CLOCK_THEME_DARK) {
        state->settings->theme = (ClockTheme)selected;
        if (!update_theme_brushes(state)) {
            MessageBoxA(state->window, "Could not apply the selected theme.",
                        "Digital Clock", MB_OK | MB_ICONERROR);
        }
        RedrawWindow(state->window, NULL, NULL,
                     RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
    } else if (control_id == ID_FACE_COMBO &&
               selected >= CLOCK_FACE_MINIMAL_DIGITAL &&
               selected <= CLOCK_FACE_CREATIVE) {
        state->settings->face = (ClockFace)selected;
        layout_controls(state);
        update_page_visibility(state);
        InvalidateRect(state->time_label, NULL, TRUE);
        InvalidateRect(state->ampm_label, NULL, TRUE);
        InvalidateRect(state->face_display, NULL, TRUE);
    }
}

static void adjust_time(GuiState *state, int control_id)
{
    static const long long adjustments[] = {
        -3600, 3600, -60, 60, -1, 1
    };
    int index = control_id - ID_ADJUST_HOUR_DOWN;

    if (index >= 0 && index < (int)(sizeof(adjustments) /
                                    sizeof(adjustments[0]))) {
        clock_adjust_seconds(state->clock, adjustments[index]);
        update_display(state);
    }
}

static LRESULT CALLBACK window_proc(HWND window, UINT message,
                                    WPARAM w_param, LPARAM l_param)
{
    GuiState *state =
        (GuiState *)GetWindowLongPtrA(window, GWLP_USERDATA);

    if (message == WM_NCCREATE) {
        CREATESTRUCTA *create_info = (CREATESTRUCTA *)l_param;
        state = (GuiState *)create_info->lpCreateParams;
        state->window = window;
        SetWindowLongPtrA(window, GWLP_USERDATA, (LONG_PTR)state);
    }

    if (state == NULL) {
        return DefWindowProcA(window, message, w_param, l_param);
    }

    switch (message) {
    case WM_CREATE:
        if (!update_theme_brushes(state)) {
            MessageBoxA(window, "Could not initialize the application colors.",
                        "Digital Clock", MB_OK | MB_ICONERROR);
            return -1;
        }
        if (!create_controls(state)) {
            MessageBoxA(window, "Could not create all application controls.",
                        "Digital Clock", MB_OK | MB_ICONERROR);
            return -1;
        }
        update_page_visibility(state);
        update_display(state);
        update_stopwatch(state);
        if (SetTimer(window, CLOCK_TIMER_ID, 1000, NULL) == 0 ||
            SetTimer(window, STOPWATCH_TIMER_ID, 50, NULL) == 0) {
            MessageBoxA(window, "Could not start the application update timer.",
                        "Digital Clock", MB_OK | MB_ICONERROR);
            return -1;
        }
        return 0;

    case WM_GETMINMAXINFO: {
        MINMAXINFO *limits = (MINMAXINFO *)l_param;
        limits->ptMinTrackSize.x = 720;
        limits->ptMinTrackSize.y = 650;
        return 0;
    }

    case WM_SIZE:
        layout_controls(state);
        return 0;

    case WM_ERASEBKGND:
        {
            RECT client;
            GetClientRect(window, &client);
            FillRect((HDC)w_param, &client, state->window_brush);
        }
        return 1;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)w_param;
        HWND control = (HWND)l_param;
        int id = GetDlgCtrlID(control);
        int header = id == 0;
        int muted = id == ID_SYSTEM_LABEL ||
                    id == ID_OFFSET_LABEL ||
                    id == ID_STOPWATCH_DESCRIPTION ||
                    id == ID_SETTINGS_NOTE;

        SetBkMode(dc, OPAQUE);
        SetBkColor(dc, header ? window_color(state) : card_color(state));
        SetTextColor(dc, muted ? muted_text_color(state)
                               : (id == ID_TIME_LABEL
                                      ? clock_time_color(state)
                                      : primary_text_color(state)));
        return (LRESULT)(header ? state->window_brush : state->card_brush);
    }

    case WM_DRAWITEM: {
        DRAWITEMSTRUCT *item = (DRAWITEMSTRUCT *)l_param;
        int id = (int)item->CtlID;

        if (id == ID_CLOCK_PANEL || id == ID_STOPWATCH_PANEL ||
            id == ID_SETTINGS_PANEL) {
            SetWindowLongPtrA(item->hwndItem, GWLP_USERDATA, (LONG_PTR)state);
            draw_panel(item);
            return TRUE;
        }
        if (id == ID_FACE_DISPLAY) {
            draw_analog_face(state, item);
            return TRUE;
        }
        draw_button(state, item);
        return TRUE;
    }

    case WM_COMMAND: {
        int id = LOWORD(w_param);
        int notification = HIWORD(w_param);

        if (notification == CBN_SELCHANGE) {
            apply_setting(state, id, (HWND)l_param);
            return 0;
        }
        if (notification == BN_CLICKED) {
            if (id >= ID_ADJUST_HOUR_DOWN &&
                id <= ID_ADJUST_SECOND_UP) {
                adjust_time(state, id);
            } else if (id == ID_RESET_TIME) {
                clock_reset(state->clock);
                update_display(state);
            } else if (id == ID_STOPWATCH_START) {
                stopwatch_start(state->stopwatch);
                update_stopwatch(state);
            } else if (id == ID_STOPWATCH_PAUSE) {
                if (state->stopwatch->running) {
                    stopwatch_pause(state->stopwatch);
                } else {
                    stopwatch_resume(state->stopwatch);
                }
                update_stopwatch(state);
            } else if (id == ID_STOPWATCH_RESET) {
                stopwatch_reset(state->stopwatch);
                update_stopwatch(state);
            } else if (id == ID_NAV_CLOCK) {
                state->page = PAGE_CLOCK;
                update_page_visibility(state);
            } else if (id == ID_NAV_STOPWATCH) {
                state->page = PAGE_STOPWATCH;
                update_page_visibility(state);
            } else if (id == ID_NAV_SETTINGS) {
                state->page = PAGE_SETTINGS;
                update_page_visibility(state);
            }
            return 0;
        }
        break;
    }

    case WM_TIMER:
        if (w_param == CLOCK_TIMER_ID) {
            update_display(state);
        } else if (w_param == STOPWATCH_TIMER_ID) {
            update_stopwatch(state);
        }
        return 0;

    case WM_DESTROY:
        KillTimer(window, CLOCK_TIMER_ID);
        KillTimer(window, STOPWATCH_TIMER_ID);
        release_graphics_resources(state);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcA(window, message, w_param, l_param);
}

int gui_run(HINSTANCE instance, int show_command, Clock *clock,
            Stopwatch *stopwatch, Settings *settings)
{
    WNDCLASSEXA window_class;
    GuiState state;
    MSG message;
    int message_result;

    memset(&state, 0, sizeof(state));
    state.clock = clock;
    state.stopwatch = stopwatch;
    state.settings = settings;
    state.page = PAGE_CLOCK;

    state.normal_font = CreateFontA(
        -16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    state.small_font = CreateFontA(
        -13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    state.title_font = CreateFontA(
        -21, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    state.clock_font = CreateFontA(
        -60, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, "Consolas");
    state.stopwatch_font = CreateFontA(
        -48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_MODERN, "Consolas");

    if (state.normal_font == NULL || state.small_font == NULL ||
        state.title_font == NULL || state.clock_font == NULL ||
        state.stopwatch_font == NULL) {
        MessageBoxA(NULL, "Could not create the application fonts.",
                    "Digital Clock", MB_OK | MB_ICONERROR);
        release_graphics_resources(&state);
        return 1;
    }

    memset(&window_class, 0, sizeof(window_class));
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window_class.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    window_class.lpszClassName = WINDOW_CLASS;

    if (RegisterClassExA(&window_class) == 0) {
        MessageBoxA(NULL, "Could not register the application window.",
                    "Digital Clock", MB_OK | MB_ICONERROR);
        release_graphics_resources(&state);
        return 1;
    }

    state.window = CreateWindowExA(
        0, WINDOW_CLASS, "Digital Clock",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 780, 700,
        NULL, NULL, instance, &state);
    if (state.window == NULL) {
        release_graphics_resources(&state);
        MessageBoxA(NULL, "Could not create the application window.",
                    "Digital Clock", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(state.window, show_command);
    UpdateWindow(state.window);

    while ((message_result = (int)GetMessageA(&message, NULL, 0, 0)) > 0) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    if (message_result == -1) {
        MessageBoxA(state.window, "The Windows message loop failed.",
                    "Digital Clock", MB_OK | MB_ICONERROR);
        return 1;
    }
    return (int)message.wParam;
}
