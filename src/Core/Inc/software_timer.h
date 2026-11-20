#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include <stdint.h>

#define MAX_TIMER       5U
#define FSM_PHASE_TIMER 0U

extern volatile uint32_t timer_counter[MAX_TIMER];
extern volatile uint8_t timer_flag[MAX_TIMER];

void software_timer_init(void);
void setTimer(uint8_t index, uint32_t duration_ms);
void set_Timer(uint8_t index, uint32_t duration_ms);
void timer_Run(void);

#endif /* INC_SOFTWARE_TIMER_H_ */
