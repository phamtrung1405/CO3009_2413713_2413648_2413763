#include "fsm_automatic.h"

#include "dht20.h"
#include "fsm_config.h"
#include "global.h"
#include "lcd.h"
#include "software_timer.h"
#include "traffic_light.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
static uint16_t env_tick_count = 0U;
static char last_env_line[17] = "";

#define AUTO_ENV_UPDATE_TICKS 100U /* 100 * 10 ms = 1 second */

static const char *get_phase_text(AutoPhase_t phase) {
  switch (phase) {
  case AUTO_R1_RED_R2_GREEN:
    return "AUTO R1:RD R2:GN";
  case AUTO_R1_RED_R2_YELLOW:
    return "AUTO R1:RD R2:YL";
  case AUTO_R1_GREEN_R2_RED:
    return "AUTO R1:GN R2:RD";
  case AUTO_R1_YELLOW_R2_RED:
    return "AUTO R1:YL R2:RD";
  case AUTO_ALL_RED_AFTER_R2:
  case AUTO_ALL_RED_AFTER_R1:
  default:
    return "AUTO R1:RD R2:RD";
  }
}

static void render_status_line(void) {
  lcd_print_line(0U, get_phase_text(current_phase));
}

static void format_env_line(char out[17]) {
  if (dht20_is_valid() == 0U) {
    (void)snprintf(out, 17U, "T:--.-C  H:--.-%%");
    return;
  }

  float temp = dht20_get_temperature();
  float hum = dht20_get_humidity();

  int32_t t10 = (temp >= 0.0f) ? (int32_t)(temp * 10.0f + 0.5f)
                               : (int32_t)(temp * 10.0f - 0.5f);
  int32_t h10 = (hum >= 0.0f) ? (int32_t)(hum * 10.0f + 0.5f) : 0;

  if (t10 > 999) {
    t10 = 999;
  } else if (t10 < -999) {
    t10 = -999;
  }

  if (h10 > 1000) {
    h10 = 1000;
  }

  int32_t t_abs = (t10 < 0) ? -t10 : t10;
  unsigned int t_int = (unsigned int)(t_abs / 10);
  unsigned int t_frac = (unsigned int)(t_abs % 10);
  unsigned int h_int = (unsigned int)(h10 / 10);
  unsigned int h_frac = (unsigned int)(h10 % 10);

  if (t10 < 0) {
    if (t_int >= 10U) {
      if (h10 >= 1000) {
        (void)snprintf(out, 17U, "T:-%2u.%1uC H:100%% ", t_int, t_frac);
      } else {
        (void)snprintf(out, 17U, "T:-%2u.%1uC H:%2u.%1u%%", t_int, t_frac,
                       h_int, h_frac);
      }
    } else {
      if (h10 >= 1000) {
        (void)snprintf(out, 17U, "T:-%1u.%1uC H:100.0%%", t_int, t_frac);
      } else {
        (void)snprintf(out, 17U, "T:-%1u.%1uC  H:%2u.%1u%%", t_int, t_frac,
                       h_int, h_frac);
      }
    }
  } else {
    if (h10 >= 1000) {
      (void)snprintf(out, 17U, "T:%2u.%1uC H:100.0%%", t_int, t_frac);
    } else {
      (void)snprintf(out, 17U, "T:%2u.%1uC  H:%2u.%1u%%", t_int, t_frac, h_int,
                     h_frac);
    }
  }
}

static void render_env_line_if_changed(void) {
  char line[17];
  format_env_line(line);

  if (strcmp(line, last_env_line) != 0) {
    (void)memcpy(last_env_line, line, sizeof(last_env_line));
    lcd_print_line(1U, line);
  }
}

static uint32_t get_clearance_ms(void) {
  uint32_t red_ms = (uint32_t)g_red_time_s * 1000U;
  uint32_t green_ms = (uint32_t)g_green_time_s * 1000U;
  uint32_t yellow_ms = (uint32_t)g_yellow_time_s * 1000U;

  if (red_ms <= (green_ms + yellow_ms)) {
    return 0U;
  }

  /* Red = green + yellow + clearance-before + clearance-after. */
  return (red_ms - green_ms - yellow_ms) / 2U;
}

static void apply_phase(AutoPhase_t phase) {
  uint32_t duration_ms = 0U;
  switch (phase) {
  case AUTO_R1_RED_R2_GREEN:
    /* R1 is kept red before R2 is allowed to become green. */
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
    /* R2 is explicitly red before R1 is allowed to become green. */
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

  render_status_line();
  setTimer(FSM_PHASE_TIMER, duration_ms);
}

void fsm_automatic_init(void) {
  current_phase = AUTO_R1_RED_R2_GREEN;
  phase_started = 0U;
  env_tick_count = AUTO_ENV_UPDATE_TICKS;
  last_env_line[0] = '\0';
  setTimer(FSM_PHASE_TIMER, 0U);
}

void fsm_automatic_run(void) {
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
    current_phase =
        (AutoPhase_t)(((uint32_t)current_phase + 1U) % AUTO_PHASE_COUNT);
    apply_phase(current_phase);
    return;
  }

  env_tick_count++;
  if (env_tick_count >= AUTO_ENV_UPDATE_TICKS) {
    env_tick_count = 0U;
    render_env_line_if_changed();
  }
}
