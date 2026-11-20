#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>

#define NUM_BUTTONS  4U

#define BTN_IDX_1    0U
#define BTN_IDX_2    1U
#define BTN_IDX_3    2U
#define BTN_IDX_4    3U

void button_init(void);
void button_read(void); /* Dang ky chay dinh ky moi 10 ms (1 tick) tren Scheduler */

uint8_t isButtonPressed(uint8_t index);
uint8_t isButtonLongPressed(uint8_t index);
uint8_t isButtonHeld(uint8_t index);

#endif /* INC_BUTTON_H_ */