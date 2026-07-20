#ifndef __PID_H
#define __PID_H

typedef struct
{
    float kp;
    float ki;
    float kd;

    float err;
    float last_err;
    float integral;

    float out;
    float out_max;
    float out_min;

    float integral_max;
    float integral_min;
} PID_t;

void PID_Init(PID_t *pid, float kp, float ki, float kd,
              float out_min, float out_max,
              float integral_min, float integral_max);

float PID_Calc(PID_t *pid, float target, float measure);
void PID_Clear(PID_t *pid);

#endif
				  