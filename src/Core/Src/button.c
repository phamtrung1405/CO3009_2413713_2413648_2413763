/*
 * button.c
 *
 *  Created on: Nov 7, 2026
 *      Author: phamn
 */


#include "button.h"
#include "main.h"

/* Muc logic khi nhan nut (kiem tra lai o Buoc 3.3).
 * Neu nut nhan la Active-High, dao nguoc 2 macro ben duoi. */
#define BUTTON_PRESSED_STATE     GPIO_PIN_RESET
#define BUTTON_NORMAL_STATE      GPIO_PIN_SET

/* Nguong nhan giu tinh theo tick 10 ms */
#define LONG_PRESS_TICKS         100U  /* 100 * 10 ms = 1000 ms */
#define LONG_PRESS_REPEAT_TICKS  30U   /* 30  * 10 ms = 300 ms  */

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} ButtonPin_t;

static const ButtonPin_t BTN_HW[NUM_BUTTONS] = {
    { BTN_1_GPIO_Port, BTN_1_Pin },
    { BTN_2_GPIO_Port, BTN_2_Pin },
    { BTN_3_GPIO_Port, BTN_3_Pin },
    { BTN_4_GPIO_Port, BTN_4_Pin }
};

static GPIO_PinState key_reg0[NUM_BUTTONS];
static GPIO_PinState key_reg1[NUM_BUTTONS];
static GPIO_PinState key_reg2[NUM_BUTTONS];
static GPIO_PinState key_reg3[NUM_BUTTONS];

static uint16_t long_press_cnt[NUM_BUTTONS];
static uint8_t  short_press_flag[NUM_BUTTONS];
static uint8_t  long_press_flag[NUM_BUTTONS];

void button_init(void) {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        key_reg0[i] = BUTTON_NORMAL_STATE;
        key_reg1[i] = BUTTON_NORMAL_STATE;
        key_reg2[i] = BUTTON_NORMAL_STATE;
        key_reg3[i] = BUTTON_NORMAL_STATE;
        long_press_cnt[i]   = 0;
        short_press_flag[i] = 0;
        long_press_flag[i]  = 0;
    }
}

void button_read(void) {
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        key_reg0[i] = key_reg1[i];
        key_reg1[i] = key_reg2[i];
        key_reg2[i] = HAL_GPIO_ReadPin(BTN_HW[i].port, BTN_HW[i].pin);

        /* Chong doi 30 ms (3 mau lien tiep dong nhat) */
        if ((key_reg0[i] == key_reg1[i]) && (key_reg1[i] == key_reg2[i])) {
            if (key_reg3[i] != key_reg2[i]) {
                key_reg3[i] = key_reg2[i];

                if (key_reg2[i] == BUTTON_PRESSED_STATE) {
                    short_press_flag[i] = 1;
                    long_press_cnt[i]   = LONG_PRESS_TICKS;
                } else {
                    long_press_cnt[i]  = 0;
                    long_press_flag[i] = 0;
                }
            } else {
                if (key_reg2[i] == BUTTON_PRESSED_STATE) {
                    if (long_press_cnt[i] > 0) {
                        long_press_cnt[i]--;
                        if (long_press_cnt[i] == 0) {
                            long_press_flag[i] = 1;
                            long_press_cnt[i]  = LONG_PRESS_REPEAT_TICKS;
                        }
                    }
                }
            }
        }
    }
}

uint8_t isButtonPressed(uint8_t index) {
    if (index >= NUM_BUTTONS) {
        return 0;
    }
    if (short_press_flag[index]) {
        short_press_flag[index] = 0;
        return 1;
    }
    return 0;
}

uint8_t isButtonLongPressed(uint8_t index) {
    if (index >= NUM_BUTTONS) {
        return 0;
    }
    if (long_press_flag[index]) {
        long_press_flag[index] = 0;
        return 1;
    }
    return 0;
}

uint8_t isButtonHeld(uint8_t index) {
    if (index >= NUM_BUTTONS) {
        return 0;
    }
    return (key_reg3[index] == BUTTON_PRESSED_STATE) ? 1U : 0U;
}