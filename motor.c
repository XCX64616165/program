#include "motor.h"
#include "tim.h"
#include "gpio.h"
#include <math.h>

#define YAW_MIN_FREQ        1U
#define YAW_MAX_FREQ     5000U

#define PITCH_MIN_FREQ      1U
#define PITCH_MAX_FREQ   5000U

/*
 * 这个变量在 main.c 里面定义：
 * float g_imu_yaw_comp = 0.0f;
 *
 * motor.c 这里只是引用它
 */
extern float g_imu_yaw_comp;

static uint32_t limit_u32(uint32_t x, uint32_t min, uint32_t max)
{
    if(x < min)
    {
        return min;
    }

    if(x > max)
    {
        return max;
    }

    return x;
}

static void TIM_SetFreq(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t freq)
{
    uint32_t arr;

    if(freq < 1U)
    {
        freq = 1U;
    }

    /*
     * 前提：定时器计数频率是 1MHz
     */
    arr = 1000000U / freq - 1U;

    if(arr < 10U)
    {
        arr = 10U;
    }

    if(arr > 65535U)
    {
        arr = 65535U;
    }

    __HAL_TIM_SET_AUTORELOAD(htim, arr);
    __HAL_TIM_SET_COMPARE(htim, channel, (arr + 1U) / 2U);
}

void Motor_Init(void)
{
    /*
     * TIM4_CH1 -> Yaw_STEP
     * TIM3_CH2 -> Pitch_STEP
     */
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);

    /*
     * 方向引脚
     * Yaw_DIR   -> PB10
     * Pitch_DIR -> PB1
     */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,  GPIO_PIN_RESET);
}

void YawMotor_SetSpeed(float speed)
{
    uint32_t freq;

    /*
     * 原来的视觉 PID 输出 + ICM42688 补偿输出
     *
     * main.c 调用：
     * YawMotor_SetSpeed(-yaw_out);
     *
     * 实际输出：
     * -yaw_out + g_imu_yaw_comp
     */
    speed = speed + g_imu_yaw_comp;

    if(fabsf(speed) < 1.0f)
    {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
        return;
    }

    if(speed >= 0.0f)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
    }

    freq = (uint32_t)fabsf(speed);
    freq = limit_u32(freq, YAW_MIN_FREQ, YAW_MAX_FREQ);

    TIM_SetFreq(&htim4, TIM_CHANNEL_1, freq);
}

void PitchMotor_SetSpeed(float speed)
{
    uint32_t freq;

    if(fabsf(speed) < 1.0f)
    {
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
        return;
    }

    if(speed >= 0.0f)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    }

    freq = (uint32_t)fabsf(speed);
    freq = limit_u32(freq, PITCH_MIN_FREQ, PITCH_MAX_FREQ);

    TIM_SetFreq(&htim3, TIM_CHANNEL_2, freq);
}

void Motor_StopAll(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
}
