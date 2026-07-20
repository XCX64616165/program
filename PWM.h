#ifndef __PWM_HEAD_
#define __PWM_HEAD_

#include "zf_common_debug.h"
#include "zf_driver_gpio.h"
#include "zf_driver_pwm.h"

// 电机引脚定义
#define MOTOR_A_PWM    PWM_TIM_A0_CH3_A28  			// 电机A PWM引脚
#define MOTOR_A_IN1    B8                	// 电机A 方向引脚1
#define MOTOR_A_IN2    B12                 	// 电机A 方向引脚2

#define MOTOR_B_PWM    PWM_TIM_A1_CH1_A31  			// 电机B PWM引脚
#define MOTOR_B_IN1    B27                 	// 电机B 方向引脚1
#define MOTOR_B_IN2    B6             	// 电机B 方向引脚2

#define MOTOR_PWM_FREQ 1000

typedef enum{
	MOTOR_STOP = 0,//停止
	MOTOR_FORWARD,//正转
	MOTOR_REVERSE//反转
	
 }motor_dir_enum;

 void Motor_Driver_Init(void);
 void Motor_LeftSetSpeed(motor_dir_enum dir, uint32 duty);
 void Motor_RightSetSpeed(motor_dir_enum dir, uint32 duty);
 void motor_forward(uint32 duty);
 void motor_stop();

#endif
