#ifndef BSP_H_
#define BSP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "def.h"
#include "stm32h7xx_hal.h"

#include "assert_def.h"



bool bspInit(void);
void bspDeInit(void);
void bspMpuInit(void);

void logPrintf(const char *fmt, ...);
void delay(uint32_t time_ms);
uint32_t millis(void);
void Error_Handler(void);



#ifdef __cplusplus
}
#endif

#endif