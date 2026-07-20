#include "Receive.h"
#include <stdio.h>
#include <string.h>

volatile int g_x_err = 0;
volatile int g_y_err = 0;
volatile uint8_t g_vision_new_data_flag = 0;
volatile uint8_t g_target_lost_flag = 0;

static char vision_rx_buf[32];
static uint8_t vision_rx_index = 0;
static uint8_t vision_rx_started = 0;

void Vision_Receive_Init(void)
{
    g_x_err = 0;
    g_y_err = 0;
    g_vision_new_data_flag = 0;
    g_target_lost_flag = 0;

    vision_rx_index = 0;
    vision_rx_started = 0;
    memset(vision_rx_buf, 0, sizeof(vision_rx_buf));
}

void Vision_ParseByte(uint8_t ch)
{
    int tx, ty;

    if(ch == '$')
    {
        vision_rx_index = 0;
        vision_rx_started = 1;
        vision_rx_buf[vision_rx_index++] = ch;
        return;
    }

    if(vision_rx_started == 0)
    {
        return;
    }

    if(vision_rx_index >= sizeof(vision_rx_buf) - 1)
    {
        vision_rx_index = 0;
        vision_rx_started = 0;
        memset(vision_rx_buf, 0, sizeof(vision_rx_buf));
        return;
    }

    vision_rx_buf[vision_rx_index++] = ch;

    if(ch == '#')
    {
        vision_rx_buf[vision_rx_index] = '\0';

        if(sscanf(vision_rx_buf, "$%d,%d#", &tx, &ty) == 2)
        {
            // 特殊帧：未检测到目标
            if(tx == 9999 && ty == 9999)
            {
                g_target_lost_flag = 1;
				g_x_err = 0;
				g_y_err = 0;
				g_vision_new_data_flag = 1;
            }
            else
            {
                g_x_err = tx;
                g_y_err = ty;
                g_target_lost_flag = 0;
                g_vision_new_data_flag = 1;
            }
        }

        vision_rx_index = 0;
        vision_rx_started = 0;
        memset(vision_rx_buf, 0, sizeof(vision_rx_buf));
    }
}
