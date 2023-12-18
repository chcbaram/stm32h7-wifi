#include "psram.h"



#ifdef _USE_HW_PSRAM
#include "spi.h"
#include "gpio.h"
#include "cli.h"

#define PSRAM_ADDR_OFFSET         HW_PSRAM_ADDR
#define PSRAM_MAX_SIZE            (8*1024*1024)
#define PSRAM_SECTOR_SIZE         (4*1024)
#define PSRAM_PAGE_SIZE           (1024)
#define PSRAM_MAX_SECTOR          (PSRAM_MAX_SIZE / PSRAM_SECTOR_SIZE)


#define SPI_CS_L()    gpioPinWrite(PSRAM_SPI_CS, _DEF_LOW)
#define SPI_CS_H()    gpioPinWrite(PSRAM_SPI_CS, _DEF_HIGH)

#define PSRAM_CMD_ENTERQPI          0x35
#define PSRAM_CMD_EXITQPI           0xF5
#define PSRAM_CMD_READID            0x9F
#define PSRAM_CMD_FAST_READ         0x0B
#define PSRAM_CMD_READ_QPI          0xEB
#define PSRAM_CMD_WRITE_QPI         0x38
#define PSRAM_CMD_WRITE_SPI         0x02
#define PSRAM_CMD_RESET_ENABLE      0x66
#define PSRAM_CMD_RESET             0x99

#define PSRAM_ID_MAX                1


typedef struct
{
  uint16_t id;
  const char *name;
} psram_id_t;

static bool is_init = false;
static uint8_t spi_ch = _DEF_SPI2;
static uint8_t psram_id_index = 0;

const psram_id_t psram_id_tbl[PSRAM_ID_MAX] = 
  {
    {0x5D0D, "APS6404"}    
  };

static bool psramTransfer(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length, uint32_t timeout);
static bool psramGetVendorID(uint16_t *p_vender_id);
static bool psramReset(void);


#if CLI_USE(HW_PSRAM)
static void cliCmd(cli_args_t *args);
#endif



bool psramInit(void)
{
  bool ret = true;
  uint16_t vendor_id = 0;


  ret = psramReset();

  if (psramGetVendorID(&vendor_id) == true)
  {
    bool is_found = false;

    for (int i=0; i<PSRAM_ID_MAX; i++)
    {
      if (vendor_id == psram_id_tbl[i].id)
      {
        logPrintf("[OK] psramInit()\n");
        logPrintf("     %s Found\r\n", psram_id_tbl[i].name);        
        ret = true;
        is_found = true;
        psram_id_index = i;
        break;
      }
    }    

    if (is_found == false)
    {
      logPrintf("[NG] psramInit()\n");
      logPrintf("     Not Found ID 0x%04X\r\n", vendor_id);      
      ret = false;
    }
  }
  else
  {
    logPrintf("[NG] psramInit()\n");
    logPrintf("     psramReset() Fail\n");
    ret = false;
  }

  is_init = ret;

#if CLI_USE(HW_PSRAM)
  cliAdd("psram", cliCmd);
#endif

  return ret;
}

bool psramIsInit(void)
{
  return is_init;
}

bool psramReset(void)
{
  bool ret = true;


  if (spiBegin(spi_ch) == false)
  {
    return false;
  }

  uint8_t tx_buf[4];


  //-- Reset Memory
  //
  tx_buf[0] = PSRAM_CMD_RESET_ENABLE;

  SPI_CS_L();
  ret &= psramTransfer(tx_buf, NULL, 1, 10);
  SPI_CS_H();

  tx_buf[0] = PSRAM_CMD_RESET;
  SPI_CS_L();
  ret &= psramTransfer(tx_buf, NULL, 1, 10);
  SPI_CS_H();

  delay(10);

  return ret;
}

bool psramGetVendorID(uint16_t *p_vender_id)
{
  bool ret = true;

  uint8_t tx_buf[4];
  uint8_t rx_buf[4];

  //-- Read ID
  //
  tx_buf[0] = PSRAM_CMD_READID;
  SPI_CS_L();
  ret &= psramTransfer(tx_buf, rx_buf, 4, 10);
  ret &= psramTransfer(tx_buf, rx_buf, 3, 10);
  SPI_CS_H();
  if (ret == true)
  {
    *p_vender_id = (rx_buf[1]<<8) | (rx_buf[0]<<0);
  }
  return ret;
}

bool psramRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint8_t tx_buf[5];

  tx_buf[0] = PSRAM_CMD_FAST_READ;
  tx_buf[1] = addr >> 16;
  tx_buf[2] = addr >> 8;
  tx_buf[3] = addr >> 0;
  tx_buf[4] = 0; // Dummy

  SPI_CS_L();
  ret &= psramTransfer(tx_buf, NULL, 5, 10);
  if (ret == true)
  {
    ret &= psramTransfer(NULL, p_data, length, 500);
  }
  SPI_CS_H();

  #ifdef _USE_HW_CACHE
  SCB_InvalidateDCache_by_Addr ((uint32_t *)p_data, length);
  #endif

  return ret;
}

bool psramWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint8_t tx_buf[4];
  uint32_t end_addr, current_size, current_addr;


  if (addr+length > psramGetLength())
  {
    return false;
  }


  /* Calculation of the size between the write address and the end of the page */
  current_size = PSRAM_PAGE_SIZE - (addr % PSRAM_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > length)
  {
    current_size = length;
  }

  /* Initialize the adress variables */
  current_addr = addr;
  end_addr = addr + length;


  /* Perform the write page by page */
  do
  {
    //-- Write Page
    //
    SPI_CS_L();
    tx_buf[0] = PSRAM_CMD_WRITE_SPI;
    tx_buf[1] = current_addr >> 16;
    tx_buf[2] = current_addr >> 8;
    tx_buf[3] = current_addr >> 0;
    ret &= psramTransfer(tx_buf, NULL, 4, 10);

    ret &= psramTransfer(p_data, NULL, current_size, 10);
    SPI_CS_H();

    if (ret == false)
    {
      break;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    p_data += current_size;
    current_size = ((current_addr + PSRAM_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : PSRAM_PAGE_SIZE;
  } while (current_addr < end_addr);


  return ret;
}

uint32_t psramGetAddr(void)
{
  return PSRAM_ADDR_OFFSET;
}

uint32_t psramGetLength(void)
{
  return PSRAM_MAX_SIZE;
}

bool psramTransfer(uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length, uint32_t timeout)
{
  bool ret = true;

  ret = spiTransferDMA(spi_ch, tx_buf, rx_buf, length, timeout);  

  return ret;
}


#if CLI_USE(HW_PSRAM)
void cliCmd(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  { 
    cliPrintf("is_init   : %s\n", is_init ? "True":"False");
    cliPrintf("addr      : 0x%X\n", psramGetAddr());
    cliPrintf("length    : %dKB\n", psramGetLength()/1024);
    if (is_init)
    {
      cliPrintf("chip id   : 0x%04X\n", psram_id_tbl[psram_id_index].id);
      cliPrintf("chip name : %s\n", psram_id_tbl[psram_id_index].name);      
    }
    
    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "read"))
  {
    uint32_t addr;
    uint32_t length;    
    uint8_t data;

    addr   = (uint32_t)args->getData(1);
    length = (uint32_t)args->getData(2);

    for (int i=0; i<length; i++)
    {
      if (psramRead(addr+i, &data, 1))
      {
        cliPrintf( "addr : 0x%X\t 0x%02X\n", addr+i, data);
      }
      else
      {
        cliPrintf( "addr : 0x%X\t Fail\n", addr+i);
      }
    }
    ret = true;
  }

  if(args->argc == 3 && args->isStr(0, "write"))
  {
    uint32_t addr;
    uint32_t data;
    uint32_t pre_time;
    bool psram_ret;

    addr = (uint32_t)args->getData(1);
    data = (uint32_t )args->getData(2);

    pre_time = millis();
    psram_ret = psramWrite(addr, (uint8_t *)&data, 4);

    cliPrintf( "addr : 0x%X\t 0x%X %dms\n", addr, data, millis()-pre_time);
    if (psram_ret)
    {
      cliPrintf("OK\n");
    }
    else
    {
      cliPrintf("FAIL\n");
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "speed-read") == true)
  {
    uint32_t buf[512/4];
    uint32_t cnt;
    uint32_t pre_time;
    uint32_t exe_time;


    cnt = 1024*1024 / 512;
    pre_time = millis();
    for (int i=0; i<cnt; i++)
    {
      if (psramRead(i*512, (uint8_t *)buf, 512) == false)
      {
        cliPrintf("psramRead() Fail:%d\n", i);
        break;
      }
    }
    exe_time = millis()-pre_time;
    if (exe_time > 0)
    {
      cliPrintf("%d KB/sec\n", 1024 * 1000 / exe_time);
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "speed-write") == true)
  {
    uint32_t w_data;
    bool psram_ret;
    uint32_t pre_time;
    uint32_t exe_time;
    
    cliPrintf("Write...\n");
    pre_time = millis();    
    for (int i=0; i<PSRAM_MAX_SIZE/4; i++)
    {
      w_data = i;
      psram_ret = psramWrite(i*4, (uint8_t *)&w_data, 4);
      if (psram_ret == false)
      {
        cliPrintf("     Fail : %d\n", i);
        break;
      }
    }
    exe_time = (millis()-pre_time) + 1;
    cliPrintf("     %d KB/sec\n", (PSRAM_MAX_SIZE/1024) * 1000 / exe_time);

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "verify") == true)
  {
    uint32_t w_data;
    uint32_t r_data;
    bool psram_ret;
    uint32_t pre_time;
    uint32_t exe_time;
    
    
    cliPrintf("Write... ");
    pre_time = millis();    
    for (int i=0; i<PSRAM_MAX_SIZE/4; i++)
    {
      w_data = i;
      psram_ret = psramWrite(i*4, (uint8_t *)&w_data, 4);
      if (psram_ret == false)
      {
        cliPrintf("Fail : %d\n", i);
        break;
      }      
    }
    exe_time = (millis()-pre_time) + 1;
    if (psram_ret == true)
      cliPrintf("OK %d KB/sec\n", (PSRAM_MAX_SIZE/1024) * 1000 / exe_time);

    cliPrintf("Read.... ");
    pre_time = millis();    
    for (int i=0; i<PSRAM_MAX_SIZE/4; i++)
    {
      psram_ret = psramRead(i*4, (uint8_t *)&r_data, 4);
      if (psram_ret == false)
      {
        cliPrintf("Fail : %d\n", i);
        break;
      }
      if (r_data != i)
      {
        cliPrintf("Data Fail : %d, r_data 0x%X\n", i, r_data);
        break;
      }
    }
    exe_time = (millis()-pre_time) + 1;
    if (psram_ret == true)
      cliPrintf("OK %d KB/sec\n", (PSRAM_MAX_SIZE/1024) * 1000 / exe_time);

    cliPrintf("Verify.. %s\n", psram_ret ? "OK":"Fail");
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf( "psram info\n");
    cliPrintf( "psram read  [addr] [length]\n");
    cliPrintf( "psram write [addr] [data]\n");
    cliPrintf( "psram speed-read\n");
    cliPrintf( "psram speed-write\n");
    cliPrintf( "psram verify\n");
  }  
}
#endif

#endif