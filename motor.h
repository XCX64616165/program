#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

void Motor_Init(void);
void YawMotor_SetSpeed(float speed);
void PitchMotor_SetSpeed(float speed);
void Motor_StopAll(void);

#endif
