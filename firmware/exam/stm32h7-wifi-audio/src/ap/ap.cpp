#include "ap.h"


extern "C" void USBD_AUDIO_INFO(void);


void apInit(void)
{  
  cliOpen(_DEF_UART1, 115200);
}

void apUpdate(void)
{
  powerUpdate();
  // sdUpdate();

  USBD_AUDIO_INFO();
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
    cliMain();    

    apUpdate();
  }
}


