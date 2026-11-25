#include "fsm_manual.h"

#include "button.h"
#include "dht20.h"
#include "fsm_config.h"
#include "global.h"
#include "lcd.h"
#include "traffic_light.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  TrafficState_t road1;
  TrafficState_t road2;
  const char *description;
} ManualStep_t;

/* Step 0 is the safe entry state. There is no timed automatic transition. */
static const ManualStep_t manual_steps[] = {
    {TL_RED, TL_RED, "MAN  R1:RD R2:RD"},
    {TL_GREEN, TL_RED, "MAN  R1:GN R2:RD"},
    {TL_AMBER, TL_RED, "MAN  R1:YL R2:RD"},
    {TL_RED, TL_RED, "MAN  R1:RD R2:RD"},
    {TL_RED, TL_GREEN, "MAN  R1:RD R2:GN"},
    {TL_RED, TL_AMBER, "MAN  R1:RD R2:YL"},
    {TL_RED, TL_RED, "MAN  R1:RD R2:RD"}};

#define MANUAL_STEP_COUNT                                                      \
  ((uint8_t)(sizeof(manual_steps) / sizeof(manual_steps[0])))
#define MANUAL_ENV_UPDATE_TICKS 100U /* 100 * 10 ms = 1 second */

static uint8_t current_step = 0U;
static uint16_t env_tick_count = 0U;
static char last_env_line[17] = "";

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

static void apply_manual_step(void) {
  const ManualStep_t *step = &manual_steps[current_step];

  /* Set the opposing road red before activating a green output. */
  if (step->road1 == TL_GREEN) {
    traffic_set(ROAD_2, TL_RED);
    traffic_set(ROAD_1, TL_GREEN);
  } else if (step->road2 == TL_GREEN) {
    traffic_set(ROAD_1, TL_RED);
    traffic_set(ROAD_2, TL_GREEN);
  } else {
    traffic_set(ROAD_1, step->road1);
    traffic_set(ROAD_2, step->road2);
  }

  lcd_print_line(0U, step->description);
}

void fsm_manual_init(void) {
  current_step = 0U;
  /* Defer line-1 refresh to the next 10 ms tick to keep each tick under 10 ms.
   */
  env_tick_count = MANUAL_ENV_UPDATE_TICKS - 1U;
  last_env_line[0] = '\0';
  apply_manual_step();
}

void fsm_manual_run(void) {
  if (g_app_mode != APP_MODE_MANUAL) {
    return;
  }

  if (isButtonPressed(FSM_BUTTON_ACTION) != 0U) {
    if (current_step >= (uint8_t)(MANUAL_STEP_COUNT - 1U)) {
      current_step = 1U;
    } else {
      current_step++;
    }
    apply_manual_step();
    return;
  }

  env_tick_count++;
  if (env_tick_count >= MANUAL_ENV_UPDATE_TICKS) {
    env_tick_count = 0U;
    render_env_line_if_changed();
  }
}
