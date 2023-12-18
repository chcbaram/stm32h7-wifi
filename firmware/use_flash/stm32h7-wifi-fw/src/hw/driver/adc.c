#include "adc.h"



#ifdef _USE_HW_ADC
#include "cli.h"
#include "cli_gui.h"


#define NAME_DEF(x)  x, #x

#ifdef _USE_HW_RTOS
#define lock()      xSemaphoreTake(mutex_lock, portMAX_DELAY);
#define unLock()    xSemaphoreGive(mutex_lock);
#else
#define lock()      
#define unLock()    
#endif


typedef struct
{
  ADC_HandleTypeDef  *p_hadc;
  uint32_t            channel;  
  uint32_t            rank;  
  AdcPinName_t        pin_name;
  const char         *p_name;
} adc_tbl_t;


#if CLI_USE(HW_ADC)
static void cliAdc(cli_args_t *args);
#endif


static ADC_HandleTypeDef hadc2;
static DMA_HandleTypeDef hdma_adc2;

#ifdef _USE_HW_RTOS
static SemaphoreHandle_t mutex_lock;
#endif
static bool is_init = false;

__attribute__((section(".non_cache")))
static uint16_t adc_data_buf[ADC_MAX_CH];
static int32_t adc_cali = 4095/4;

static const adc_tbl_t adc_tbl[ADC_MAX_CH] = 
  {
    {&hadc2, ADC_CHANNEL_4          , ADC_REGULAR_RANK_1 , NAME_DEF(VBAT_ADC)     },
    {&hadc2, ADC_CHANNEL_VBAT       , ADC_REGULAR_RANK_2 , NAME_DEF(VBAT_RTC)     },
    {&hadc2, ADC_CHANNEL_TEMPSENSOR , ADC_REGULAR_RANK_3 , NAME_DEF(TEMP_SENSOR)  },
    {&hadc2, ADC_CHANNEL_VREFINT    , ADC_REGULAR_RANK_4 , NAME_DEF(VREF_INT)     },
  };




bool adcInit(void)
{
  bool ret = true;  
  ADC_ChannelConfTypeDef sConfig = {0};

#ifdef _USE_HW_RTOS
  mutex_lock = xSemaphoreCreateMutex();
#endif

  __HAL_RCC_DMA1_CLK_ENABLE();

  for (int i=0; i<ADC_MAX_CH; i++)
  {
    assert(i == (int)adc_tbl[i].pin_name);
  }

  hadc2.Instance                    = ADC2;
  hadc2.Init.ClockPrescaler         = ADC_CLOCK_ASYNC_DIV2;
  hadc2.Init.Resolution             = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode           = ADC_SCAN_ENABLE;
  hadc2.Init.EOCSelection           = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait       = DISABLE;
  hadc2.Init.ContinuousConvMode     = ENABLE;
  hadc2.Init.NbrOfConversion        = 4;
  hadc2.Init.DiscontinuousConvMode  = DISABLE;
  hadc2.Init.ExternalTrigConv       = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge   = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
  hadc2.Init.Overrun                = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.LeftBitShift           = ADC_LEFTBITSHIFT_NONE;
  hadc2.Init.OversamplingMode       = ENABLE;
  hadc2.Init.Oversampling.Ratio     = 16;
  hadc2.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_4;
  hadc2.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc2.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;

  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    ret = false;
  }
  if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED) != HAL_OK)
  {    
    ret = false;
  }



  sConfig.SamplingTime  = ADC_SAMPLETIME_810CYCLES_5;
  sConfig.SingleDiff    = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber  = ADC_OFFSET_NONE;
  sConfig.Offset        = 0;
  sConfig.OffsetSignedSaturation = DISABLE;

  for (int i=0; i<ADC_MAX_CH; i++)
  {
    sConfig.Channel = adc_tbl[i].channel;
    sConfig.Rank    = adc_tbl[i].rank;
    if (HAL_ADC_ConfigChannel(adc_tbl[i].p_hadc, &sConfig) != HAL_OK)
    {
      ret = false;
      break;
    }
  }


  if (HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&adc_data_buf[0], hadc2.Init.NbrOfConversion) != HAL_OK)
  {
    ret = false;
  }    


  is_init = ret;
  adc_cali = HAL_ADCEx_Calibration_GetValue(&hadc2, ADC_SINGLE_ENDED);

  logPrintf("[%s] adcInit()\n", is_init ? "OK":"NG");
  logPrintf("     cali adc  : %d\n", adc_cali);
  logPrintf("     cali vref : %d\n", (uint32_t)(*VREFINT_CAL_ADDR));

#if CLI_USE(HW_ADC)
  cliAdd("adc", cliAdc);
#endif
  return ret;
}

bool adcIsInit(void)
{
  return is_init;
}

int32_t adcRead(uint8_t ch)
{
  assert_param(ch < ADC_MAX_CH);

  return adc_data_buf[ch];
}

int32_t adcRead8(uint8_t ch)
{
  return adcRead(ch)>>4;
}

int32_t adcRead10(uint8_t ch)
{
  return adcRead(ch)>>2;
}

int32_t adcRead12(uint8_t ch)
{
  return adcRead(ch)>>0;
}

int32_t adcRead16(uint8_t ch)
{
  return adcRead(ch)<<4;  
}

uint8_t adcGetRes(uint8_t ch)
{
  return 12;
}

float adcReadVoltage(uint8_t ch)
{
  return adcConvVoltage(ch, adcRead(ch));
}

float adcConvVoltage(uint8_t ch, uint32_t adc_value)
{
  float ret = 0;


  switch (ch)
  {
    case VBAT_ADC:
      ret  = ((float)adc_value * 3.3f) * 2.0f / (4095.f); 
      break;

    case VBAT_RTC:
      ret  = ((float)adc_value * 3.3f) * 4.0f / (4095.f); 
      break;

    case VREF_INT:    
      ret  = (float)__HAL_ADC_CALC_VREFANALOG_VOLTAGE(adc_value, ADC_RESOLUTION_12B)/1000.0f;
      break;

    case TEMP_SENSOR:
      ret = __HAL_ADC_CALC_TEMPERATURE(__HAL_ADC_CALC_VREFANALOG_VOLTAGE(adc_data_buf[VREF_INT], ADC_RESOLUTION_12B),
                                   adc_value,
                                   ADC_RESOLUTION_12B);
      break;

    default :
      ret  = ((float)adc_value * 3.3f) / (4095.f);
      break;
  }

  return ret;
}


void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(adcHandle->Instance==ADC2)
  {
  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* ADC2 clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**ADC2 GPIO Configuration
    PC4     ------> ADC2_INP4
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


    /* ADC2 DMA Init */
    /* ADC2 Init */
    hdma_adc2.Instance = DMA1_Stream4;
    hdma_adc2.Init.Request = DMA_REQUEST_ADC2;
    hdma_adc2.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc2.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc2.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc2.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc2.Init.Mode = DMA_CIRCULAR;
    hdma_adc2.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc2.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_adc2) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc2);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC2)
  {
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC2 GPIO Configuration
    PC4     ------> ADC2_INP4
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4);

    /* ADC2 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);    
  }
}


#if CLI_USE(HW_ADC)
void cliAdc(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    cliPrintf("adc init : %d\n", is_init);
    cliPrintf("adc res  : %d\n", adcGetRes(0));
    for (int i=0; i<ADC_MAX_CH; i++)
    {
      cliPrintf("%02d. %-32s : %04d\n", i, adc_tbl[i].p_name, adcRead(i));
    }

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "show") == true)
  {
    cliShowCursor(false);
    while(cliKeepLoop())
    {
      for (int i=0; i<ADC_MAX_CH; i++)
      {
        cliPrintf("%02d. %-32s : %04d \n", i, adc_tbl[i].p_name, adcRead(i));
      }
      delay(50);
      cliPrintf("\x1B[%dA", ADC_MAX_CH);
    }
    cliPrintf("\x1B[%dB", ADC_MAX_CH);
    cliShowCursor(true);
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "show") && args->isStr(1, "vol"))
  {
    cliShowCursor(false);
    while(cliKeepLoop())
    {
      for (int i=0; i<ADC_MAX_CH; i++)
      {
        float adc_data;

        adc_data = adcReadVoltage(i);

        cliPrintf("%02d. %-32s : %d.%02dV \n",i, adc_tbl[i].p_name, (int)adc_data, (int)(adc_data * 100)%100);
      }
      delay(50);
      cliPrintf("\x1B[%dA", ADC_MAX_CH);
    }
    cliPrintf("\x1B[%dB", ADC_MAX_CH);
    cliShowCursor(true);
    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("adc info\n");
    cliPrintf("adc show\n");
    cliPrintf("adc show vol\n");
  }
}
#endif

#endif