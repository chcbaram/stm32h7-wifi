#include "pca9554.h"


#ifdef _USE_HW_PCA9554
#include "i2c.h"
#include "cli.h"
#include "cli_gui.h"


#define PCA9554_MAX_PIN_CH      8


#ifdef _USE_HW_RTOS
#define lock()      xSemaphoreTake(mutex_lock, portMAX_DELAY);
#define unLock()    xSemaphoreGive(mutex_lock);
#else
#define lock()      
#define unLock()    
#endif


#if CLI_USE(HW_PCA9554)
void cliCmd(cli_args_t *args);
#endif

static bool is_init = false;
static const uint8_t i2c_ch = _DEF_I2C1;
static const uint8_t i2c_addr = 0x38;
static uint8_t i2c_retry = 0;




bool pca9554Init(void)
{
  bool ret = true;

#ifdef _USE_HW_RTOS
  mutex_lock = xSemaphoreCreateMutex();
#endif

  if (i2cIsBegin(i2c_ch) == false)
  {
    ret = i2cBegin(i2c_ch, 400);
  }
  if (ret == true)
  {
    uint8_t i2c_data;
    ret = i2cReadByte(i2c_ch, i2c_addr, 0, &i2c_data, 100);
  }

  logPrintf("[%s] pca9554Init()\n", ret ? "OK":"NG");
  if (ret == true)
  {
    logPrintf("     found : 0x%02X\n", i2c_addr);
  }
  else
  {
    logPrintf("     empty\n");
  }

#if CLI_USE(HW_PCA9554)
  cliAdd("pca9554", cliCmd);
#endif

  is_init = ret;

  return ret;
}

bool pca9554IsInit(void)
{
  return is_init;
}

bool pca9554RegRead(PCA9554Reg_t reg, uint8_t *p_data)
{
  bool ret;
  uint8_t i2c_data = 0;
  
  lock();
  for (int i=0; i<2; i++)
  {
    ret = i2cReadByte(i2c_ch, i2c_addr, (uint8_t)reg, &i2c_data, 10);  
    if (ret)
    {
      *p_data = i2c_data;
      i2c_retry = 0;
      break;
    }  
    else
    {
      if (i2c_retry < 3)
      {
        i2cRecovery(i2c_ch);
        i2c_retry++;
      }
      else
      {
        break;
      }
    }
  }
  unLock();

  return ret;
}

bool pca9554RegWrite(PCA9554Reg_t reg, uint8_t data)
{
  bool ret;
  
  lock();
  ret = i2cWriteByte(i2c_ch, i2c_addr, (uint8_t)reg, data, 10);  
  unLock();

  return ret;
}

bool pca9554PinGetMode(uint8_t pin_num, PCA9554Mode_t *p_mode)
{
  bool ret;
  uint8_t rd_data = 0;

  ret = pca9554RegRead(PCA9554_REG_CONFIG, &rd_data);
  if (ret)
  {
    if (rd_data & (1<<pin_num))
      *p_mode = PCA9554_MODE_INPUT;
    else
      *p_mode = PCA9554_MODE_OUTPUT;
  }
  return ret;  
}

bool pca9554PinSetMode(uint8_t pin_num, PCA9554Mode_t mode)
{
  bool ret;
  uint8_t rd_data = 0;

  ret = pca9554RegRead(PCA9554_REG_CONFIG, &rd_data);
  if (ret)
  {
    if (mode == PCA9554_MODE_INPUT)
      rd_data |=  (1 << pin_num);
    else
      rd_data &= ~(1 << pin_num);

    ret = pca9554RegWrite(PCA9554_REG_CONFIG, rd_data);
  }
  return ret;  
}

bool pca9554PinRead(uint8_t pin_num, uint8_t *p_data)
{
  bool ret;
  uint8_t rd_data = 0;

  ret = pca9554RegRead(PCA9554_REG_INPUT, &rd_data);
  if (ret)
  {
    if (rd_data & (1<<pin_num))
      *p_data = _DEF_HIGH;
    else
      *p_data = _DEF_LOW;
  }
  return ret;
}

bool pca9554PinWrite(uint8_t pin_num, uint8_t data)
{
  bool ret;
  uint8_t rd_data = 0;

  ret = pca9554RegRead(PCA9554_REG_OUTPUT, &rd_data);
  if (ret)
  {
    rd_data &= ~(1    << pin_num);
    rd_data |=  (data << pin_num);
    ret = pca9554RegWrite(PCA9554_REG_OUTPUT, rd_data);
  }
  return ret;
}


#if CLI_USE(HW_PCA9554)
void cliCmd(cli_args_t *args)
{
  bool ret = false;



  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("is_init   : %s\n", is_init ? "True":"False");
    if (is_init)
    {
      cliPrintf("addr      : 0x%02X\n", i2c_addr);    
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "show"))
  {
    if (is_init)
    {
      cliShowCursor(false);
      while(cliKeepLoop())
      {
        for (int i=0; i<PCA9554_MAX_PIN_CH; i++)
        {
          PCA9554Mode_t mode;
          uint8_t pin_state;

          pca9554PinRead(i, &pin_state);
          pca9554PinGetMode(i, &mode);        
          cliPrintf("%3d : %s pin: %d\n", 
            i,
            mode == PCA9554_MODE_INPUT ? "INPUT  ":"OUTPUT ",
            (int)pin_state);
        }  
        delay(100);
        cliMoveUp(PCA9554_MAX_PIN_CH);
      }
      cliMoveDown(PCA9554_MAX_PIN_CH);
      cliShowCursor(true);
    }
    else
    {
      cliPrintf("not init\n");
    }
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "read"))
  {
    PCA9554Reg_t reg;
    uint8_t data;
    bool reg_ret;
    uint32_t pre_time;
    uint32_t exe_time;

    reg = constrain(args->getData(1), 0, PCA9554_REG_MAX-1);

    pre_time = millis();
    for (int i=0; i<8; i++)
      pca9554PinRead(i, &data);
    reg_ret = pca9554RegRead(reg, &data);
    exe_time = millis()-pre_time;
    cliPrintf("%s : reg %d, data 0x%02X\n", reg_ret?"OK":"Fail", reg, data);
    cliPrintf("%d ms\n", exe_time);

    ret = true;    
  }

  if (args->argc == 3 && args->isStr(0, "write"))
  {
    PCA9554Reg_t reg;    
    uint8_t data;
    bool reg_ret;
    uint32_t pre_time;
    uint32_t exe_time;

    reg  = constrain(args->getData(1), 0, PCA9554_REG_MAX-1);
    data = args->getData(2);

    pre_time = millis();
    reg_ret = pca9554RegWrite(reg, data);
    exe_time = millis()-pre_time;
    cliPrintf("%s : reg %d, data 0x%02X\n", reg_ret?"OK":"Fail", reg, data);
    cliPrintf("%d ms\n", exe_time);

    ret = true;    
  }

  if (ret == false)
  {
    cliPrintf( "pca9554 info\n");
    cliPrintf( "pca9554 show\n");    
    cliPrintf( "pca9554 read 0~3\n");
    cliPrintf( "pca9554 write 0~3 data\n");
  }

}
#endif 

#endif