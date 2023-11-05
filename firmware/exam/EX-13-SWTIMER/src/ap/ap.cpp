#include "ap.h"



void ledISR(void *arg)
{
  ledToggle(_DEF_LED1);
}


void apInit(void)
{  
  cliOpen(_DEF_UART1, 115200);

  swtimer_handle_t led_timer;

  led_timer = swtimerGetHandle();

  swtimerSet(led_timer, 500, LOOP_TIME, ledISR, NULL);
  swtimerStart(led_timer);
}

void apUpdate(void)
{
  powerUpdate();
}

void apMain(void)
{
  uint32_t pre_time;

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      // ledToggle(_DEF_LED1);
    }
    cliMain();    

    apUpdate();
  }
}


