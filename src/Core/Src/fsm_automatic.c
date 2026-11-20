#include "fsm_automatic.h"

#include "fsm_config.h"
#include "dht20.h"
#include "global.h"
#include "lcd.h"
#include "software_timer.h"
#include "traffic_light.h"

#include <stdint.h>

typedef enum {
    AUTO_R1_RED_R2_GREEN = 0,
    AUTO_R1_RED_R2_YELLOW,
    AUTO_ALL_RED_AFTER_R2,
    AUTO_R1_GREEN_R2_RED,
    AUTO_R1_YELLOW_R2_RED,
    AUTO_ALL_RED_AFTER_R1,
    AUTO_PHASE_COUNT
} AutoPhase_t;

static AutoPhase_t current_phase = AUTO_R1_RED_R2_GREEN;
static uint8_t phase_started = 0U;
static uint8_t environment_page = 0U;
static uint16_t display_tick_count = 0U;

#define AUTO_LCD_PAGE_TICKS 300U /* 300 * 10 ms = 3 seconds */

static const char *get_phase_text(AutoPhase_t phase)
{
    switch (phase) {
    case AUTO_R1_RED_R2_GREEN:  return "R1:RED  R2:GRN";
    case AUTO_R1_RED_R2_YELLOW: return "R1:RED  R2:YEL";
    case AUTO_R1_GREEN_R2_RED:  return "R1:GRN  R2:RED";
    case AUTO_R1_YELLOW_R2_RED: return "R1:YEL  R2:RED";
    case AUTO_ALL_RED_AFTER_R2:
    case AUTO_ALL_RED_AFTER_R1:
    default:                    return "R1:RED  R2:RED";
    }
}

static void render_status_page(void)
{
    lcd_print_line(0U, "MODE: AUTO");
    lcd_print_line(1U, get_phase_text(current_phase));
}

static void render_environment_page(void)
{
    if (dht20_is_valid() == 0U) {
        lcd_print_line(0U, "DHT20: WAITING");
        lcd_print_line(1U, "NO VALID SAMPLE");
        return;
    }

    lcd_print_line(0U, "TEMP (C):");
    lcd_set_cursor(0U, 9U);
    lcd_send_float(dht20_get_temperature(), 1U);
    lcd_send_char('C');

    lcd_print_line(1U, "HUMIDITY:");
    lcd_set_cursor(1U, 9U);
    lcd_send_float(dht20_get_humidity(), 1U);
    lcd_send_char('%');
}

static void render_current_page(void)
{
    if (environment_page != 0U) {
        render_environment_page();
    } else {
        render_status_page();
    }
}

static uint32_t get_clearance_ms(void)
{
    uint32_t red_ms = (uint32_t)g_red_time_s * 1000U;
    uint32_t green_ms = (uint32_t)g_green_time_s * 1000U;
    uint32_t yellow_ms = (uint32_t)g_yellow_time_s * 1000U;

    if (red_ms <= (green_ms + yellow_ms)) {
        return 0U;
    }

    return (red_ms - green_ms - yellow_ms) / 2U;
}

static void apply_phase(AutoPhase_t phase)
{
    uint32_t duration_ms = 0U;
    switch (phase) {
    case AUTO_R1_RED_R2_GREEN:
        traffic_set(ROAD_1, TL_RED);
        traffic_set(ROAD_2, TL_GREEN);
        duration_ms = (uint32_t)g_green_time_s * 1000U;
        break;

    case AUTO_R1_RED_R2_YELLOW:
        traffic_set(ROAD_1, TL_RED);
        traffic_set(ROAD_2, TL_AMBER);
        duration_ms = (uint32_t)g_yellow_time_s * 1000U;
        break;

    case AUTO_ALL_RED_AFTER_R2:
        traffic_set(ROAD_1, TL_RED);
        traffic_set(ROAD_2, TL_RED);
        duration_ms = get_clearance_ms();
        break;

    case AUTO_R1_GREEN_R2_RED:
        traffic_set(ROAD_2, TL_RED);
        traffic_set(ROAD_1, TL_GREEN);
        duration_ms = (uint32_t)g_green_time_s * 1000U;
        break;

    case AUTO_R1_YELLOW_R2_RED:
        traffic_set(ROAD_1, TL_AMBER);
        traffic_set(ROAD_2, TL_RED);
        duration_ms = (uint32_t)g_yellow_time_s * 1000U;
        break;

    case AUTO_ALL_RED_AFTER_R1:
    default:
        traffic_set(ROAD_1, TL_RED);
        traffic_set(ROAD_2, TL_RED);
        duration_ms = get_clearance_ms();
        break;
    }

    if (environment_page == 0U) {
        render_status_page();
    }
    setTimer(FSM_PHASE_TIMER, duration_ms);
}

void fsm_automatic_init(void)
{
    current_phase = AUTO_R1_RED_R2_GREEN;
    phase_started = 0U;
    environment_page = 0U;
    display_tick_count = 0U;
    setTimer(FSM_PHASE_TIMER, 0U);
}

void fsm_automatic_run(void)
{
    if (g_app_mode != APP_MODE_AUTO) {
        return;
    }

    if (phase_started == 0U) {
        apply_phase(current_phase);
        phase_started = 1U;
        return;
    }

    if (timer_flag[FSM_PHASE_TIMER] != 0U) {
        timer_flag[FSM_PHASE_TIMER] = 0U;
        current_phase = (AutoPhase_t)(((uint32_t)current_phase + 1U) % AUTO_PHASE_COUNT);
        apply_phase(current_phase);
    }

    display_tick_count++;
    if (display_tick_count >= AUTO_LCD_PAGE_TICKS) {
        display_tick_count = 0U;
        environment_page = (uint8_t)!environment_page;
        render_current_page();
    }
}
