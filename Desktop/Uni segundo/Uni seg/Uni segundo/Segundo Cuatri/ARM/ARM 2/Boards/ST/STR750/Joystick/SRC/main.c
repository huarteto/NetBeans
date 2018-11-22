/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Date First Issued  : 03/10/2006 : V0.1
* Description        : Main program body
********************************************************************************
* History:
* 03/10/2006 : V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "75x_lib.h"
#include "USB_lib.h"
#include "USB_conf.h"
#include "USB_prop.h"
#include "USB_pwr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef    GPIO_InitStructure;
EIC_IRQInitTypeDef  EIC_IRQInitStructure;

// directions of transfer for CopyBuffer
#define USR_TO_PMA 0
#define PMA_TO_USR 1

// key codes for direction
#define  KEY_NOKEY  0
#define  KEY_SEL    1 
#define  KEY_RIGHT  2
#define  KEY_LEFT   3
#define  KEY_UP     4
#define  KEY_DOWN   5
 
#define CURSOR_STEP     10

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name : gInit
* Description   : Hardware initialisation
* Input         : None
* Output        : None
* Return value  : None
*******************************************************************************/
void gInit(void)
{
  wInterrupt_Mask = IMR_MSK;

  /* MRCC configuration --------------------------------------------------------*/
  MRCC_DeInit();

  /* Enable GPIOs and EXTIT clocks */
  MRCC_PeripheralClockConfig(MRCC_Peripheral_GPIO | MRCC_Peripheral_USB, ENABLE);

  /* Clear No Clock Detected flag */
  MRCC_ClearFlag(MRCC_FLAG_NCKD);

  /* Enbale No Clock Detected interrupt */
  MRCC_ITConfig(MRCC_IT_NCKD, ENABLE);

  /* Set HCLK to 60 MHz */
  MRCC_HCLKConfig(MRCC_CKSYS_Div1);

  /* Set CKTIM to 30 MHz */
  MRCC_CKTIMConfig(MRCC_HCLK_Div2);

  /* Set PCLK to 30 MHz */
  MRCC_PCLKConfig(MRCC_CKTIM_Div1);

  /* Enable FLASH burst mode */
  CFG_FLASHBurstConfig(CFG_FLASHBurst_Enable);

  /* Set CKSYS to 60 MHz */
  MRCC_CKSYSConfig(MRCC_CKSYS_OSC4MPLL, MRCC_PLL_Mul_15);

  /* Set USB kernel clock to 48 MHz */
  MRCC_CKUSBConfig (MRCC_CKUSB_Internal); 

  /* Enable and configure the priority of the USB_LP IRQ Channel*/
  EIC_IRQInitStructure.EIC_IRQChannel = USB_LP_IRQChannel;
  EIC_IRQInitStructure.EIC_IRQChannelPriority = 2;
  EIC_IRQInitStructure.EIC_IRQChannelCmd = ENABLE;
  EIC_IRQInit(&EIC_IRQInitStructure);

  /* Enable the Interrupt controller to manage IRQ channel*/
  EIC_IRQCmd(ENABLE);
} 

/*******************************************************************************
* Function Name : CopyBuffer
* Description   : Transfers a packet of data from/to PMA to/from user memory
* Input 1       : iTrDirection: USR_TO_PMA or PMA_TO_USR
* Input 2       : EpNum: endpoint number
* Input 3       : Usr_buffer: a pointer to user memory
* Input 4       : Nbytes: number of bytes to transfer
* Output        : None
* Return value  : None
*******************************************************************************/
void CopyBuffer(int iTrDirection, BYTE EpNum, BYTE *Usr_buffer, int Nbytes)
{
  DWORD *pTxBuff;
  WORD wTra;
  BYTE *pbTra;
  int i;

  if (iTrDirection == USR_TO_PMA)
  {
    pTxBuff = (DWORD *)(PMAAddr + (BYTE *)(GetEPTxAddr(EpNum)*2));

    for(i = 0; i < Nbytes;)
    {
      pbTra = (BYTE *)&wTra;
      *pbTra++ = *Usr_buffer++;
      i++;

      if (i < Nbytes) /* check for ODD transfers */
      {
        *pbTra = *Usr_buffer++;
      }
      else
      {
        *pbTra = 0;
      }

      *pTxBuff++ = wTra;
      i++;
    }
  }
} 

/*******************************************************************************
* Function Name : Mouse_Send
* Description   : Decodes commands arriving from serial and
*                 prepares buffer to be sent containing mouse event infos
* Input         : Keys: keys received from terminal
* Output        : None
* Return value  : None
*******************************************************************************/
void Mouse_Send(BYTE Keys)
{
  BYTE Mouse_Buffer[3] = {0,0,0};
  char X = 0, Y = 0;

  switch (Keys)
  {
    case KEY_LEFT:
      X -= CURSOR_STEP;
      break;

    case KEY_RIGHT:
      X += CURSOR_STEP;
      break;

    case KEY_UP:
      Y -= CURSOR_STEP;
      break;

    case KEY_DOWN:
      Y += CURSOR_STEP;
      break;

      default:
	  return;
  }

  /* prepare buffer to send */
  Mouse_Buffer[1] = X;
  Mouse_Buffer[2] = Y;
  CopyBuffer(USR_TO_PMA, ENDP1, Mouse_Buffer, 3);

  /* enable endpoint for transmission */
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxCount(ENDP1, 4);
  SetEPTxValid(ENDP1);

} 

/*******************************************************************************
* Function Name  : ReadUSBKeys
* Description    : Reads key from demoboard.
* Input          : None
* Output         : None
* Return         : Return KEY_RIGHT, KEY_LEFT, KEY_SEL, KEY_UP, KEY_DOWN or KEY_NOKEY
*******************************************************************************/
u8 ReadUSBKeys ( void )
{
  /* "RIGHT" key is pressed */
  if(!GPIO_ReadBit(GPIO1, GPIO_Pin_13))
  {
    return KEY_RIGHT; 
  }
  /* "LEFT" key is pressed */
  if(!GPIO_ReadBit(GPIO1, GPIO_Pin_12))
  {
    return KEY_LEFT; 
  }
  /* "SEL" key is pressed */
  if(!GPIO_ReadBit(GPIO0, GPIO_Pin_2))
  {
    return KEY_SEL; 
  }
  /* "UP" key is pressed */
  if(!GPIO_ReadBit(GPIO0, GPIO_Pin_8))
  {
    return KEY_UP; 
  }
  /* "DOWN" key is pressed */
  if(!GPIO_ReadBit(GPIO1, GPIO_Pin_7))
  {
    return KEY_DOWN; 
  }
  /* No key is pressed */
  else 
  {
    return KEY_NOKEY;
  }
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main()
{
 
  #ifdef DEBUG
   debug();
  #endif

  /* USB  initialization */
    gInit();  
    USB_Init();


  while(1)
  {
    Mouse_Send(ReadUSBKeys());           
  }
    
}

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/

