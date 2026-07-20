#include "pid.h"

static float limit_float(float x, float min, float max)
{
    if(x > max) return max;
    if(x < min) return min;
    return x;
}

void PID_Init(PID_t *pid, float kp, float ki, float kd,
              float out_min, float out_max,
              float integral_min, float integral_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->err = 0.0f;
    pid->last_err = 0.0f;
    pid->integral = 0.0f;
    pid->out = 0.0f;

    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->integral_min = integral_min;
    pid->integral_max = integral_max;
}

float PID_Calc(PID_t *pid, float target, float measure)
{
    float p_out, i_out, d_out;

    pid->err = target - measure;

    pid->integral += pid->err;
    pid->integral = limit_float(pid->integral, pid->integral_min, pid->integral_max);

    p_out = pid->kp * pid->err;
    i_out = pid->ki * pid->integral;
    d_out = pid->kd * (pid->err - pid->last_err);

    pid->out = p_out + i_out + d_out;
    pid->out = limit_float(pid->out, pid->out_min, pid->out_max);

    pid->last_err = pid->err;

    return pid->out;
}

void PID_Clear(PID_t *pid)
{
    pid->err = 0.0f;
    pid->last_err = 0.0f;
    pid->integral = 0.0f;
    pid->out = 0.0f;
}

