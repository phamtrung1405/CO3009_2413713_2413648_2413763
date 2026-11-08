/*
 * software_timer.c
 *
 *  Created on: Nov 7, 2026
 *      Author: phamn
 */

#include "main.h"
#include "scheduler.h"
#include "software_timer.h"


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {       /* TIM2 ticks only */
        SCH_Update();
    }
}
