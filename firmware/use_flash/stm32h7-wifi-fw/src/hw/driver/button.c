#include "button.h"


#ifdef _USE_HW_BUTTON
#include "gpio.h"
#include "cli.h"





typedef struct
{
  uint32_t  pin;
  uint8_t   on_state;
} button_pin_t;



#if CLI_USE(HW_BUTTON)
static void cliButton(cli_args_t *args);
#endif
static bool buttonGetPin(uint8_t ch);


static const button_pin_t button_pin[BUTTON_MAX_CH] =
    {
      {(uint32_t)GPIO_BTN,  _DEF_LOW},  // 0. BTN
      {(uint32_t)POWER_BTN, _DEF_LOW},  // 1. POWER_BTN
    };

static const char *button_name[BUTTON_MAX_CH] = 
{
  "_BTN_SW",   
  "_BTN_POWER",   
};

static bool is_enable = true;





bool buttonInit(void)
{
  bool ret = true;


#if CLI_USE(HW_BUTTON)
  cliAdd("button", cliButton);
#endif

  return ret;
}


bool buttonGetPin(uint8_t ch)
{
  bool ret = false;

  if (ch >= BUTTON_MAX_CH)
  {
    return false;
  }

  if (gpioPinRead(button_pin[ch].pin) == button_pin[ch].on_state)
  {
    ret = true;
  }

  return ret;
}

void buttonEnable(bool enable)
{
  is_enable = enable;
}

bool buttonGetPressed(uint8_t ch)
{
  if (ch >= BUTTON_MAX_CH || is_enable == false)
  {
    return false;
  }

  return buttonGetPin(ch);
}

uint32_t buttonGetData(void)
{
  uint32_t ret = 0;


  for (int i=0; i<BUTTON_MAX_CH; i++)
  {
    ret |= (buttonGetPressed(i)<<i);
  }

  return ret;
}

const char *buttonGetName(uint8_t ch)
{
  ch = constrain(ch, 0, BUTTON_MAX_CH);

  return button_name[ch];
}

uint8_t  buttonGetPressedCount(void)
{
  uint32_t i;
  uint8_t ret = 0;

  for (i=0; i<BUTTON_MAX_CH; i++)
  {
    if (buttonGetPressed(i) == true)
    {
      ret++;
    }
  }

  return ret;
}

#if CLI_USE(HW_BUTTON)
void cliButton(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    for (int i=0; i<BUTTON_MAX_CH; i++)
    {
      cliPrintf("%-12s pin %d : %d\n", buttonGetName(i), button_pin[i].pin, buttonGetPressed(i));
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "show"))
  {    
    while(cliKeepLoop())
    {
      for (int i=0; i<BUTTON_MAX_CH; i++)
      {
        cliPrintf("%d", buttonGetPressed(i));
      }
      delay(50);
      cliPrintf("\r");
    }
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("button info\n");
    cliPrintf("button show\n");
  }
}
#endif



#endif
