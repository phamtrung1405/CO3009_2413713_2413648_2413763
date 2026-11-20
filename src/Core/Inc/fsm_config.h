#ifndef INC_FSM_CONFIG_H_
#define INC_FSM_CONFIG_H_

#include "button.h"

#define FSM_BUTTON_MODE       BTN_IDX_1
#define FSM_BUTTON_ACTION     BTN_IDX_2
#define FSM_BUTTON_DECREASE   BTN_IDX_3
#define FSM_BUTTON_INCREASE   BTN_IDX_4

#define FSM_MIN_GREEN_S       3U
#define FSM_MAX_GREEN_S       90U
#define FSM_MIN_YELLOW_S      1U
#define FSM_MAX_YELLOW_S     10U
#define FSM_MAX_RED_S        180U
#define FSM_MIN_CLEARANCE_S   1U
#define FSM_RED_MARGIN_S      (2U * FSM_MIN_CLEARANCE_S)

#endif /* INC_FSM_CONFIG_H_ */
