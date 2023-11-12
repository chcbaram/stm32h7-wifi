#ifndef HW_DEF_H_
#define HW_DEF_H_



#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION    "V231109R1"
#define _DEF_BOARD_NAME           "STM32H7-WIFI-FW"


#define _USE_HW_CACHE
#define _USE_HW_FAULT
#define _USE_HW_BUZZER
// #define _USE_HW_SD
// #define _USE_HW_FATFS
#define _USE_HW_QSPI
#define _USE_HW_PCA9554
#define _USE_HW_NVS
#define _USE_HW_SAI
// #define _USE_HW_FILES


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

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    32
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    8
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1
#define      HW_I2C_CH_EEPROM       _DEF_I2C1

#define _USE_HW_EEPROM
#define      HW_EEPROM_MAX_SIZE     (8*1024)

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         10

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       2 

#define _USE_HW_RTC
#define      HW_RTC_BOOT_MODE       RTC_BKP_DR3
#define      HW_RTC_RESET_BITS      RTC_BKP_DR4

#define _USE_HW_RESET
#define      HW_RESET_BOOT          1

#define _USE_HW_POWER
#define      HW_POWER_OFF_TIME_MS   1500

#define _USE_HW_SWTIMER
#define      HW_SWTIMER_MAX_CH      8

#define _USE_HW_ADC                 
#define      HW_ADC_MAX_CH          4

#define _USE_HW_SPI
#define      HW_SPI_MAX_CH          2

#define _USE_HW_SPI_FLASH
#define      HW_SPI_FLASH_ADDR      0x91000000

#define _USE_HW_FS
#define      HW_FS_MAX_SIZE         (8*1024*1024)

#define _USE_HW_PSRAM
#define      HW_PSRAM_ADDR          0x92000000

#define _USE_HW_MIXER
#define      HW_MIXER_MAX_CH        4
#define      HW_MIXER_MAX_BUF_LEN   (48*2*4*4) // 48Khz * Stereo * 4ms * 2

#define _USE_HW_ESP
#define      HW_ESP_UART_CH         HW_UART_CH_ESP

#define _USE_HW_USB
#define _USE_HW_CDC
#define      HW_USE_CDC             0
#define      HW_USE_MSC             0
#define      HW_USE_AUDIO           1


//-- CLI
//
#define _USE_CLI_HW_LOG             1
#define _USE_CLI_HW_ASSERT          1
#define _USE_CLI_HW_RESET           1
#define _USE_CLI_HW_EEPROM          0
#define _USE_CLI_HW_GPIO            0
#define _USE_CLI_HW_I2C             0
#define _USE_CLI_HW_RTC             0
#define _USE_CLI_HW_PCA9554         0
#define _USE_CLI_HW_QSPI            0
#define _USE_CLI_HW_SD              0
#define _USE_CLI_HW_BUZZER          0
#define _USE_CLI_HW_UART            0
#define _USE_CLI_HW_SPI_FLASH       0
#define _USE_CLI_HW_FATFS           0
#define _USE_CLI_HW_FS              0
#define _USE_CLI_HW_SAI             1
#define _USE_CLI_HW_ESP             0
#define _USE_CLI_HW_BUTTON          0
#define _USE_CLI_HW_ADC             0
#define _USE_CLI_HW_USB             1
#define _USE_CLI_HW_PSRAM           1


typedef enum
{
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

typedef enum
{
  VBAT_ADC = 0,
  VBAT_RTC,
  TEMP_SENSOR,
  VREF_INT,
  ADC_PIN_MAX
} AdcPinName_t;

#endif
