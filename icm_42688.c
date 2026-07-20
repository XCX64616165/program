#include "icm_42688.h"
#include "spi.h"

/*
 * ICM42688 Bank0 寄存器
 */
#define ICM42688_REG_DEVICE_CONFIG      0x11

#define ICM42688_REG_TEMP_DATA1         0x1D
#define ICM42688_REG_TEMP_DATA0         0x1E

#define ICM42688_REG_ACCEL_DATA_X1      0x1F
#define ICM42688_REG_ACCEL_DATA_X0      0x20
#define ICM42688_REG_ACCEL_DATA_Y1      0x21
#define ICM42688_REG_ACCEL_DATA_Y0      0x22
#define ICM42688_REG_ACCEL_DATA_Z1      0x23
#define ICM42688_REG_ACCEL_DATA_Z0      0x24

#define ICM42688_REG_GYRO_DATA_X1       0x25
#define ICM42688_REG_GYRO_DATA_X0       0x26
#define ICM42688_REG_GYRO_DATA_Y1       0x27
#define ICM42688_REG_GYRO_DATA_Y0       0x28
#define ICM42688_REG_GYRO_DATA_Z1       0x29
#define ICM42688_REG_GYRO_DATA_Z0       0x2A

#define ICM42688_REG_INT_STATUS         0x2D

#define ICM42688_REG_PWR_MGMT0          0x4E
#define ICM42688_REG_GYRO_CONFIG0       0x4F
#define ICM42688_REG_ACCEL_CONFIG0      0x50

#define ICM42688_REG_WHO_AM_I           0x75
#define ICM42688_REG_BANK_SEL           0x76

#define ICM42688_WHO_AM_I_VALUE         0x47


static void ICM42688_WriteReg(uint8_t reg, uint8_t data)
{
    Soft_SPI_CS(0);

    Soft_SPI_WriteByte(reg & 0x7F);
    Soft_SPI_WriteByte(data);

    Soft_SPI_CS(1);
}


static uint8_t ICM42688_ReadReg(uint8_t reg)
{
    uint8_t data;

    Soft_SPI_CS(0);

    Soft_SPI_WriteByte(reg | 0x80);
    data = Soft_SPI_ReadByte();

    Soft_SPI_CS(1);

    return data;
}


static void ICM42688_ReadRegs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;

    Soft_SPI_CS(0);

    Soft_SPI_WriteByte(reg | 0x80);

    for(i = 0; i < len; i++)
    {
        buf[i] = Soft_SPI_ReadByte();
    }

    Soft_SPI_CS(1);
}


uint8_t ICM42688_ReadID(void)
{
    ICM42688_WriteReg(ICM42688_REG_BANK_SEL, 0x00);
    HAL_Delay(1);

    return ICM42688_ReadReg(ICM42688_REG_WHO_AM_I);
}


uint8_t ICM42688_Init(void)
{
    uint8_t id;

    Soft_SPI_Init();

    HAL_Delay(10);

    /*
     * 选择 Bank0
     */
    ICM42688_WriteReg(ICM42688_REG_BANK_SEL, 0x00);
    HAL_Delay(1);

    /*
     * 软件复位
     */
    ICM42688_WriteReg(ICM42688_REG_DEVICE_CONFIG, 0x01);
    HAL_Delay(10);

    /*
     * 复位后重新选择 Bank0
     */
    ICM42688_WriteReg(ICM42688_REG_BANK_SEL, 0x00);
    HAL_Delay(1);

    /*
     * 读取 ID
     * 正常 ICM42688 为 0x47
     */
    id = ICM42688_ReadID();

    if(id != ICM42688_WHO_AM_I_VALUE)
    {
        return 0;
    }

    /*
     * 陀螺仪配置：
     * bit[7:5] = 000：±2000 dps
     * bit[3:0] = 0110：ODR 1kHz
     */
    ICM42688_WriteReg(ICM42688_REG_GYRO_CONFIG0, 0x06);
    HAL_Delay(1);

    /*
     * 加速度配置：
     * bit[7:5] = 000：±16g
     * bit[3:0] = 0110：ODR 1kHz
     */
    ICM42688_WriteReg(ICM42688_REG_ACCEL_CONFIG0, 0x06);
    HAL_Delay(1);

    /*
     * 打开陀螺仪和加速度计
     * bit[3:2] = 11：Gyro Low Noise
     * bit[1:0] = 11：Accel Low Noise
     */
    ICM42688_WriteReg(ICM42688_REG_PWR_MGMT0, 0x0F);
    HAL_Delay(50);

    return 1;
}


void ICM42688_ReadData(icm42688_data_t *data)
{
    uint8_t buf[14];

    ICM42688_ReadRegs(ICM42688_REG_TEMP_DATA1, buf, 14);

    data->temp_raw = (int16_t)(((uint16_t)buf[0] << 8) | buf[1]);

    data->acc_x_raw = (int16_t)(((uint16_t)buf[2] << 8) | buf[3]);
    data->acc_y_raw = (int16_t)(((uint16_t)buf[4] << 8) | buf[5]);
    data->acc_z_raw = (int16_t)(((uint16_t)buf[6] << 8) | buf[7]);

    data->gyro_x_raw = (int16_t)(((uint16_t)buf[8]  << 8) | buf[9]);
    data->gyro_y_raw = (int16_t)(((uint16_t)buf[10] << 8) | buf[11]);
    data->gyro_z_raw = (int16_t)(((uint16_t)buf[12] << 8) | buf[13]);

    /*
     * 当前配置：
     * 加速度 ±16g，2048 LSB/g
     * 陀螺仪 ±2000 dps，16.4 LSB/dps
     */
    data->acc_x_g = data->acc_x_raw / 2048.0f;
    data->acc_y_g = data->acc_y_raw / 2048.0f;
    data->acc_z_g = data->acc_z_raw / 2048.0f;

    data->gyro_x_dps = data->gyro_x_raw / 16.4f;
    data->gyro_y_dps = data->gyro_y_raw / 16.4f;
    data->gyro_z_dps = data->gyro_z_raw / 16.4f;

    data->temp_c = data->temp_raw / 132.48f + 25.0f;
}
