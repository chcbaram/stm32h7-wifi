#ifndef HW_H_
#define HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#include "led.h"
#include "uart.h"
#include "log.h"
#include "cli.h"
#include "cli_gui.h"
#include "fault.h"
#include "i2c.h"
#include "eeprom.h"
#include "pca9554.h"
#include "gpio.h"
#include "rtc.h"
#include "reset.h"
#include "power.h"
#include "qspi.h"
#include "buzzer.h"
#include "swtimer.h"


bool hwInit(void);


#ifdef __cplusplus
}
#endif

#endif