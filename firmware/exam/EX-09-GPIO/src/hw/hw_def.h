#ifndef HW_DEF_H_
#define HW_DEF_H_



#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION    "V231030R1"
#define _DEF_BOARD_NAME           "STM32H7-WIFI-FW"


#define _USE_HW_CACHE
#define _USE_HW_FAULT


#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         1
#define      HW_UART_CH_SWD         _DEF_UART1

#define _USE_HW_LOG
#define      HW_LOG_CH              _DEF_UART1
#define      HW_LOG_BOOT_BUF_MAX    2048
#define      HW_LOG_LIST_BUF_MAX    4096

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    32
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    8
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_CLI_GUI
#define      HW_CLI_GUI_WIDTH       80
#define      HW_CLI_GUI_HEIGHT      24

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1
#define      HW_I2C_CH_EEPROM       _DEF_I2C1

#define _USE_HW_EEPROM
#define      HW_EEPROM_MAX_SIZE     (8*1024)

#define _USE_HW_PCA9554

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         11



typedef enum
{
  FLASH_QSPI_SS,
  FLASH_SPI_CS,
  PSRAM_SPI_CS,
  GPIO_3V3_OFF,
  I2S_SD_MODE,

  STDBY_FLAG,
  CHRG_FLAG,
  GPIO_BTN,
  POWER_BTN,
  SD_CD,
  MCU_ESP_RST,

  GPIO_PIN_MAX,  
} GpioPinName_t;
#endif
