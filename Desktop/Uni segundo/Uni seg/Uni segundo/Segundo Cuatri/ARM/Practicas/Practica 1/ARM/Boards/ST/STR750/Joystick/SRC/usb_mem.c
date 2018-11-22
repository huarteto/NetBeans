/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : usb_mem.c
* Author             : MCD Application Team
* Date First Issued  : 03/10/2006 : V0.1
* Description        : Utility functions for memory transfers
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
#include "USB_lib.h"
#include "USB_mem.h"
#include "75x_lib.h"

/* ----------------------------------------------------------------------------*/
/* UserToPMABufferCopy */
/* copy a buffer from user memory area to packet memory area (PMA) */
/* IN : pbUsrBuf = pointer to user memory area */
/*      wPMABufAddr = address into PMA */
/*		wNBytes = no. of bytes to be copied */
/* OUT: none */
/* ----------------------------------------------------------------------------*/
void UserToPMABufferCopy(BYTE *pbUsrBuf,WORD wPMABufAddr, WORD wNBytes)
{
 BYTE *pbVal;
 WORD wVal;
 DWORD *pdwVal;
 int wNTrasf=wNBytes;

	 if((wNBytes) == 0) return;
	 pdwVal = (DWORD *)(wPMABufAddr*2 + PMAAddr);
	 while(TRUE)
	 {
		wVal = 0;
		pbVal = (BYTE *)&wVal;
	 	if((wNTrasf--) > 0)
	 	{
		 	*pbVal++ = *pbUsrBuf++;
	 	 	if((wNTrasf--) > 0)
	 	 	{
			 	*pbVal++ = *pbUsrBuf++;
	 	 	}
	 	}
		*pdwVal++ = (DWORD)wVal;
		if(wNTrasf == 0) return;
	 }/* while */

} /* UserToPMABufferCopy */

/* ----------------------------------------------------------------------------*/
/* PMAToUserBufferCopy */
/* copy a buffer from user memory area to packet memory area (PMA) */
/* IN : pbUsrBuf = pointer to user memory area */
/*      wPMABufAddr = address into PMA */
/*		wNBytes = no. of bytes to be copied */
/* OUT: none */
/* ----------------------------------------------------------------------------*/
void PMAToUserBufferCopy(BYTE *pbUsrBuf,WORD wPMABufAddr, WORD wNBytes)
{
 BYTE *pbVal;
 WORD wNTrasf=wNBytes;

	 if((wNBytes) == 0) return;
	 pbVal = (BYTE *)(wPMABufAddr*2 + PMAAddr);
	 while(TRUE)
	 {
	 	 *pbUsrBuf++ = *pbVal++;
	 	 if((--wNTrasf) == 0) return;
	 	 *pbUsrBuf++ = *pbVal++;
	 	 if((--wNTrasf) == 0) return;
		 pbVal++; pbVal++;
	 }/* while */

} /* PMAToUserBufferCopy */


/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/

