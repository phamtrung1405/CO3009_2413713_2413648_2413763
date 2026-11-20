#include "fsm_mode_manager.h"

#include "button.h"
#include "fsm_automatic.h"
#include "fsm_config.h"
#include "fsm_manual.h"
#include "fsm_tuning.h"
#include "global.h"
#include "software_timer.h"
#include "traffic_light.h"

static void set_both_roads_red(void)
{
    traffic_set(ROAD_1, TL_RED);
    traffic_set(ROAD_2, TL_RED);
}

static void enter_mode(AppMode_t next_mode)
{
    set_both_roads_red();
    setTimer(FSM_PHASE_TIMER, 0U);
    g_app_mode = next_mode;

    switch (g_app_mode) {
    case APP_MODE_AUTO:
        fsm_automatic_init();
        break;
    case APP_MODE_MANUAL:
        fsm_manual_init();
        break;
    case APP_MODE_TUNING:
        fsm_tuning_init();
        break;
    default:
        g_app_mode = APP_MODE_AUTO;
        fsm_automatic_init();
        break;
    }
}

void fsm_mode_manager_init(void)
{
    g_app_mode = APP_MODE_AUTO;
    set_both_roads_red();
    setTimer(FSM_PHASE_TIMER, 0U);

    fsm_automatic_init();
}

void fsm_mode_manager_run(void)
{
    if (isButtonPressed(FSM_BUTTON_MODE) == 0U) {
        return;
    }

    switch (g_app_mode) {
    case APP_MODE_AUTO:
        enter_mode(APP_MODE_MANUAL);
        break;
    case APP_MODE_MANUAL:
        enter_mode(APP_MODE_TUNING);
        break;
    case APP_MODE_TUNING:
    default:
        enter_mode(APP_MODE_AUTO);
        break;
    }
}
