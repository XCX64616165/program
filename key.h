#ifndef _KEY_HEAD_
#define _KEY_HEAD_
#include "zf_common_debug.h"
#include "zf_driver_gpio.h"
#include "zf_driver_delay.h"


#define CIR_NUM A26
#define START A24

void Key_init(void);
uint8_t Key_GetCIR_NUM(void);
uint8_t Key_GetSTART(void);

#endif
