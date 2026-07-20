#ifndef __ICM_42688_H
#define __ICM_42688_H

#include "main.h"
#include <stdint.h>

typedef struct
{
    int16_t acc_x_raw;
    int16_t acc_y_raw;
    int16_t acc_z_raw;

    int16_t gyro_x_raw;
    int16_t gyro_y_raw;
    int16_t gyro_z_raw;

    int16_t temp_raw;

    float acc_x_g;
    float acc_y_g;
    float acc_z_g;

    float gyro_x_dps;
    float gyro_y_dps;
    float gyro_z_dps;

    float temp_c;

} icm42688_data_t;

uint8_t ICM42688_Init(void);
uint8_t ICM42688_ReadID(void);
void ICM42688_ReadData(icm42688_data_t *data);

#endif
