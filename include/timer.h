/*
 * timer.h
 *
 *  Created on: 24 jan. 2018
 *      Author: erik.welander
 */

#ifndef TIMER_H_
#define TIMER_H_

//#include <stdint.h>

void timerInit();
void waitSyncTimer();

extern volatile uint32_t timerCounter;

#endif /* TIMER_H_ */
