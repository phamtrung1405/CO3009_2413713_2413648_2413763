#include "main.h"
#include "scheduler.h"
#include "software_timer.h"

volatile uint32_t timer_counter[MAX_TIMER];
volatile uint8_t timer_flag[MAX_TIMER];

void software_timer_init(void)
{
    for (uint8_t i = 0U; i < MAX_TIMER; i++) {
        timer_counter[i] = 0U;
        timer_flag[i] = 0U;
    }
}

void setTimer(uint8_t index, uint32_t duration_ms)
{
    uint32_t ticks;
    if (index >= MAX_TIMER) return;
    ticks = (duration_ms == 0U) ? 0U : ((duration_ms + TICK_MS - 1U) / TICK_MS);
    timer_counter[index] = ticks;
    timer_flag[index] = 0U;
}

void set_Timer(uint8_t index, uint32_t duration_ms)
{
    setTimer(index, duration_ms);
}

void timer_Run(void)
{
    for (uint8_t i = 0U; i < MAX_TIMER; i++) {
        if (timer_counter[i] > 0U) {
            timer_counter[i]--;
            if (timer_counter[i] == 0U) timer_flag[i] = 1U;
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        SCH_Update();
        timer_Run();
    }
}
