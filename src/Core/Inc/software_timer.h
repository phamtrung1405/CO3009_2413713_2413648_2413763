/*
 * software_timer.h
 *
 *  Created on: Nov 7, 2026
 *      Author: phamn
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#define MAX_TIMER 5
int timer_counter[MAX_TIMER];
int timer_flag[MAX_TIMER];
int TIMER_CYCLE;

void set_Timer(int index, int duration);
void timer_Run(void);

#endif /* INC_SOFTWARE_TIMER_H_ */
