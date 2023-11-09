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
#include "sd.h"
#include "fatfs.h"
#include "spi.h"
#include "spi_flash.h"
#include "fs.h"
#include "files.h"
#include "nvs.h"
#include "sai.h"
#include "esp.h"
#include "adc.h"
#include "usb.h"
#include "cdc.h"
#include "button.h"


bool hwInit(void);


#ifdef __cplusplus
}
#endif

#endif