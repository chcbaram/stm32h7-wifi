#include "ap.h"


static void cliQC(cli_args_t *args);



void apInit(void)
{  
  cliOpen(_DEF_UART1, 115200);
  cliAdd("qc", cliQC);
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


void cliQC(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "test"))
  {

    cliPrintf("qc buzzer\n");
    buzzerOn(1000, 100);
    delay(100);

    cliPrintf("qc button\n");
    while(cliKeepLoop())
    {
      for (int i=0; i<BUTTON_MAX_CH; i++)
      {
        cliPrintf("%d ", buttonGetPressed(i));
      }
      cliPrintf("\r");
      delay(50);
    }
    cliPrintf("\n");
    cliRead();

    cliPrintf("qc eeprom..%s\n", eepromValid(0) ? "OK" : "FAIL");

    cliPrintf("qc spiFlash..%s\n", spiFlashIsInit() ? "OK" : "FAIL");

    cliPrintf("qc qspiFlash..%s\n", qspiIsInit() ? "OK" : "FAIL");

    cliPrintf("qc PSRAM..%s\n", psramIsInit() ? "OK" : "FAIL");

    cliPrintf("qc fatfs..%s\n", fatfsIsInit() ? "OK" : "FAIL");

    cliRunStr("adc show vol\n");
    cliRead();

    cliPrintf("qc audio\n");
    cliRunStr("sai volume 100\n");
    cliRunStr("sai melody\n");
    
    cliPrintf("qc esp32 C3..%s\n", espPing(100) ? "OK" : "FAIL");
    
    if (espPing(100))
    {
      cliPrintf("qc esp32 wifi list\n");
      cliRunStr("esp at AT+CWMODE=3 100");
      delay(100);
      cliRunStr("esp log on");
      cliRunStr("esp wifi list");
    }
    
    
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("qc test\n");
  }
}