#ifndef HW_DEF_H_
#define HW_DEF_H_



#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION    "V231218R1"
#define _DEF_BOARD_NAME           "STM32H7-WIFI-FSBL"


#define _USE_HW_FAULT
#define _USE_HW_QSPI


#define _USE_HW_LED                 
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART                
#define      HW_UART_MAX_CH         3
#define      HW_UART_CH_SWD         _DEF_UART1
#define      HW_UART_CH_ESP         _DEF_UART2
#define      HW_UART_CH_USB         _DEF_UART3

#define _USE_HW_LOG
#define      HW_LOG_CH              _DEF_UART1
#define      HW_LOG_BOOT_BUF_MAX    2048
#define      HW_LOG_LIST_BUF_MAX    4096

#define _USE_HW_RTC
#define      HW_RTC_BOOT_MODE       RTC_BKP_DR3
#define      HW_RTC_RESET_BITS      RTC_BKP_DR4

#define _USE_HW_RESET
#define      HW_RESET_BOOT          1


#endif
