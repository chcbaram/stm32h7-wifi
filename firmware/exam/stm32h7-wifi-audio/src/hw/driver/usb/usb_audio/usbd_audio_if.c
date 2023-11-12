/**
  ******************************************************************************
  * @file    USB_Device/AUDIO_Standalone/Src/usbd_audio_if.c
  * @author  MCD Application Team
  * @brief   USB Device Audio interface file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------ */
#include "usbd_audio_if.h"
#include "sai.h"

/* Private typedef ----------------------------------------------------------- */
/* Private define ------------------------------------------------------------ */
/* Private macro ------------------------------------------------------------- */
/* Private function prototypes ----------------------------------------------- */
static int8_t Audio_Init(uint32_t AudioFreq, uint32_t Volume, uint32_t options);
static int8_t Audio_DeInit(uint32_t options);
static int8_t Audio_PlaybackCmd(uint8_t * pbuf, uint32_t size, uint8_t cmd);
static int8_t Audio_VolumeCtl(uint8_t vol);
static int8_t Audio_MuteCtl(uint8_t cmd);
static int8_t Audio_PeriodicTC(uint8_t *pbuf, uint32_t size, uint8_t cmd);
static int8_t Audio_GetState(void);
static int8_t Audio_Receive(uint8_t *pbuf, uint32_t size);
int8_t Audio_ReceiveReady(void);


/* Private variables --------------------------------------------------------- */
extern USBD_HandleTypeDef USBD_Device;
USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops = {
  Audio_Init,
  Audio_DeInit,
  Audio_PlaybackCmd,
  Audio_VolumeCtl,
  Audio_MuteCtl,
  Audio_PeriodicTC,
  Audio_GetState,
  Audio_Receive
};


volatile static bool is_rx_full = false;
static uint8_t sai_ch = 0;
static uint32_t pre_time_rd = 0;


// BSP_AUDIO_Init_t audio_init;
/* Private functions --------------------------------------------------------- */

/**
  * @brief  Initializes the AUDIO media low layer.
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  options: Reserved for future use
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Audio_Init(uint32_t AudioFreq, uint32_t Volume, uint32_t options)
{
  // audio_init.Device = AUDIO_OUT_DEVICE_HEADPHONE;
  // audio_init.ChannelsNbr = 2;
  // audio_init.SampleRate = AudioFreq;
  // audio_init.BitsPerSample = AUDIO_RESOLUTION_16B;
  // audio_init.Volume = Volume;
  // BSP_AUDIO_OUT_Init(0, &audio_init);

  // TODO : Audio_Init()
  logPrintf("Audio_Init()\n");
  logPrintf("  AudioFreq : %d\n", AudioFreq);
  logPrintf("  Volume    : %d\n", Volume);
  logPrintf("  options   : %d\n", options);

  saiSetSampleRate(AudioFreq);
  return 0;
}

uint8_t Audio_Sof(USBD_HandleTypeDef *pdev)
{
  if (is_rx_full)
  {
    uint32_t w_buf_len;

    w_buf_len = saiAvailableForWrite(sai_ch); 
    if (w_buf_len >= AUDIO_OUT_PACKET/2)
    {
      Audio_ReceiveReady();
      is_rx_full = false;
    }
  }
  else
  {
    if (millis()-pre_time_rd >= 10)
    {
      Audio_ReceiveReady();
      pre_time_rd = millis();
    }
  }
  return (uint8_t)USBD_OK;
}

/**
  * @brief  De-Initializes the AUDIO media low layer.
  * @param  options: Reserved for future use
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Audio_DeInit(uint32_t options)
{
  // TODO: Audio_DeInit()
  // BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
  logPrintf("Audio_DeInit()\n");
  logPrintf("  options : \n", options);
  return 0;
}

/**
  * @brief  Handles AUDIO command.
  * @param  pbuf: Pointer to buffer of data to be sent
  * @param  size: Number of data to be sent (in bytes)
  * @param  cmd: Command opcode
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Audio_PlaybackCmd(uint8_t * pbuf, uint32_t size, uint8_t cmd)
{
  switch (cmd)
  {
  case AUDIO_CMD_PLAY:
  case AUDIO_CMD_START:
    // TODO: Audio_PlaybackCmd()
    // BSP_AUDIO_OUT_Play(0, (uint8_t *) pbuf, 2 * size);
    // logPrintf("Audio_PlaybackCmd()\n");
    // logPrintf("  size : %d\n", size);
    // logPrintf("  cmd  : %d\n", cmd);
    break;

  default:
    break;
  }
  return 0;
}

/**
  * @brief  Controls AUDIO Volume.
  * @param  vol: Volume level (0..100)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Audio_VolumeCtl(uint8_t vol)
{
  // TODO : Audio_VolumeCtl()
  // BSP_AUDIO_OUT_SetVolume(0, vol);
  logPrintf("Audio_VolumeCtl()\n");
  logPrintf("  vol : %d\n", vol);
  return 0;
}

/**
  * @brief  Controls AUDIO Mute.
  * @param  cmd: Command opcode
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Audio_MuteCtl(uint8_t cmd)
{
  // TODO: Audio_MuteCtl()
  // BSP_AUDIO_OUT_Mute(0);
  // logPrintf("Audio_MuteCtl()");
  // logPrintf("  cmd : %d\n", cmd);
  return 0;
}

/**
  * @brief  Audio_PeriodicTC
  * @param  cmd: Command opcode
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Audio_PeriodicTC(uint8_t *pbuf, uint32_t size, uint8_t cmd)
{
  UNUSED(pbuf);
  UNUSED(size);
  UNUSED(cmd);

  // logPrintf("Audio_PeriodicTC()\n");
  return (0);
}

/**
  * @brief  Gets AUDIO State.
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Audio_GetState(void)
{
  return 0;
}

int8_t Audio_Receive(uint8_t *pbuf, uint32_t size)
{
  saiWrite(sai_ch, (int16_t *)pbuf, size/2);

  uint32_t w_buf_len;

  w_buf_len = saiAvailableForWrite(sai_ch); 
  // logPrintf("%d %d\n", w_buf_len, millis()-pre_time_rd);
  // logPrintf("%d\n", size);
  if (w_buf_len >= AUDIO_OUT_PACKET/2)
  {
    Audio_ReceiveReady();  
  }
  else
  {
    is_rx_full = true;
  }

  pre_time_rd = millis();

  return (int8_t)USBD_OK;
}

int8_t Audio_ReceiveReady(void)
{
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef*)USBD_Device.pClassData;

  (void)USBD_LL_PrepareReceive(&USBD_Device, AUDIO_OUT_EP,
                                haudio->buffer,
                                AUDIO_OUT_PACKET);      

  return 0;
}
