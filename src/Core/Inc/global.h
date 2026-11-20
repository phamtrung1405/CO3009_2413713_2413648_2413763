#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>

typedef enum {
    APP_MODE_AUTO = 0,
    APP_MODE_MANUAL,
    APP_MODE_TUNING
} AppMode_t;

extern AppMode_t g_app_mode;
extern uint16_t g_red_time_s;
extern uint16_t g_green_time_s;
extern uint16_t g_yellow_time_s;

#endif /* INC_GLOBAL_H_ */
