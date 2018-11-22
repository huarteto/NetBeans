/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : usb_int.c
* Author             : MCD Application Team
* Date First Issued  : 03/10/2006 : V0.1
* Description        : Endpoint CTR interrupt service routine
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

extern void  (*pEpInt[15])(void);
/*----------------------------------------------------------------------------*/
void CTR_ISR()
{

 WORD wEPVal;
 	/* stay in loop while pending ints */
	while(((wIstr = _GetISTR()) & ISTR_CTR)!= 0)
	{
	    _SetISTR((WORD)CLR_CTR); /* clear CTR flag */
		/* extract highest priority endpoint number */
		EPindex = (BYTE)(wIstr & ISTR_EP_ID);
		if(EPindex == 0)
		{
/*==================================================================*/
/* Decode and service control endpoint interrupt */
/* calling related service routine */
/* (Setup0_Process, In0_Process, Out0_Process) */
/*==================================================================*/

			/* save RX & TX status */
			/* and set both to NAK */
			SaveRState = _GetEPRxStatus(ENDP0);
			SaveTState = _GetEPTxStatus(ENDP0);
			_SetEPRxStatus(ENDP0, EP_RX_NAK);
			_SetEPTxStatus(ENDP0, EP_TX_NAK);

		    /*-------------------------------------------------*/
			/* DIR bit = origin of the interrupt */
		    /*-------------------------------------------------*/
			if((wIstr & ISTR_DIR) == 0)
			{/* DIR = 0	*/
			    /*-------------------------------------------------*/
				/* DIR = 0      => IN  int */
				/* DIR = 0 implies that (EP_CTR_TX = 1) always  */
			    /*-------------------------------------------------*/
				/*		wEPVal = GetENDPOINT(ENDP0); */
				/*		if((wEPVal & EP_CTR_TX) != 0) */
				/*		{ */
							_ClearEP_CTR_TX(ENDP0);
							In0_Process();
				/*		} */
				/* check if SETUP arrived during IN processing */
				wEPVal = _GetENDPOINT(ENDP0);
				if((wEPVal & (EP_CTR_RX|EP_SETUP)) != 0)
				{
                                        _ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
					Setup0_Process();
				}

			}/* DIR = 0	*/
			else
			{/* DIR = 1 */

			    /*-------------------------------------------------*/
				/* DIR = 1 & CTR_RX       => SETUP or OUT int */
				/* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */
			    /*-------------------------------------------------*/
				wEPVal = _GetENDPOINT(ENDP0);
				if((wEPVal & EP_CTR_TX) != 0)
				{
					_ClearEP_CTR_TX(ENDP0);
					In0_Process();
				}
                               	if((wEPVal &EP_SETUP) != 0)
				{
					_ClearEP_CTR_RX(ENDP0); /* SETUP bit kept frozen while CTR_RX = 1 */
					Setup0_Process();
				}

                                else if((wEPVal & EP_CTR_RX) != 0)
				{
					_ClearEP_CTR_RX(ENDP0);
					Out0_Process();
				}

			}/* DIR = 1 */

			/* before terminate set Tx & Rx status */
			_SetEPRxStatus(ENDP0, SaveRState);
			_SetEPTxStatus(ENDP0, SaveTState);
                   //    a = _GetEPRxStatus(ENDP0); 
		}/* if(EPindex == 0) */
		else
		{
/*===============================================================*/
/* Decode and service non control endpoints interrupt  */
/*===============================================================*/
			/* process related endpoint register */
			wEPVal = _GetENDPOINT(EPindex);
			if((wEPVal & EP_CTR_RX) != 0)
			{
					/* clear int flag */
					_ClearEP_CTR_RX(EPindex);

			} /* if((wEPVal & EP_CTR_RX) */
			if((wEPVal & EP_CTR_TX) != 0)
			{
					/* clear int flag */
					_ClearEP_CTR_TX(EPindex);
			} /* if((wEPVal & EP_CTR_TX) != 0) */

			/* call service function */
			(*pEpInt[EPindex-1])();
		}/* if(EPindex == 0) else	*/

	}/* while(...)	*/

} /* CTR_ISR */

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/
