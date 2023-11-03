#include "power.h"


#ifdef _USE_HW_POWER
#include "gpio.h"
#include "led.h"





bool powerInit(void)
{
  return true;
}

bool powerUpdate(void)
{  
  typedef enum
  {
    STATE_POWER_ON,
    STATE_POWER_CHECK,
    STATE_PRESSED,
    STATE_POWER_OFF
  } PowerPinState_t;

  enum
  {
    POWER_BTN_PRESSED  = _DEF_LOW,
    POWER_BTN_RELEASED = _DEF_HIGH
  };

  enum
  {
    POWER_PIN_OFF = _DEF_LOW,
    POWER_PIN_ON  = _DEF_HIGH
  };

  static uint32_t pre_time;
  static uint32_t pre_time_update = 0;
  static PowerPinState_t state = STATE_POWER_ON;


  if (millis()-pre_time_update < 10)
  {
    return false;
  }

  switch (state)
  {
    case STATE_POWER_ON:
      if (gpioPinRead(POWER_BTN) != POWER_BTN_PRESSED)
      {
        state = STATE_POWER_CHECK;
      }    
      break;

    case STATE_POWER_CHECK:
      if (gpioPinRead(POWER_BTN) == POWER_BTN_PRESSED)
      {
        pre_time = millis();
        state = STATE_PRESSED;
        logPrintf("Power Btn Pressed\n");
      }
      break;

    case STATE_PRESSED:
      if (gpioPinRead(POWER_BTN) == POWER_BTN_PRESSED)
      {
        if (millis()-pre_time >= HW_POWER_OFF_TIME_MS)
        {
          logPrintf("Power Off Ready\n");
          state = STATE_POWER_OFF;  
        }
      }
      else
      {
        state = STATE_POWER_CHECK;
      }
      break;

    case STATE_POWER_OFF:
      while(1)
      {
        if (gpioPinRead(POWER_BTN) == POWER_BTN_RELEASED)        
        {
          logPrintf("Power Off Done\n");
          gpioPinWrite(GPIO_3V3_OFF, POWER_PIN_OFF);
        }

        ledToggle(_DEF_LED1);
        delay(50);
      }
      break;
  }

  return true;
}

#endif