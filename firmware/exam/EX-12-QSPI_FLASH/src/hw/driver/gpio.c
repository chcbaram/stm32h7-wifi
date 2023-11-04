#include "gpio.h"


#ifdef _USE_HW_GPIO
#include "cli.h"
#include "cli_gui.h"
#include "pca9554.h"


#define NAME_DEF(x)  x, #x

typedef enum
{
  GPIO_HW,
  GPIO_EX,
} GpioType_t;

typedef struct
{
  GpioType_t    type;
  GPIO_TypeDef *port;
  uint32_t      pin;
  uint8_t       mode;
  GPIO_PinState on_state;
  GPIO_PinState off_state;
  bool          init_value;
  GpioPinName_t pin_name;
  const char   *p_name;
} gpio_tbl_t;


static const gpio_tbl_t gpio_tbl[GPIO_MAX_CH] =
    {
      {GPIO_HW, GPIOC, GPIO_PIN_13,  _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH, NAME_DEF(FLASH_SPI_CS)   },  
      {GPIO_HW, GPIOE, GPIO_PIN_0 ,  _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH, NAME_DEF(PSRAM_SPI_CS)   },  
      {GPIO_HW, GPIOE, GPIO_PIN_3 ,  _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH, NAME_DEF(GPIO_3V3_OFF)   },  
      {GPIO_HW, GPIOB, GPIO_PIN_12,  _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH, NAME_DEF(I2S_SD_MODE)    },  

      {GPIO_EX, NULL,            0,  _DEF_INPUT , GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH , NAME_DEF(STDBY_FLAG)    },      
      {GPIO_EX, NULL,            1,  _DEF_INPUT , GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH , NAME_DEF(CHRG_FLAG)     },      
      {GPIO_EX, NULL,            2,  _DEF_INPUT , GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH , NAME_DEF(GPIO_BTN)      },      
      {GPIO_EX, NULL,            3,  _DEF_INPUT , GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH , NAME_DEF(POWER_BTN)     },      
      {GPIO_EX, NULL,            4,  _DEF_INPUT , GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH , NAME_DEF(SD_CD)         }, 
      {GPIO_EX, NULL,            5,  _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET,   _DEF_HIGH , NAME_DEF(MCU_ESP_RST)   },
    };


static uint8_t gpio_data[GPIO_MAX_CH];

#ifdef _USE_HW_CLI
static void cliGpio(cli_args_t *args);
#endif







bool gpioInit(void)
{
  bool ret = true;

  assert(GPIO_MAX_CH == GPIO_PIN_MAX);
  assert(pca9554IsInit());
    
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();


  for (int i=0; i<GPIO_MAX_CH; i++)
  {
    if (gpio_tbl[i].mode & _DEF_OUTPUT)
    {
      gpioPinWrite(i, gpio_tbl[i].init_value);
    }
    gpioPinMode(i, gpio_tbl[i].mode);

    assert(i == (int)gpio_tbl[i].pin_name);
  }

#ifdef _USE_HW_CLI
  cliAdd("gpio", cliGpio);
#endif

  return ret;
}

bool gpioPinMode(uint8_t ch, uint8_t mode)
{
  bool ret = true;
  GPIO_InitTypeDef GPIO_InitStruct = {0};


  if (ch >= GPIO_MAX_CH)
  {
    return false;
  }
  if (gpio_tbl[ch].type == GPIO_EX)
  {
    if (mode == _DEF_INPUT)
      pca9554PinSetMode(gpio_tbl[ch].pin, PCA9554_MODE_INPUT);
    else
      pca9554PinSetMode(gpio_tbl[ch].pin, PCA9554_MODE_OUTPUT);    
    return true;
  }

  switch(mode)
  {
    case _DEF_INPUT:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      break;

    case _DEF_INPUT_PULLUP:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      break;

    case _DEF_INPUT_PULLDOWN:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      break;

    case _DEF_OUTPUT:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      break;

    case _DEF_OUTPUT_PULLUP:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      break;

    case _DEF_OUTPUT_PULLDOWN:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      break;
  }

  GPIO_InitStruct.Pin = gpio_tbl[ch].pin;
  HAL_GPIO_Init(gpio_tbl[ch].port, &GPIO_InitStruct);

  return ret;
}

void gpioPinWrite(uint8_t ch, uint8_t value)
{
  if (ch >= GPIO_MAX_CH)
  {
    return;
  }

  if (gpio_tbl[ch].type == GPIO_HW)
  {
    if (value == _DEF_HIGH)
    {
      HAL_GPIO_WritePin(gpio_tbl[ch].port, gpio_tbl[ch].pin, gpio_tbl[ch].on_state);
    }
    else
    {
      HAL_GPIO_WritePin(gpio_tbl[ch].port, gpio_tbl[ch].pin, gpio_tbl[ch].off_state);
    }
  }
  else
  {
    pca9554PinWrite(gpio_tbl[ch].pin, value);
  }

  gpio_data[ch] = value;
}

uint8_t gpioPinRead(uint8_t ch)
{
  uint8_t ret = _DEF_LOW;

  if (ch >= GPIO_MAX_CH)
  {
    return ret;
  }

  if (gpio_tbl[ch].type == GPIO_HW)
  {
    if (HAL_GPIO_ReadPin(gpio_tbl[ch].port, gpio_tbl[ch].pin) == gpio_tbl[ch].on_state)
    {
      ret = _DEF_HIGH;
    }
  }
  else
  {
    pca9554PinRead(gpio_tbl[ch].pin, &ret);
  }

  gpio_data[ch] = ret;
  return ret;
}

void gpioPinToggle(uint8_t ch)
{
  if (ch >= GPIO_MAX_CH)
  {
    return;
  }
  
  gpio_data[ch] = !gpio_data[ch];
  gpioPinWrite(gpio_tbl[ch].pin, gpio_data[ch]);
}





#ifdef _USE_HW_CLI
void cliGpio(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    for (int i=0; i<GPIO_MAX_CH; i++)
    {
      cliPrintf("%02d. %s %s %-16s - %d\n", 
        i,
        gpio_tbl[i].mode & _DEF_INPUT ? "I":"O", 
        gpio_tbl[i].type == GPIO_HW ? "HW":"EX",
        gpio_tbl[i].p_name, 
        gpioPinRead(i));
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "show") == true)
  {
    while(cliKeepLoop())
    {
      for (int i=0; i<GPIO_MAX_CH; i++)
      {
        cliPrintf("%d", gpioPinRead(i));
      }
      cliPrintf("\n");
      delay(100);
    }
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "show") && args->isStr(1, "input"))
  {
    uint16_t line = 0;

    cliGui()->showCursor(false);
    while(cliKeepLoop())
    {  
      line = 0;
      for (int i=0; i<GPIO_MAX_CH; i++)
      {
        if (gpio_tbl[i].mode & _DEF_INPUT)
        {
          cliPrintf("%02d. %s %s %-16s - %d\n", 
            i,
            gpio_tbl[i].mode & _DEF_INPUT ? "I":"O", 
            gpio_tbl[i].type == GPIO_HW ? "HW":"EX",
            gpio_tbl[i].p_name, 
            gpioPinRead(i));
          line++;
        }
      }
      cliPrintf("\x1B[%dA", line);
    }
    cliPrintf("\x1B[%dB", line);
    cliGui()->showCursor(true);
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "show") && args->isStr(1, "output"))
  {
    uint16_t line = 0;

    cliGui()->showCursor(false);
    while(cliKeepLoop())
    {  
      line = 0;
      for (int i=0; i<GPIO_MAX_CH; i++)
      {
        if (gpio_tbl[i].mode & _DEF_OUTPUT)
        {
          cliPrintf("%02d. %s %s %-16s - %d\n", 
            i,
            gpio_tbl[i].mode & _DEF_INPUT ? "I":"O", 
            gpio_tbl[i].type == GPIO_HW ? "HW":"EX",
            gpio_tbl[i].p_name, 
            gpioPinRead(i));
          line++;
        }
      }
      cliPrintf("\x1B[%dA", line);
    }
    cliPrintf("\x1B[%dB", line);
    cliGui()->showCursor(true);
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "show") && args->isStr(1, "ex"))
  {
    uint16_t line = 0;

    cliGui()->showCursor(false);
    while(cliKeepLoop())
    {  
      line = 0;
      for (int i=0; i<GPIO_MAX_CH; i++)
      {
        if (gpio_tbl[i].type == GPIO_EX)
        {
          cliPrintf("%02d. %s %s %-16s - %d\n", 
            i,
            gpio_tbl[i].mode & _DEF_INPUT ? "I":"O", 
            gpio_tbl[i].type == GPIO_HW ? "HW":"EX",
            gpio_tbl[i].p_name, 
            gpioPinRead(i));
          line++;
        }
      }
      cliPrintf("\x1B[%dA", line);
    }
    cliPrintf("\x1B[%dB", line);
    cliGui()->showCursor(true);
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "read") == true)
  {
    uint8_t ch;

    ch = (uint8_t)args->getData(1);

    while(cliKeepLoop())
    {
      cliPrintf("gpio read %s %d : %d\n", gpio_tbl[ch].p_name, ch, gpioPinRead(ch));
      delay(100);
    }

    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "write") == true)
  {
    uint8_t ch;
    uint8_t data;

    ch   = (uint8_t)args->getData(1);
    data = (uint8_t)args->getData(2);

    gpioPinWrite(ch, data);

    cliPrintf("gpio write %s %d : %d\n", gpio_tbl[ch].p_name, ch, data);
    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("gpio info\n");
    cliPrintf("gpio show\n");
    cliPrintf("gpio show input\n");
    cliPrintf("gpio show output\n");
    cliPrintf("gpio show ex\n");
    cliPrintf("gpio read ch[0~%d]\n", GPIO_MAX_CH-1);
    cliPrintf("gpio write ch[0~%d] 0:1\n", GPIO_MAX_CH-1);
  }
}
#endif


#endif