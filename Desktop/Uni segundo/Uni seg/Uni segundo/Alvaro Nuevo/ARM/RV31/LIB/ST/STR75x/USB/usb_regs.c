/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : usb_regs.c
* Author             : MCD Application Team
* Date First Issued  : 03/10/2006 : V0.1
* Description        : Interface functions to USB cell registers
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

/*----------------------------------------------------------------*/
/* Local declarations						  */
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
void SetCNTR(WORD wRegValue )
{
	_SetCNTR(wRegValue);
} /* SetCNTR */
/*----------------------------------------------------------------*/
WORD GetCNTR(void)
{
	return(_GetCNTR());
} /* GetCNTR */
/*----------------------------------------------------------------*/
void SetISTR(WORD wRegValue )
{
	_SetISTR(wRegValue);
} /* SetISTR */
/*----------------------------------------------------------------*/
WORD GetISTR(void)
{
	return(_GetISTR());
} /* GetISTR */
/*----------------------------------------------------------------*/
WORD GetFNR(void)
{
	return(_GetFNR());
} /* GetFNR */
/*----------------------------------------------------------------*/
void SetDADDR(WORD wRegValue )
{
	_SetDADDR(wRegValue);
} /* SetDADDR */
/*----------------------------------------------------------------*/
WORD GetDADDR(void)
{
	return(_GetDADDR());
} /* GetDADDR */
/*----------------------------------------------------------------*/
void SetBTABLE(WORD wRegValue )
{
	_SetBTABLE(wRegValue);
} /* SetBTABLE */
/*----------------------------------------------------------------*/
WORD GetBTABLE(void)
{
	return(_GetBTABLE());
} /* GetBTABLE */
/*----------------------------------------------------------------*/
void SetENDPOINT(BYTE bEpNum, WORD wRegValue )
{
	_SetENDPOINT(bEpNum,wRegValue);
} /* SetENDPOINT */

/*----------------------------------------------------------------*/
WORD GetENDPOINT(BYTE bEpNum)
{
	return(_GetENDPOINT(bEpNum));
} /* GetENDPOINT */

/*----------------------------------------------------------------*/
void SetEPType(BYTE bEpNum, WORD wType)
{
	_SetEPType(bEpNum, wType);
} /* SetEPType */

/*----------------------------------------------------------------*/
WORD GetEPType(BYTE bEpNum)
{
	return(_GetEPType(bEpNum));
}/*GetEPType */

/*----------------------------------------------------------------*/
void SetEPTxStatus(BYTE bEpNum, WORD wState)
{
	_SetEPTxStatus(bEpNum,wState);
} /* SetEPTxStatus */

/*----------------------------------------------------------------*/
void SetEPRxStatus(BYTE bEpNum, WORD wState)
{
	_SetEPRxStatus(bEpNum,wState);
} /* SetEPRxStatus */

/*----------------------------------------------------------------*/
WORD GetEPTxStatus(BYTE bEpNum)
{
	return(_GetEPTxStatus(bEpNum));
}/*GetEPTxStatus */

/*----------------------------------------------------------------*/
WORD GetEPRxStatus(BYTE bEpNum)
{
	return(_GetEPRxStatus(bEpNum));
}/*GetEPRxStatus */

/*----------------------------------------------------------------*/
void SetEPTxValid(BYTE bEpNum)
{
 	_SetEPTxStatus(bEpNum, EP_TX_VALID);
} /* SetEPTxValid */

/*----------------------------------------------------------------*/
void SetEPRxValid(BYTE bEpNum)
{
 	_SetEPRxStatus(bEpNum, EP_RX_VALID);
} /* SetEPRxValid */

/*----------------------------------------------------------------*/
void SetEP_KIND(BYTE bEpNum)
{
	_SetEP_KIND(bEpNum);
}/*SetEP_KIND */

/*----------------------------------------------------------------*/
void ClearEP_KIND(BYTE bEpNum)
{
	_ClearEP_KIND(bEpNum);
}/*ClearEP_KIND */

/*----------------------------------------------------------------*/
void Clear_Status_Out(BYTE bEpNum){
	 _ClearEP_KIND(bEpNum);
} /* Clear_Status_Out */

/*----------------------------------------------------------------*/
void Set_Status_Out(BYTE bEpNum)
{
	_SetEP_KIND(bEpNum);
} /* Set_Status_Out	*/

/*----------------------------------------------------------------*/
void SetEPDoubleBuff(BYTE bEpNum)
{
	 _SetEP_KIND(bEpNum);
} /* SetEPDoubleBuff */

/*----------------------------------------------------------------*/
void ClearEPDoubleBuff(BYTE bEpNum)
{
	 _ClearEP_KIND(bEpNum);
} /* ClearEPDoubleBuff */

/*----------------------------------------------------------------*/
BOOL GetTxStallStatus(BYTE bEpNum)
{
	return(_GetTxStallStatus(bEpNum));
}/*GetTxStallStatus */

/*----------------------------------------------------------------*/
BOOL GetRxStallStatus(BYTE bEpNum)
{
	return(_GetRxStallStatus(bEpNum));
}/*GetRxStallStatus */

/*----------------------------------------------------------------*/
void ClearEP_CTR_RX(BYTE bEpNum)
{
	_ClearEP_CTR_RX(bEpNum);
} /* ClearEP_CTR_RX */

/*----------------------------------------------------------------*/
void ClearEP_CTR_TX(BYTE bEpNum)
{
	_ClearEP_CTR_TX(bEpNum);
} /* ClearEP_CTR_TX */

/*----------------------------------------------------------------*/
void ToggleDTOG_RX(BYTE bEpNum)
{
	_ToggleDTOG_RX(bEpNum);
}/*ToggleDTOG_RX */

/*----------------------------------------------------------------*/
void ToggleDTOG_TX(BYTE bEpNum)
{
	_ToggleDTOG_TX(bEpNum);
}/*ToggleDTOG_TX */

/*----------------------------------------------------------------*/
void ClearDTOG_RX(BYTE bEpNum)
{
	_ClearDTOG_RX(bEpNum);
}/*ClearDTOG_RX */

/*----------------------------------------------------------------*/
void ClearDTOG_TX(BYTE bEpNum)
{
	_ClearDTOG_TX(bEpNum);
}/*ClearDTOG_TX */

/*----------------------------------------------------------------*/
void SetEPAddress(BYTE bEpNum,BYTE bAddr)
{
	_SetEPAddress(bEpNum,bAddr);
}/*SetEPAddress */

/*----------------------------------------------------------------*/
BYTE GetEPAddress(BYTE bEpNum)
{
	return(_GetEPAddress(bEpNum));
}/*GetEPAddress */

/*----------------------------------------------------------------*/
void SetEPTxAddr(BYTE bEpNum, WORD wAddr)
{
	_SetEPTxAddr(bEpNum,wAddr);
} /* SetEPTxAddr */

/*----------------------------------------------------------------*/
void SetEPRxAddr(BYTE bEpNum, WORD wAddr)
{
	 _SetEPRxAddr(bEpNum,wAddr);
} /* SetEPRxAddr */

/*----------------------------------------------------------------*/
WORD GetEPTxAddr(BYTE bEpNum)
{
	 return(_GetEPTxAddr(bEpNum));
} /* GetEPTxAddr */

/*----------------------------------------------------------------*/
WORD GetEPRxAddr(BYTE bEpNum)
{
	 return(_GetEPRxAddr(bEpNum));
} /* GetEPRxAddr */

/*----------------------------------------------------------------*/
void SetEPTxCount(BYTE bEpNum, WORD wCount)
{
	_SetEPTxCount(bEpNum,wCount);
} /* SetEPTxCount */

/*----------------------------------------------------------------*/
void SetEPCountRxReg(DWORD *pdwReg, WORD wCount)
{
	_SetEPCountRxReg(dwReg, wCount);
} /* SetEPCountRxReg */

/*----------------------------------------------------------------*/
void SetEPRxCount(BYTE bEpNum, WORD wCount)
{
	_SetEPRxCount(bEpNum,wCount);
} /* SetEPRxCount */

/*----------------------------------------------------------------*/
WORD GetEPTxCount(BYTE bEpNum)
{
	 return(_GetEPTxCount(bEpNum));
} /* GetEPTxCount */

/*----------------------------------------------------------------*/
WORD GetEPRxCount(BYTE bEpNum)
{
	 return(_GetEPRxCount(bEpNum));
} /* GetEPRxCount */

/*----------------------------------------------------------------*/
void SetEPDblBuffAddr(BYTE bEpNum, WORD wBuf0Addr, WORD wBuf1Addr)
{
	_SetEPDblBuffAddr(bEpNum, wBuf0Addr, wBuf1Addr);
}/* SetEPDblBuffAddr */

/*----------------------------------------------------------------*/
void SetEPDblBuf0Addr(BYTE bEpNum,WORD wBuf0Addr)
{
	_SetEPDblBuf0Addr(bEpNum, wBuf0Addr);
} /* SetEPDblBuf0Addr */

/*----------------------------------------------------------------*/
void SetEPDblBuf1Addr(BYTE bEpNum,WORD wBuf1Addr)
{
	_SetEPDblBuf1Addr(bEpNum, wBuf1Addr);
} /* SetEPDblBuf1Addr */

/*----------------------------------------------------------------*/
WORD GetEPDblBuf0Addr(BYTE bEpNum)
{
	return(_GetEPDblBuf0Addr(bEpNum));
} /* GetEPDblBuf0Addr */

/*----------------------------------------------------------------*/
WORD GetEPDblBuf1Addr(BYTE bEpNum)
{
	return(_GetEPDblBuf1Addr(bEpNum));
} /* GetEPDblBuf1Addr */

/*----------------------------------------------------------------*/
void SetEPDblBuffCount(BYTE bEpNum, BYTE bDir, WORD wCount)
{
	 _SetEPDblBuffCount(bEpNum, bDir, wCount);
} /* SetEPDblBuffCount */

/*----------------------------------------------------------------*/
void SetEPDblBuf0Count(BYTE bEpNum, BYTE bDir,WORD wCount)
{
	_SetEPDblBuf0Count(bEpNum,bDir,wCount);
} /* SetEPDblBuf0Count */

/*----------------------------------------------------------------*/
void SetEPDblBuf1Count(BYTE bEpNum, BYTE bDir,WORD wCount)
{
	_SetEPDblBuf1Count(bEpNum,bDir,wCount);
} /* SetEPDblBuf1Count */

/*----------------------------------------------------------------*/
WORD GetEPDblBuf0Count(BYTE bEpNum)
{
	 return(_GetEPDblBuf0Count(bEpNum));
} /* GetEPDblBuf0Count */

/*----------------------------------------------------------------*/
WORD GetEPDblBuf1Count(BYTE bEpNum)
{
	 return(_GetEPDblBuf1Count(bEpNum));
} /* GetEPDblBuf1Count */

/*----------------------------------------------------------------*/
EP_DBUF_DIR GetEPDblBufDir(BYTE bEpNum) {
	 if((WORD)(*_pEPRxCount(bEpNum) & 0xFC00) != 0)
	 		return(EP_DBUF_OUT);
	 else if(((WORD)(*_pEPTxCount(bEpNum)) & 0x03FF) != 0)
	 		return(EP_DBUF_IN);
	 else
	 		return(EP_DBUF_ERR);
} /* GetEPDblBufDir */

/*----------------------------------------------------------------*/
void FreeUserBuffer(BYTE bEpNum)
{
	 if(GetEPDblBufDir(bEpNum) == EP_DBUF_OUT)
	 { /* OUT endpoint */
	 	_ToggleDTOG_TX(bEpNum);
	 }
	 else if(GetEPDblBufDir(bEpNum) == EP_DBUF_IN)
	 { /* IN endpoint */
	 	_ToggleDTOG_RX(bEpNum);
	 }
} /* FreeUserBuffer */

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* ToWord
  Puts 2 bytes into a single word
  IN  : bh & bl
  OUT : return a word */
/*----------------------------------------------------------------*/
WORD ToWord(BYTE bh, BYTE bl)
{
 WORD wRet;
   wRet = (WORD)bl | ((WORD)bh << 8);
   return(wRet);
}/*	ToWord */

/*----------------------------------------------------------------*/
/* ByteSwap
   Swaps two bytes in a word
   IN  : word to swap
   OUT : return word swapped */
/*----------------------------------------------------------------*/
WORD ByteSwap(WORD wSwW)
{
 BYTE bTemp;
 WORD wRet;
   bTemp = (BYTE)(wSwW & 0xff);
   wRet =  (wSwW >> 8) | ((WORD)bTemp << 8);
   return(wRet);
} /* ByteSwap */


/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
