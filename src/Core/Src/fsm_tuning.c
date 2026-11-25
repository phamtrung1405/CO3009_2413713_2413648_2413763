#include "fsm_tuning.h"

#include "button.h"
#include "fsm_config.h"
#include "global.h"
#include "lcd.h"

#include <stdio.h>
#include <stdint.h>

typedef enum {
    TUNE_RED = 0,
    TUNE_GREEN,
    TUNE_YELLOW,
    TUNE_FIELD_COUNT
} TuningField_t;

static TuningField_t selected_field = TUNE_RED;

static uint16_t clamp_u16(int32_t value, uint16_t min_value, uint16_t max_value)
{
    if (value < (int32_t)min_value) {
        return min_value;
    }
    if (value > (int32_t)max_value) {
        return max_value;
    }
    return (uint16_t)value;
}

static void normalize_timing(void)
{
    uint16_t max_green = FSM_MAX_GREEN_S;
    uint16_t max_yellow = FSM_MAX_YELLOW_S;

    g_green_time_s = clamp_u16(g_green_time_s, FSM_MIN_GREEN_S, max_green);
    g_yellow_time_s = clamp_u16(g_yellow_time_s, FSM_MIN_YELLOW_S, max_yellow);

    if (g_red_time_s > FSM_MAX_RED_S) {
        g_red_time_s = FSM_MAX_RED_S;
    }

    if (g_red_time_s < (uint16_t)(g_green_time_s + g_yellow_time_s + FSM_RED_MARGIN_S)) {
        g_red_time_s = (uint16_t)(g_green_time_s + g_yellow_time_s + FSM_RED_MARGIN_S);
    }
}

static void render_tuning_value(void)
{
    char line[17];
    const char *field_name = "RED";
    uint16_t value = g_red_time_s;

    switch (selected_field) {
    case TUNE_GREEN:
        field_name = "GREEN";
        value = g_green_time_s;
        break;
    case TUNE_YELLOW:
        field_name = "YELLOW";
        value = g_yellow_time_s;
        break;
    case TUNE_RED:
    default:
        field_name = "RED";
        value = g_red_time_s;
        break;
    }

    (void)snprintf(line, sizeof(line), "%s TIME:%3us", field_name, (unsigned int)value);
    lcd_print_line(1U, line);
}

static void render_tuning_screen(void)
{
    lcd_print_line(0U, "TUNING  ALL-RED ");
    render_tuning_value();
}

static void adjust_selected_field(int8_t delta)
{
    int32_t next_value;

    switch (selected_field) {
    case TUNE_RED: {
        uint16_t min_red = (uint16_t)(g_green_time_s + g_yellow_time_s + FSM_RED_MARGIN_S);
        if (min_red < (FSM_MIN_GREEN_S + FSM_MIN_YELLOW_S + FSM_RED_MARGIN_S)) {
            min_red = (uint16_t)(FSM_MIN_GREEN_S + FSM_MIN_YELLOW_S + FSM_RED_MARGIN_S);
        }
        next_value = (int32_t)g_red_time_s + delta;
        g_red_time_s = clamp_u16(next_value, min_red, FSM_MAX_RED_S);
        break;
    }

    case TUNE_GREEN: {
        uint16_t max_green = (uint16_t)(g_red_time_s - g_yellow_time_s - FSM_RED_MARGIN_S);
        if (max_green > FSM_MAX_GREEN_S) {
            max_green = FSM_MAX_GREEN_S;
        }
        next_value = (int32_t)g_green_time_s + delta;
        g_green_time_s = clamp_u16(next_value, FSM_MIN_GREEN_S, max_green);
        break;
    }

    case TUNE_YELLOW: {
        uint16_t max_yellow = (uint16_t)(g_red_time_s - g_green_time_s - FSM_RED_MARGIN_S);
        if (max_yellow > FSM_MAX_YELLOW_S) {
            max_yellow = FSM_MAX_YELLOW_S;
        }
        next_value = (int32_t)g_yellow_time_s + delta;
        g_yellow_time_s = clamp_u16(next_value, FSM_MIN_YELLOW_S, max_yellow);
        break;
    }

    default:
        break;
    }

    normalize_timing();
}

void fsm_tuning_init(void)
{
    selected_field = TUNE_RED;
    normalize_timing();
    render_tuning_screen();
}

void fsm_tuning_run(void)
{
    uint8_t redraw = 0U;

    if (g_app_mode != APP_MODE_TUNING) {
        return;
    }

    if (isButtonPressed(FSM_BUTTON_ACTION) != 0U) {
        selected_field = (TuningField_t)(((uint32_t)selected_field + 1U) % TUNE_FIELD_COUNT);
        redraw = 1U;
    }

    if (isButtonPressed(FSM_BUTTON_DECREASE) != 0U) {
        adjust_selected_field(-1);
        redraw = 1U;
    }

    if (isButtonPressed(FSM_BUTTON_INCREASE) != 0U) {
        adjust_selected_field(1);
        redraw = 1U;
    }

    if (redraw != 0U) {
        render_tuning_value();
    }
}
