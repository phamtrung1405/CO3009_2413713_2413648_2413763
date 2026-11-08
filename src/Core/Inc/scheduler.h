/*
 * scheduler.h
 *
 *  Created on: Nov 7, 2026
 *      Author: phamn
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include "main.h"
#include <string.h>

typedef struct {
	void (*pTask)(void);
	uint32_t Delay;
	uint32_t Period;
	uint8_t RunMe;
	uint32_t TaskID;
} sTask;

#define SCH_MAX_TASKS   40
#define NO_TASK_ID      0u
#define TICK_MS         10u

void     SCH_Init(void);
void     SCH_Update(void);          /* call from the timer ISR      */
void     SCH_Dispatch_Tasks(void);  /* call from while(1) in main() */
/* DELAY, PERIOD in ticks; PERIOD = 0 -> one-shot task */
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);  /* 1 = deleted, 0 = not found */


#endif /* INC_SCHEDULER_H_ */
