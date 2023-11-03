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
    STATE_IDLE,
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
  static PowerPinState_t state = STATE_IDLE;


  switch (state)
  {
    case STATE_IDLE:
      if (gpioPinRead(POWER_BTN) == POWER_BTN_PRESSED)
      {
        pre_time = millis();
        state = STATE_PRESSED;
      }
      break;

    case STATE_PRESSED:
      if (gpioPinRead(POWER_BTN) == POWER_BTN_PRESSED)
      {
        if (millis()-pre_time >= HW_POWER_OFF_TIME_MS)
        {
          state = STATE_POWER_OFF;  
        }
      }
      else
      {
        state = STATE_IDLE;
      }
      break;

    case STATE_POWER_OFF:
      while(1)
      {
        if (gpioPinRead(POWER_BTN) == POWER_BTN_RELEASED)        
        {
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