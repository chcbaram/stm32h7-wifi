#include "ap.h"



#define QSPI_FW_ADDR   0x90000000



void apInit(void)
{    
  if (qspiSetXipMode(true))
  {
    void (**jump_func)(void) = (void (**)(void))(QSPI_FW_ADDR + 4);

    if ((uint32_t)(*jump_func) >= QSPI_FW_ADDR && (uint32_t)(*jump_func) < (QSPI_FW_ADDR + qspiGetLength()))
    {
      logPrintf("[OK] Jump To Firmware\n");
      delay(10);
      bspDeInit();
       __set_MSP(*(__IO uint32_t*)QSPI_FW_ADDR);
      (*jump_func)();  
    }
  }

  logPrintf("[NG] Fail To Run Firmware\n");
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
  }
}


