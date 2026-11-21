#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include <stdint.h>

#define ROAD_1  1U
#define ROAD_2  2U

typedef enum {
    TL_OFF = 0,
    TL_RED,
    TL_AMBER,
    TL_GREEN
} TrafficState_t;

void traffic_init(void);
void traffic_set(uint8_t road, TrafficState_t state);
void traffic_all_off(void);

#endif /* INC_TRAFFIC_LIGHT_H_ */