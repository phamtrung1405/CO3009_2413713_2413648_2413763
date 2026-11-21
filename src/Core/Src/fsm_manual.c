#include "fsm_manual.h"

#include "button.h"
#include "fsm_config.h"
#include "global.h"
#include "lcd.h"
#include "traffic_light.h"

#include <stdint.h>

typedef struct {
    TrafficState_t road1;
    TrafficState_t road2;
    const char *description;
} ManualStep_t;

/* Step 0 is the safe entry state. There is no timed automatic transition. */
static const ManualStep_t manual_steps[] = {
    { TL_RED,   TL_RED,   "R1:RED  R2:RED"   },
    { TL_GREEN, TL_RED,   "R1:GRN  R2:RED"   },
    { TL_AMBER, TL_RED,   "R1:YEL  R2:RED"   },
    { TL_RED,   TL_RED,   "R1:RED  R2:RED"   },
    { TL_RED,   TL_GREEN, "R1:RED  R2:GRN"   },
    { TL_RED,   TL_AMBER, "R1:RED  R2:YEL"   },
    { TL_RED,   TL_RED,   "R1:RED  R2:RED"   }
};

#define MANUAL_STEP_COUNT ((uint8_t)(sizeof(manual_steps) / sizeof(manual_steps[0])))

static uint8_t current_step = 0U;

static void apply_manual_step(void)
{
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

    lcd_print_line(0U, "MODE: MANUAL");
    lcd_print_line(1U, step->description);
}

void fsm_manual_init(void)
{
    current_step = 0U;
    apply_manual_step();
}

void fsm_manual_run(void)
{
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
    }
}
