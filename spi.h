#ifndef __SOFT_SPI_H
#define __SOFT_SPI_H

#include "main.h"
#include <stdint.h>

/*
 * 当前软件 SPI 引脚
 *
 * 注意：
 * PA9 / PA10 通常是 USART1_TX / USART1_RX
 * 如果你的 MaixCam 用 USART1，那么 PA9/PA10 会冲突。
 *
 * 这里只是先按你原来的引脚写，保证能编译。
 * 如果通信异常，必须换 SPI 引脚。
 */
#define SPI_CS_PORT        GPIOA
#define SPI_CS_PIN         GPIO_PIN_3

#define SPI_SCK_PORT       GPIOA
#define SPI_SCK_PIN        GPIO_PIN_6

#define SPI_MOSI_PORT      GPIOA
#define SPI_MOSI_PIN       GPIO_PIN_4

#define SPI_MISO_PORT      GPIOA
#define SPI_MISO_PIN       GPIO_PIN_5

#define SOFT_SPI_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()

void Soft_SPI_Init(void);
void Soft_SPI_CS(uint8_t level);
uint8_t Soft_SPI_SwapByte(uint8_t tx_data);
void Soft_SPI_WriteByte(uint8_t data);
uint8_t Soft_SPI_ReadByte(void);

#endif
