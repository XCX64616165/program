#ifndef __GRAYSCALE_HEAD_
#define __GRAYSCALE_HEAD_

#include "PWM.h"
#include "oled.h"

#define H1      A27     // OUT1
#define H2      A25     // OUT2
#define H3      B25     // OUT3
#define H4      B20     // OUT4
#define H5      A14     // OUT5
#define H6      A16     // OUT6
#define H7      B17     // OUT7
#define H8      B19     // OUT8
#define H9      A2     // OUT9
#define H10     B21     // OUT10
#define H11     A30     // OUT11
#define H12     B0     // OUT12

#define GRAY_ACTIVE_LEVEL   0

/*
 * 电机基础速度
 * 逐飞 PWM 占空比常见范围是 0 ~ 10000
 */
#define LEFT_BASE_SPEED      1870
#define RIGHT_BASE_SPEED     2230

/*
 * 最大速度限制
 */
#define MAX_SPEED           6500
#define MIN_SPEED           0
#define PID_OUT_MAX         3000

#define CORNER_KEEP_CYCLES       30
#define CORNER_OUTER_SPEED       1800
#define CORNER_INNER_SPEED       1800
/*
 * PID 参数
 * 先只用 P 和 D，I 暂时不用
 */
#define XUNJI_KP            390.0f
#define XUNJI_KI            0.0f
#define XUNJI_KD            0.0f

/*
 * 丢线后的处理
 * 1：沿着上一次方向继续找线
 * 0：直接停车
 */
#define LOST_USE_LAST_DIR   1

extern int16 xunji_error;
extern int16 xunji_pid_out;
extern uint16 gray_value;

void Xunji_Init(void);
uint16 Gray_Read(void);
int16 Xunji_Get_Error(void);
int16 Xunji_PID_Calc(float error);
void Xunji_Control(void);
void Xunji_Stop(void);



#endif