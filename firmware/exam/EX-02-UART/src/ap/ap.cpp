#include "ap.h"




void apInit(void)
{  

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
      ledToggle(_DEF_LED1);
    }    

    if (uartAvailable(_DEF_UART1))
    {
      uartPrintf(0, "rx : 0x%02X\n", uartRead(_DEF_UART1));
    }
  }
}

