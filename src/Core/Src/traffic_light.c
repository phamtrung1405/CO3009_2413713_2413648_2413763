#include "traffic_light.h"
#include "main.h"

/* Bang chan tri 2 tin hieu {PinA, PinB} -> 4 trang thai den.
 * Doi chieu va cap nhat lai theo ket qua do thuc te Buoc 3.2. */
static const GPIO_PinState TL_PIN_MAP[4][2] = {
    [TL_OFF]   = { GPIO_PIN_RESET, GPIO_PIN_RESET }, /* (0, 0) */
    [TL_RED]   = { GPIO_PIN_SET,   GPIO_PIN_SET   }, /* (1, 1) */
    [TL_AMBER] = { GPIO_PIN_RESET, GPIO_PIN_SET   }, /* (0, 1) */
    [TL_GREEN] = { GPIO_PIN_SET,   GPIO_PIN_RESET }  /* (1, 0) */
};

void traffic_init(void) {
    traffic_all_off();
}

void traffic_set(uint8_t road, TrafficState_t state) {
    if (state > TL_GREEN) {
        return;
    }

    GPIO_PinState pin_a = TL_PIN_MAP[state][0];
    GPIO_PinState pin_b = TL_PIN_MAP[state][1];

    if (road == ROAD_1) {
        HAL_GPIO_WritePin(TL1_A_GPIO_Port, TL1_A_Pin, pin_a);
        HAL_GPIO_WritePin(TL1_B_GPIO_Port, TL1_B_Pin, pin_b);
    } else if (road == ROAD_2) {
        HAL_GPIO_WritePin(TL2_A_GPIO_Port, TL2_A_Pin, pin_a);
        HAL_GPIO_WritePin(TL2_B_GPIO_Port, TL2_B_Pin, pin_b);
    }
}

void traffic_all_off(void) {
    traffic_set(ROAD_1, TL_OFF);
    traffic_set(ROAD_2, TL_OFF);
}