/*
 * stm32f10x_usart.h
 *
 *  Created on: 17 jan. 2018
 *      Author: erik.welander
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdint.h>
int debugInit();
void debugSend(char c);
void debugSendString(char *string);
char *int2str(uint32_t N, char *bufEnd);
char *hex2str(uint32_t N, char *bufEnd);
char *Dhex2str(uint32_t N);

#endif
