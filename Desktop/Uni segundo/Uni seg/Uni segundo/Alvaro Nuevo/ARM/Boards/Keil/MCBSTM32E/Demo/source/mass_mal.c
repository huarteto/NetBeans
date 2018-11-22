/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : mass_mal.c
* Author             : MCD Application Team
* Version            : V1.1.1
* Date               : 06/13/2008
* Description        : Medium Access Layer interface
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sdcard.h"
#include "fsmc_nand.h"
#include "nand_if.h"
#include "mass_mal.h"
#include "hw_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u32 Mass_Memory_Size[2];
u32 Mass_Block_Size[2];
u32 Mass_Block_Count[2];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 MAL_Init(u8 lun)
{
  u16 status = MAL_OK;

  switch (lun)
  {
    case 0:
      status = SD_Init();
      SD_EnableWideBusOperation(SDIO_BusWide_4b);
      SD_SetDeviceMode(SD_POLLING_MODE);
      break;
    case 1:
//    status = NAND_Init();
//    break;
    default:
      return MAL_FAIL;
  }
  return status;
}
/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 MAL_Write(u8 lun, u32 Memory_Offset, u32 *Writebuff, u16 Transfer_Length)
{

  switch (lun)
  {
    case 0:
      SD_WriteBlock(Memory_Offset, Writebuff, Transfer_Length);
      break;

    case 1:
//    NAND_Write(Memory_Offset, Writebuff, Transfer_Length);
//    break;

    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
u16 MAL_Read(u8 lun, u32 Memory_Offset, u32 *Readbuff, u16 Transfer_Length)
{

  switch (lun)
  {
    case 0:

      SD_ReadBlock(Memory_Offset, Readbuff, Transfer_Length);
      break;

    case 1:
//    NAND_Read(Memory_Offset, Readbuff, Transfer_Length);
//    break;

    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 MAL_GetStatus (u8 lun)
{
  SD_CardInfo SD_cardinfo;
  u32 DeviceSizeMul = 0;
//NAND_IDTypeDef NAND_ID;


  if (lun == 0)
  {
    if (SD_Init() == SD_OK)
    {
      SD_GetCardInfo(&SD_cardinfo);
      DeviceSizeMul = (SD_cardinfo.SD_csd.DeviceSizeMul + 2);
      SD_SelectDeselect((u32) (SD_cardinfo.RCA << 16));
      Mass_Block_Count[0] = (SD_cardinfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul);
      Mass_Block_Size[0]  = 1 << (SD_cardinfo.SD_csd.RdBlockLen);

      Mass_Memory_Size[0] = Mass_Block_Count[0] * Mass_Block_Size[0];
      GPIO_SetBits(USB_LED_PORT, GPIO_Pin_7);
      return MAL_OK;
    }
  }
//else
//{
//  FSMC_NAND_ReadID(&NAND_ID);
//  if (NAND_ID.Device_ID != 0 )
//  {
//    /* only one zone is used */
//    Mass_Block_Count[1] = NAND_ZONE_SIZE * NAND_BLOCK_SIZE * NAND_MAX_ZONE ;
//    Mass_Block_Size[1]  = NAND_PAGE_SIZE;
//    Mass_Memory_Size[1] = (Mass_Block_Count[1] * Mass_Block_Size[1]);
//    return MAL_OK;
//  }
//}
  GPIO_ResetBits(USB_LED_PORT, GPIO_Pin_7);
  return MAL_FAIL;
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
