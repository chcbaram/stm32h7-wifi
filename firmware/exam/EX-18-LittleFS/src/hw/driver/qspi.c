#include "qspi.h"



#ifdef _USE_HW_QSPI
#include "qspi/w25q128fv.h"
#include "cli.h"




/* QSPI Error codes */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)



/* QSPI Base Address */
#define QSPI_BASE_ADDRESS          0x90000000



/* QSPI Info */
typedef struct {
  uint32_t FlashSize;          /*!< Size of the flash */
  uint32_t EraseSectorSize;    /*!< Size of sectors for the erase operation */
  uint32_t EraseSectorsNumber; /*!< Number of sectors for the erase operation */
  uint32_t ProgPageSize;       /*!< Size of pages for the program operation */
  uint32_t ProgPagesNumber;    /*!< Number of pages for the program operation */

  uint8_t  device_id[20];
} QSPI_Info;


static bool is_init = false;
static OSPI_HandleTypeDef hqspi;


uint8_t BSP_QSPI_Init(void);
uint8_t BSP_QSPI_DeInit(void);
uint8_t BSP_QSPI_Read(uint8_t* p_data, uint32_t addr, uint32_t length);
uint8_t BSP_QSPI_Write(uint8_t* p_data, uint32_t addr, uint32_t length);
uint8_t BSP_QSPI_Erase_Block(uint32_t block_addr);
uint8_t BSP_QSPI_Erase_Sector(uint32_t SectorAddress);
uint8_t BSP_QSPI_Erase_Chip (void);
uint8_t BSP_QSPI_GetStatus(void);
uint8_t BSP_QSPI_GetInfo(QSPI_Info* p_info);
uint8_t BSP_QSPI_EnableMemoryMappedMode(void);
uint8_t BSP_QSPI_GetID(QSPI_Info* p_info);
uint8_t BSP_QSPI_Config(void);
uint8_t BSP_QSPI_Reset(void);
uint8_t BSP_QSPI_Abort(void);

#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif






bool qspiInit(void)
{
  bool ret = true;
  QSPI_Info info;


  if (BSP_QSPI_Init() == QSPI_OK)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }


  if (BSP_QSPI_GetID(&info) == QSPI_OK)
  {
    if (info.device_id[0] == 0xEF && info.device_id[1] == 0x40 && info.device_id[2] == 0x18)
    {
      logPrintf("[OK] qspiInit()\n");
      logPrintf("     W25Q128JV Found\r\n");
      ret = true;
    }
    else
    {
      logPrintf("[OK] qspiInit()\n");
      logPrintf("     W25Q128JV Not Found %X %X %X\r\n", info.device_id[0], info.device_id[1], info.device_id[2]);
      ret = false;
    }
  }
  else
  {
    logPrintf("[NG] qspiInit()\n");
    ret = false;
  }



  is_init = ret;

#ifdef _USE_HW_CLI
  cliAdd("qspi", cliCmd);
#endif
  return ret;
}

bool qspiReset(void)
{
  bool ret = false;

  if (is_init == true)
  {
    if (BSP_QSPI_Reset() == QSPI_OK)
    {
      ret = true;
    }
  }

  return ret;
}

bool qspiAbort(void)
{
  bool ret = false;

  if (is_init == true)
  {
    if (BSP_QSPI_Abort() == QSPI_OK)
    {
      ret = true;
    }
  }

  return ret;
}

bool qspiIsInit(void)
{
  return is_init;
}

bool qspiRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  uint8_t ret;

  assert(qspiGetXipMode() == false);

  if (addr >= qspiGetLength())
  {
    return false;
  }  

  ret = BSP_QSPI_Read(p_data, addr, length);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  uint8_t ret;


  assert(qspiGetXipMode() == false);

  if (addr >= qspiGetLength())
  {
    return false;
  }

  ret = BSP_QSPI_Write(p_data, addr, length);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiEraseBlock(uint32_t block_addr)
{
  uint8_t ret;


  assert(qspiGetXipMode() == false);

  ret = BSP_QSPI_Erase_Block(block_addr);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiEraseSector(uint32_t sector_addr)
{
  uint8_t ret;


  assert(qspiGetXipMode() == false);

  ret = BSP_QSPI_Erase_Sector(sector_addr);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }  
}
bool qspiErase(uint32_t addr, uint32_t length)
{
  bool ret = true;
  uint32_t flash_length;
  uint32_t block_size;
  uint32_t block_begin;
  uint32_t block_end;
  uint32_t i;


  assert(qspiGetXipMode() == false);


  flash_length = W25Q128FV_FLASH_SIZE;
  block_size   = W25Q128FV_SECTOR_SIZE;


  if ((addr > flash_length) || ((addr+length) > flash_length))
  {
    return false;
  }
  if (length == 0)
  {
    return false;
  }


  block_begin = addr / block_size;
  block_end   = (addr + length - 1) / block_size;


  for (i=block_begin; i<=block_end; i++)
  {
    ret = qspiEraseSector(block_size*i);
    if (ret == false)
    {
      break;
    }
  }

  return ret;
}

bool qspiEraseChip(void)
{
  uint8_t ret;

  assert(qspiGetXipMode() == false);

  ret = BSP_QSPI_Erase_Chip();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiGetStatus(void)
{
  uint8_t ret;

  ret = BSP_QSPI_GetStatus();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiGetInfo(qspi_info_t* p_info)
{
  uint8_t ret;

  ret = BSP_QSPI_GetInfo((QSPI_Info *)p_info);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiEnableMemoryMappedMode(void)
{
  uint8_t ret;

  ret = BSP_QSPI_EnableMemoryMappedMode();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiSetXipMode(bool enable)
{
  uint8_t ret = true;

  if (enable)
  {
    if (qspiGetXipMode() == false)
    {
      ret = qspiEnableMemoryMappedMode();
    }
  }
  else
  {
    if (qspiGetXipMode() == true)
    {
      ret = qspiReset();
    }
  }

  return ret;
}

bool qspiGetXipMode(void)
{
  bool ret = false;

  if (HAL_OSPI_GetState(&hqspi) == HAL_OSPI_STATE_BUSY_MEM_MAPPED)
  {
    ret = true;
  }

  return ret;
}

uint32_t qspiGetAddr(void)
{
  return QSPI_BASE_ADDRESS;
}

uint32_t qspiGetLength(void)
{
  return W25Q128FV_FLASH_SIZE;
}






static uint8_t QSPI_ResetMemory(OSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_WriteEnable(OSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_AutoPollingMemReady(OSPI_HandleTypeDef *hqspi, uint32_t Timeout);
static uint8_t QSPI_ReadStatus(OSPI_HandleTypeDef *hqspi, uint8_t cmd, uint8_t *p_data);
static uint8_t QSPI_WriteStatus(OSPI_HandleTypeDef *hqspi, uint8_t cmd, uint8_t data);


uint8_t BSP_QSPI_Init(void)
{
  OSPIM_CfgTypeDef sOspiManagerCfg = {0};


  hqspi.Instance = OCTOSPI1;
  /* Call the DeInit function to reset the driver */
  if (HAL_OSPI_DeInit(&hqspi) != HAL_OK)
  {
    return QSPI_ERROR;
  }


  /* QSPI initialization */
  /* ClockPrescaler set to 1, so QSPI clock = 112MHz / (1) = 112MHz */
  hqspi.Init.FifoThreshold       = 4;
  hqspi.Init.DualQuad            = HAL_OSPI_DUALQUAD_DISABLE;
  hqspi.Init.MemoryType          = HAL_OSPI_MEMTYPE_MICRON;
  hqspi.Init.DeviceSize          = POSITION_VAL(W25Q128FV_FLASH_SIZE);
  hqspi.Init.ChipSelectHighTime  = 5;
  hqspi.Init.FreeRunningClock    = HAL_OSPI_FREERUNCLK_DISABLE;
  hqspi.Init.ClockMode           = HAL_OSPI_CLOCK_MODE_0;
  hqspi.Init.WrapSize            = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hqspi.Init.ClockPrescaler      = 1;
  hqspi.Init.SampleShifting      = HAL_OSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  hqspi.Init.ChipSelectBoundary  = 0;
  hqspi.Init.DelayBlockBypass    = HAL_OSPI_DELAY_BLOCK_BYPASSED;
  hqspi.Init.MaxTran             = 0;
  hqspi.Init.Refresh             = 0;
  if (HAL_OSPI_Init(&hqspi) != HAL_OK)
  {
    logPrintf("HAL_QSPI_Init() fail\n");
    return QSPI_ERROR;
  }

  sOspiManagerCfg.ClkPort = 1;
  sOspiManagerCfg.NCSPort = 1;
  sOspiManagerCfg.IOLowPort = HAL_OSPIM_IOPORT_1_LOW;
  sOspiManagerCfg.Req2AckTime = 1;
  if (HAL_OSPIM_Config(&hqspi, &sOspiManagerCfg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }


  /* QSPI memory reset */
  if (QSPI_ResetMemory(&hqspi) != QSPI_OK)
  {
    logPrintf("QSPI_ResetMemory() fail\n");
    return QSPI_NOT_SUPPORTED;
  }

  if (BSP_QSPI_Config() != QSPI_OK)
  {
    logPrintf("QSPI_Config() fail\n");
    return QSPI_NOT_SUPPORTED;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Reset(void)
{
  if (QSPI_ResetMemory(&hqspi) != QSPI_OK)
  {
    return QSPI_NOT_SUPPORTED;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Abort(void)
{
  if (HAL_OSPI_Abort(&hqspi) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Config(void)
{
  uint8_t reg = 0;


  if (QSPI_ReadStatus(&hqspi, READ_STATUS_REG2_CMD, &reg) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  // QUAD MODE Enable
  if ((reg & (1<<1)) == 0x00)
  {
    reg |= (1<<1);
    if (QSPI_WriteStatus(&hqspi, WRITE_STATUS_REG2_CMD, reg) != QSPI_OK)
    {
      return QSPI_ERROR;
    }
  }


  return QSPI_OK;
}

uint8_t BSP_QSPI_DeInit(void)
{
  hqspi.Instance = OCTOSPI1;

  /* Call the DeInit function to reset the driver */
  if (HAL_OSPI_DeInit(&hqspi) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Read(uint8_t* p_data, uint32_t addr, uint32_t length)
{
  OSPI_RegularCmdTypeDef s_command = {0};


  /* Initialize the read command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = QUAD_INOUT_FAST_READ_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_4_LINES;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_4_LINES;
  s_command.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_8_BITS;
  s_command.AlternateBytes     = 0;

  s_command.DataMode           = HAL_OSPI_DATA_4_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = W25Q128FV_DUMMY_CYCLES_READ_QUAD;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_ONLY_FIRST_CMD;

  s_command.Address            = addr;
  s_command.NbData             = length;


  /* Send the command */
  if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return false;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(&hqspi, p_data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return false;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Write(uint8_t* p_data, uint32_t addr, uint32_t length)
{
  OSPI_RegularCmdTypeDef s_command = {0};
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = W25Q128FV_PAGE_SIZE - (addr % W25Q128FV_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > length)
  {
    current_size = length;
  }

  /* Initialize the adress variables */
  current_addr = addr;
  end_addr = addr + length;

  /* Initialize the program command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = QUAD_IN_FAST_PROG_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_4_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Perform the write page by page */
  do
  {
    s_command.Address = current_addr;
    s_command.NbData  = current_size;

    /* Enable write operations */
    if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure the command */
    if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Transmission of the data */
    if (HAL_OSPI_Transmit(&hqspi, p_data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure automatic polling mode to wait for end of program */
    if (QSPI_AutoPollingMemReady(&hqspi, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    p_data += current_size;
    current_size = ((current_addr + W25Q128FV_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25Q128FV_PAGE_SIZE;
  } while (current_addr < end_addr);

  return QSPI_OK;
}

uint8_t BSP_QSPI_Erase_Block(uint32_t BlockAddress)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the erase command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = SUBSECTOR_ERASE_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  s_command.Address            = BlockAddress;
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;


  /* Enable write operations */
  if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&hqspi, W25Q128FV_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Erase_Sector(uint32_t SectorAddress)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the erase command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = SECTOR_ERASE_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_1_LINE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  s_command.Address            = SectorAddress;

  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;


  /* Enable write operations */
  if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&hqspi, W25Q128FV_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_Erase_Chip(void)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the erase command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = BULK_ERASE_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable(&hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&hqspi, W25Q128FV_BULK_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_GetStatus(void)
{
  OSPI_RegularCmdTypeDef s_command = {0};
  uint8_t reg;

  /* Initialize the read flag status register command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = READ_FLAG_STATUS_REG_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_1_LINE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  s_command.NbData             = 1;

  /* Configure the command */
  if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(&hqspi, &reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Check the value of the register */
  if ((reg & (W25Q128FV_FSR_PRERR | W25Q128FV_FSR_VPPERR | W25Q128FV_FSR_PGERR | W25Q128FV_FSR_ERERR)) != 0)
  {
    return QSPI_ERROR;
  }
  else if ((reg & (W25Q128FV_FSR_PGSUS | W25Q128FV_FSR_ERSUS)) != 0)
  {
    return QSPI_SUSPENDED;
  }
  else if ((reg & W25Q128FV_FSR_READY) != 0)
  {
    return QSPI_OK;
  }
  else
  {
    return QSPI_BUSY;
  }
}

uint8_t BSP_QSPI_GetID(QSPI_Info* p_info)
{
  OSPI_RegularCmdTypeDef s_command = {0};


  /* Initialize the read flag status register command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = READ_ID_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_1_LINE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  s_command.NbData             = 20;
  
  /* Configure the command */
  if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(&hqspi, p_info->device_id, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

uint8_t BSP_QSPI_GetInfo(QSPI_Info* p_info)
{
  /* Configure the structure with the memory configuration */
  p_info->FlashSize          = W25Q128FV_FLASH_SIZE;
  p_info->EraseSectorSize    = W25Q128FV_SUBSECTOR_SIZE;
  p_info->EraseSectorsNumber = (W25Q128FV_FLASH_SIZE/W25Q128FV_SUBSECTOR_SIZE);
  p_info->ProgPageSize       = W25Q128FV_PAGE_SIZE;
  p_info->ProgPagesNumber    = (W25Q128FV_FLASH_SIZE/W25Q128FV_PAGE_SIZE);

  return QSPI_OK;
}

uint8_t BSP_QSPI_EnableMemoryMappedMode(void)
{
  OSPI_RegularCmdTypeDef   s_command = {0};
  OSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

  /* Configure the command for the read instruction */
  s_command.OperationType      = HAL_OSPI_OPTYPE_READ_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = QUAD_INOUT_FAST_READ_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_4_LINES;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_4_LINES;
  s_command.AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_8_BITS;
  s_command.AlternateBytes     = (1<<5);
  
  s_command.DataMode           = HAL_OSPI_DATA_4_LINES;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;



  // /* Configure the command for write */
  // s_command.OperationType      = HAL_OSPI_OPTYPE_WRITE_CFG;
  // s_command.Instruction        = QUAD_IN_FAST_PROG_CMD;
  // s_command.DummyCycles        = 0;
  // s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  // s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  // if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  // {
  //   return QSPI_ERROR;
  // }

  /* Configure the command for read */
  s_command.OperationType      = HAL_OSPI_OPTYPE_READ_CFG;
  s_command.Instruction        = QUAD_INOUT_FAST_READ_CMD;
  s_command.DummyCycles        = W25Q128FV_DUMMY_CYCLES_READ_QUAD;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_ONLY_FIRST_CMD;

  if (HAL_OSPI_Command(&hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  hqspi.State = HAL_OSPI_STATE_CMD_CFG;

  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_ENABLE;
  s_mem_mapped_cfg.TimeOutPeriod     = 0x20;
  if (HAL_OSPI_MemoryMapped(&hqspi, &s_mem_mapped_cfg) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_ResetMemory(OSPI_HandleTypeDef *p_hqspi)
{
  OSPI_RegularCmdTypeDef s_command = {0};


  if (HAL_OSPI_GetState(&hqspi) != HAL_OSPI_STATE_READY)
  {
    HAL_OSPI_Abort(p_hqspi);
  }

  /* Initialize the reset enable command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = RESET_ENABLE_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(p_hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the reset memory command */
  s_command.Instruction = RESET_MEMORY_CMD;
  if (HAL_OSPI_Command(p_hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  if (QSPI_AutoPollingMemReady(p_hqspi, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_WriteEnable(OSPI_HandleTypeDef *p_hqspi)
{
  OSPI_RegularCmdTypeDef  s_command = {0};
  OSPI_AutoPollingTypeDef s_config = {0};

  /* Enable write operations */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = WRITE_ENABLE_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_NONE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData             = 0;


  if (HAL_OSPI_Command(p_hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  s_command.Instruction    = READ_STATUS_REG_CMD;
  s_command.DataMode       = HAL_OSPI_DATA_1_LINE;
  s_command.NbData         = 1;
  if (HAL_OSPI_Command(p_hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.Match           = W25Q128FV_SR_WREN;
  s_config.Mask            = W25Q128FV_SR_WREN;
  s_config.MatchMode       = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_OSPI_AutoPolling(p_hqspi, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_AutoPollingMemReady(OSPI_HandleTypeDef *p_hqspi, uint32_t Timeout)
{
  OSPI_RegularCmdTypeDef  s_command = {0};
  OSPI_AutoPollingTypeDef s_config = {0};

  /* Configure automatic polling mode to wait for memory ready */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = READ_STATUS_REG_CMD;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_1_LINE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData             = 1;


  if (HAL_OSPI_Command(p_hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return false;
  }

  s_config.Match           = 0;
  s_config.Mask            = W25Q128FV_SR_WIP;
  s_config.MatchMode       = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_OSPI_AutoPolling(p_hqspi, &s_config, Timeout) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

static uint8_t QSPI_ReadStatus(OSPI_HandleTypeDef *p_hqspi, uint8_t cmd, uint8_t *p_data)
{
  OSPI_RegularCmdTypeDef s_command = {0};
  uint8_t reg;

  /* Initialize the read flag status register command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = cmd;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_1_LINE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData             = 1;


  /* Configure the command */
  if (HAL_OSPI_Command(p_hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(p_hqspi, &reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  *p_data = reg;

  return QSPI_OK;
}

static uint8_t QSPI_WriteStatus(OSPI_HandleTypeDef *p_hqspi, uint8_t cmd, uint8_t data)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the program command */
  s_command.OperationType      = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId            = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode    = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.InstructionSize    = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Instruction        = cmd;
  
  s_command.AddressMode        = HAL_OSPI_ADDRESS_NONE;
  s_command.AddressDtrMode     = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.AddressSize        = HAL_OSPI_ADDRESS_24_BITS;
  
  s_command.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
  
  s_command.DataMode           = HAL_OSPI_DATA_1_LINE;
  s_command.DataDtrMode        = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DummyCycles        = 0;
  s_command.DQSMode            = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode           = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData             = 1;

  /* Enable write operations */
  if (QSPI_WriteEnable(p_hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure the command */
  if (HAL_OSPI_Command(p_hqspi, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_OSPI_Transmit(p_hqspi, &data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of program */
  if (QSPI_AutoPollingMemReady(p_hqspi, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
  {
    return QSPI_ERROR;
  }


  return QSPI_OK;
}






void HAL_OSPI_MspInit(OSPI_HandleTypeDef* ospiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if(ospiHandle->Instance==OCTOSPI1)
  {
    /* OCTOSPI1 clock enable */
    __HAL_RCC_OCTOSPIM_CLK_ENABLE();
    __HAL_RCC_OSPI1_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**OCTOSPI1 GPIO Configuration
    PE2     ------> OCTOSPIM_P1_IO2
    PA1     ------> OCTOSPIM_P1_IO3
    PB2     ------> OCTOSPIM_P1_CLK
    PD11     ------> OCTOSPIM_P1_IO0
    PC10     ------> OCTOSPIM_P1_IO1
    PC11     ------> OCTOSPIM_P1_NCS
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }
}

void HAL_OSPI_MspDeInit(OSPI_HandleTypeDef* ospiHandle)
{

  if(ospiHandle->Instance==OCTOSPI1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_OCTOSPIM_CLK_DISABLE();
    __HAL_RCC_OSPI1_CLK_DISABLE();

    /**OCTOSPI1 GPIO Configuration
    PE2     ------> OCTOSPIM_P1_IO2
    PA1     ------> OCTOSPIM_P1_IO3
    PB2     ------> OCTOSPIM_P1_CLK
    PD11     ------> OCTOSPIM_P1_IO0
    PC10     ------> OCTOSPIM_P1_IO1
    PC11     ------> OCTOSPIM_P1_NCS
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);
  }
}



#ifdef _USE_HW_CLI
void cliCmd(cli_args_t *args)
{
  bool ret = false;
  uint32_t i;
  uint32_t addr;
  uint32_t length;
  uint8_t  data;
  uint32_t pre_time;
  bool flash_ret;



  if(args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("qspi flash addr  : 0x%X\n", 0);
    cliPrintf("qspi xip   addr  : 0x%X\n", qspiGetAddr());
    cliPrintf("qspi xip   mode  : %s\n", qspiGetXipMode() ? "True":"False");
    cliPrintf("qspi state       : ");

    switch(HAL_OSPI_GetState(&hqspi))
    {
      case HAL_OSPI_STATE_RESET:
        cliPrintf("RESET\n");
        break;
      case HAL_OSPI_STATE_HYPERBUS_INIT:
        cliPrintf("HYPERBUS_INIT\n");
        break;
      case HAL_OSPI_STATE_READY:
        cliPrintf("READY\n");
        break; 
      case HAL_OSPI_STATE_CMD_CFG:
        cliPrintf("CMD_CFG\n");
        break;    
      case HAL_OSPI_STATE_READ_CMD_CFG:
        cliPrintf("READ_CMD_CFG\n");
        break;                                  
      case HAL_OSPI_STATE_WRITE_CMD_CFG:
        cliPrintf("WRITE_CMD_CFG\n");
        break;      
      case HAL_OSPI_STATE_BUSY_CMD:
        cliPrintf("BUSY_CMD\n");
        break;          
      case HAL_OSPI_STATE_BUSY_TX:
        cliPrintf("BUSY_TX\n");
        break;       
      case HAL_OSPI_STATE_BUSY_RX:
        cliPrintf("BUSY_RX\n");
        break;    
      case HAL_OSPI_STATE_BUSY_AUTO_POLLING:
        cliPrintf("BUSY_AUTO_POLLING\n");
        break;          
      case HAL_OSPI_STATE_BUSY_MEM_MAPPED:
        cliPrintf("BUSY_MEM_MAPPED\n");
        break;         
      case HAL_OSPI_STATE_ABORT:
        cliPrintf("ABORT\n");
        break;        
      case HAL_OSPI_STATE_ERROR:
        cliPrintf("ERROR\n");
        break;                                                                                        
      default:
        cliPrintf("UNKWNON\n");
        break;
    }
    ret = true;
  }
  
  if(args->argc == 1 && args->isStr(0, "test"))
  {
    uint8_t rx_buf[256];

    for (int i=0; i<100; i++)
    {
      if (qspiRead(0x1000*i, rx_buf, 256))
      {
        cliPrintf("%d : OK\n", i);
      }
      else
      {
        cliPrintf("%d : FAIL\n", i);
        break;
      }
    }
    ret = true;
  }    

  if (args->argc == 2 && args->isStr(0, "xip"))
  {
    bool xip_enable;

    xip_enable = args->isStr(1, "on") ? true:false;

    if (qspiSetXipMode(xip_enable))
      cliPrintf("qspiSetXipMode() : OK\n");
    else
      cliPrintf("qspiSetXipMode() : Fail\n");
    
    cliPrintf("qspi xip mode  : %s\n", qspiGetXipMode() ? "True":"False");

    ret = true;
  } 

  if (args->argc == 3 && args->isStr(0, "read"))
  {
    addr   = (uint32_t)args->getData(1);
    length = (uint32_t)args->getData(2);

    for (i=0; i<length; i++)
    {
      flash_ret = qspiRead(addr+i, &data, 1);

      if (flash_ret == true)
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
  
  if(args->argc == 3 && args->isStr(0, "erase") == true)
  {
    addr   = (uint32_t)args->getData(1);
    length = (uint32_t)args->getData(2);

    pre_time = millis();
    flash_ret = qspiErase(addr, length);

    cliPrintf( "addr : 0x%X\t len : %d %d ms\n", addr, length, (millis()-pre_time));
    if (flash_ret)
    {
      cliPrintf("OK\n");
    }
    else
    {
      cliPrintf("FAIL\n");
    }
    ret = true;
  }

  if(args->argc == 3 && args->isStr(0, "write") == true)
  {
    uint32_t flash_data;

    addr = (uint32_t)args->getData(1);
    flash_data = (uint32_t )args->getData(2);

    pre_time = millis();
    flash_ret = qspiWrite(addr, (uint8_t *)&flash_data, 4);

    cliPrintf( "addr : 0x%X\t 0x%X %dms\n", addr, flash_data, millis()-pre_time);
    if (flash_ret)
    {
      cliPrintf("OK\n");
    }
    else
    {
      cliPrintf("FAIL\n");
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "speed-test") == true)
  {
    uint32_t buf[512/4];
    uint32_t cnt;
    uint32_t pre_time;
    uint32_t exe_time;
    uint32_t xip_addr;

    xip_addr = qspiGetAddr();
    cnt = 1024*1024 / 512;
    pre_time = millis();
    for (int i=0; i<cnt; i++)
    {
      if (qspiGetXipMode())
      {
        memcpy(buf, (void *)(xip_addr + i*512), 512);
      }
      else
      {
        if (qspiRead(i*512, (uint8_t *)buf, 512) == false)
        {
          cliPrintf("qspiRead() Fail:%d\n", i);
          break;
        }
      }
    }
    exe_time = millis()-pre_time;
    if (exe_time > 0)
    {
      cliPrintf("%d KB/sec\n", 1024 * 1000 / exe_time);
    }
    ret = true;
  }


  if (ret == false)
  {
    cliPrintf("qspi info\n");
    cliPrintf("qspi xip on:off\n");
    cliPrintf("qspi test\n");
    cliPrintf("qspi speed-test\n");
    cliPrintf("qspi read  [addr] [length]\n");
    cliPrintf("qspi erase [addr] [length]\n");
    cliPrintf("qspi write [addr] [data]\n");
  }
}
#endif

#endif