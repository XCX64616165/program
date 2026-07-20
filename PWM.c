#include "PWM.h"

void Motor_Driver_Init(void)
{
	/*使能引脚置高
	(gpio_pin_enum pin, gpio_dir_enum dir, 
	const uint8 dat, gpio_mode_enum mode);
	*/
	gpio_init(B23,GPO,GPIO_HIGH,GPO_PUSH_PULL);
	
	//方向引脚初始化
	// 初始化方向控制GPIO（推挽输出）
    gpio_init(MOTOR_A_IN1, GPO, GPIO_LOW, GPO_PUSH_PULL);
    gpio_init(MOTOR_A_IN2, GPO, GPIO_LOW, GPO_PUSH_PULL);
    gpio_init(MOTOR_B_IN1, GPO, GPIO_LOW, GPO_PUSH_PULL);
    gpio_init(MOTOR_B_IN2, GPO, GPIO_LOW, GPO_PUSH_PULL);
	
	//PWM初始化
	/*
	pwm_init(pwm_channel_enum pin, const uint32 freq, 
	const uint32 duty);
	*/
	pwm_init(MOTOR_A_PWM,MOTOR_PWM_FREQ,0);
	pwm_init(MOTOR_B_PWM,MOTOR_PWM_FREQ,0);
}

//控制电机A
void Motor_RightSetSpeed(motor_dir_enum dir, uint32 duty)
{
	if(duty > PWM_DUTY_MAX)
	{
		duty = PWM_DUTY_MAX;
	}
	switch(dir)
	{
		case MOTOR_STOP:
			// 停止：两个IN引脚都输出低电平
            gpio_set_level(MOTOR_A_IN1, GPIO_LOW);
            gpio_set_level(MOTOR_A_IN2, GPIO_LOW);
            // 设置PWM占空比为0
            pwm_set_duty(MOTOR_A_PWM, 0);
            break;
		case MOTOR_FORWARD:
			// 停止：两个IN引脚都输出低电平
            gpio_set_level(MOTOR_A_IN1, GPIO_HIGH);
            gpio_set_level(MOTOR_A_IN2, GPIO_LOW);
            // 设置PWM占空比为0
            pwm_set_duty(MOTOR_A_PWM, duty);
            break;
		case MOTOR_REVERSE:
			// 停止：两个IN引脚都输出低电平
            gpio_set_level(MOTOR_A_IN1, GPIO_LOW);
            gpio_set_level(MOTOR_A_IN2, GPIO_HIGH);
            // 设置PWM占空比为0
            pwm_set_duty(MOTOR_A_PWM, duty);
            break;
		
		default:
			break;
	}
}

//控制电机B
void Motor_LeftSetSpeed(motor_dir_enum dir, uint32 duty)
{
	if(duty > PWM_DUTY_MAX)
	{
		duty = PWM_DUTY_MAX;
	}
	switch(dir)
	{
		case MOTOR_STOP:
			// 停止：两个IN引脚都输出低电平
            gpio_set_level(MOTOR_B_IN1, GPIO_LOW);
            gpio_set_level(MOTOR_B_IN2, GPIO_LOW);
            // 设置PWM占空比为0
            pwm_set_duty(MOTOR_B_PWM, 0);
            break;
		case MOTOR_FORWARD:
			// 停止：两个IN引脚都输出低电平
            gpio_set_level(MOTOR_B_IN1, GPIO_HIGH);
            gpio_set_level(MOTOR_B_IN2, GPIO_LOW);
            // 设置PWM占空比为0
            pwm_set_duty(MOTOR_B_PWM, duty);
            break;
		case MOTOR_REVERSE:
			// 停止：两个IN引脚都输出低电平
            gpio_set_level(MOTOR_B_IN1, GPIO_LOW);
            gpio_set_level(MOTOR_B_IN2, GPIO_HIGH);
            // 设置PWM占空比为0
            pwm_set_duty(MOTOR_B_PWM, duty);
            break;
		
		default:
			break;
	}
}

//前进
void motor_forward(uint32 duty)
{
	Motor_LeftSetSpeed(MOTOR_FORWARD,duty);
	Motor_RightSetSpeed(MOTOR_FORWARD,duty);
}

//停止
void motor_stop()
{
	Motor_LeftSetSpeed(MOTOR_STOP,0);
	Motor_RightSetSpeed(MOTOR_STOP,0);
}

