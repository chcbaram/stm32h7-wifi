#include "ap.h"
#include "WiFi.h"




void apInit(void)
{  
  cliOpen(_DEF_UART1, 115200);
  delay(500);

  WiFi.init();

  if (WiFi.status() == WL_NO_MODULE) 
  {
    logPrintf("Communication with WiFi module failed!\n");
  }  
}

void apUpdate(void)
{
  powerUpdate();
  sdUpdate();
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


