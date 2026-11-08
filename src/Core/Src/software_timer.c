/*
 * software_timer.c
 *
 *  Created on: Nov 7, 2026
 *      Author: phamn
 */

#include "main.h"
#include "scheduler.h"
#include "software_timer.h"




void set_Timer(int index, int duration){
    timer_counter[index] = duration / TIMER_CYCLE;
    timer_flag[index] = 0;
}

void timer_Run(){
    for(int i = 0; i < 3; i++){
        if(timer_counter[i] > 0){
            timer_counter[i]--;
            if(timer_counter[i] == 0) timer_flag[i] = 1;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {       /* TIM2 ticks only */
        SCH_Update();
        timer_Run();
    }
}
