#include "spi.h"

static void Soft_SPI_Delay(void)
{
    volatile uint32_t i;

    /*
     * 简单软件延时
     * 如果 SPI 不稳定，可以把 20 改大，比如 50、100
     */
    for(i = 0; i < 20; i++)
    {
        __NOP();
    }
}

void Soft_SPI_CS(uint8_t level)
{
    if(level)
    {
        HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(SPI_CS_PORT, SPI_CS_PIN, GPIO_PIN_RESET);
    }
}

void Soft_SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    SOFT_SPI_GPIO_CLK_ENABLE();

    /*
     * CS / SCK / MOSI 输出
     */
    GPIO_InitStruct.Pin = SPI_CS_PIN | SPI_SCK_PIN | SPI_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*
     * MISO 输入
     */
    GPIO_InitStruct.Pin = SPI_MISO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    Soft_SPI_CS(1);

    HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, GPIO_PIN_RESET);
}

uint8_t Soft_SPI_SwapByte(uint8_t tx_data)
{
    uint8_t i;
    uint8_t rx_data = 0;

    for(i = 0; i < 8; i++)
    {
        if(tx_data & 0x80)
        {
            HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(SPI_MOSI_PORT, SPI_MOSI_PIN, GPIO_PIN_RESET);
        }

        tx_data <<= 1;

        Soft_SPI_Delay();

        HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, GPIO_PIN_SET);

        Soft_SPI_Delay();

        rx_data <<= 1;

        if(HAL_GPIO_ReadPin(SPI_MISO_PORT, SPI_MISO_PIN) == GPIO_PIN_SET)
        {
            rx_data |= 0x01;
        }

        HAL_GPIO_WritePin(SPI_SCK_PORT, SPI_SCK_PIN, GPIO_PIN_RESET);

        Soft_SPI_Delay();
    }

    return rx_data;
}

void Soft_SPI_WriteByte(uint8_t data)
{
    Soft_SPI_SwapByte(data);
}

uint8_t Soft_SPI_ReadByte(void)
{
    return Soft_SPI_SwapByte(0xFF);
}
