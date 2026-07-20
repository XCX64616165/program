#ifndef __RECEIVE_H
#define __RECEIVE_H

#include "main.h"

extern volatile int g_x_err;
extern volatile int g_y_err;
extern volatile uint8_t g_vision_new_data_flag;
extern volatile uint8_t g_target_lost_flag;

void Vision_ParseByte(uint8_t ch);
void Vision_Receive_Init(void);

#endif
