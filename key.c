#include "key.h"

void Key_init(void)
{
	gpio_init(B24, GPO, GPIO_LOW, GPO_PUSH_PULL);
	gpio_init(A22, GPO, GPIO_LOW, GPO_PUSH_PULL);
	
	gpio_init(CIR_NUM, GPI, GPIO_HIGH, GPI_PULL_UP);
	gpio_init(START, GPI, GPIO_HIGH, GPI_PULL_UP);
}

uint8_t Key_GetCIR_NUM(void)
{
    uint8_t KeyNum = 0;

    if(gpio_get_level(CIR_NUM) == 0)
    {
        system_delay_ms(60);

        while(gpio_get_level(CIR_NUM) == 0);

        system_delay_ms(60);

        KeyNum = 1;
    }


    return KeyNum;
}

uint8_t Key_GetSTART(void)
{
    uint8_t KeyNum = 0;

    if(gpio_get_level(START) == 0)
    {
        system_delay_ms(80);

        while(gpio_get_level(START) == 0);

        system_delay_ms(80);

        KeyNum = 1;
    }

    return KeyNum;
}
