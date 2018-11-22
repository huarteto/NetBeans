/******************** (C) COPYRIGHT 2006 STMicroelectronics ********************
* File Name          : usb_istr.c
* Author             : MCD Application Team
* Date First Issued  : 03/10/2006 : V0.1
* Description        : ISTR events interrupt service routines
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

volatile WORD wIstr;  /* ISTR register last read value */
WORD wIstrCnt[9]={0,0,0,0,0,0,0,0,0};
typedef enum _EVCOUNT{CTRcnt,DOVRcnt,ERRcnt,WKUPcnt,SUSPcnt,RESETcnt,SOFcnt,ESOFcnt,GENERcnt} EVCOUNT;

volatile BYTE bIntPackSOF=0;  /* SOFs received between 2 consecutive packets */
BOOL fConnected=FALSE; /* connection status */

void USB_Istr(void);

/* function prototypes */
/* automatically built defining related macros */
#ifdef CTR_Callback
	void CTR_Callback(void);
#endif
#ifdef DOVR_Callback
	void DOVR_Callback(void);
#endif
#ifdef ERR_Callback
	void ERR_Callback(void);
#endif
#ifdef WKUP_Callback
	void WKUP_Callback(void);
#endif
#ifdef SUSP_Callback
	void SUSP_Callback(void);
#endif
#ifdef RESET_Callback
	void RESET_Callback(void);
#endif
#ifdef SOF_Callback
	void SOF_Callback(void);
#endif
#ifdef ESOF_Callback
	void ESOF_Callback(void);
#endif

extern void EP1_Callback(void);
extern void EP2_Callback(void);
extern void EP3_Callback(void);
extern void EP4_Callback(void);
extern void EP5_Callback(void);
extern void EP6_Callback(void);
extern void EP7_Callback(void);
extern void EP8_Callback(void);
extern void EP9_Callback(void);
extern void EP10_Callback(void);
extern void EP11_Callback(void);
extern void EP12_Callback(void);
extern void EP13_Callback(void);
extern void EP14_Callback(void);
extern void EP15_Callback(void);

/* function pointers to non-control endpoints service routines */
void (*pEpInt[15])(void)={
	EP1_Callback,
	EP2_Callback,
	EP3_Callback,
	EP4_Callback,
	EP5_Callback,
	EP6_Callback,
	EP7_Callback,
	EP8_Callback,
	EP9_Callback,
	EP10_Callback,
	EP11_Callback,
	EP12_Callback,
	EP13_Callback,
	EP14_Callback,
	EP15_Callback
};

/*==============================================================================*/
/* USB_Istr */
/*==============================================================================*/
void USB_Istr(void)
{

	wIstr = _GetISTR();
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_RESET)
	if (wIstr & ISTR_RESET & wInterrupt_Mask) {
		_SetISTR((WORD)CLR_RESET);
		Device_Property.Reset();
#ifdef RESET_Callback
    	RESET_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if	(IMR_MSK & ISTR_DOVR)
	if (wIstr & ISTR_DOVR & wInterrupt_Mask) {
		_SetISTR((WORD)CLR_DOVR);
#ifdef DOVR_Callback
		DOVR_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if	(IMR_MSK & ISTR_ERR)
	if (wIstr & ISTR_ERR & wInterrupt_Mask) {
		_SetISTR((WORD)CLR_ERR);
#ifdef ERR_Callback
		ERR_Callback();
#endif
	}
#endif

	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if	(IMR_MSK & ISTR_WKUP)
	if (wIstr & ISTR_WKUP & wInterrupt_Mask) {
		_SetISTR((WORD)CLR_WKUP);
		Resume(RESUME_EXTERNAL);
#ifdef WKUP_Callback
		WKUP_Callback();
#endif
	}
#endif



	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if	(IMR_MSK & ISTR_SUSP)
	if (wIstr & ISTR_SUSP & wInterrupt_Mask) {

		/* check if SUSPEND is possible */
		if(fSuspendEnabled)
		{
			Suspend();
		}
		else
		{
			/* if not possible then resume after xx ms */
		    Resume(RESUME_LATER);
		}
		/* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
		_SetISTR((WORD)CLR_SUSP);
#ifdef SUSP_Callback
		SUSP_Callback();
#endif
	}
#endif



	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_SOF)
	if (wIstr & ISTR_SOF & wInterrupt_Mask) {
		_SetISTR((WORD)CLR_SOF);
		bIntPackSOF++;

#ifdef SOF_Callback
		SOF_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if (IMR_MSK & ISTR_ESOF)
	if (wIstr & ISTR_ESOF & wInterrupt_Mask) {
		_SetISTR((WORD)CLR_ESOF);
		/* resume handling timing is made with ESOFs */
		Resume(RESUME_ESOF); /* request without change of the machine state */

#ifdef ESOF_Callback
		ESOF_Callback();
#endif
	}
#endif
	/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#if	(IMR_MSK & ISTR_CTR)
	if (wIstr & ISTR_CTR & wInterrupt_Mask) {
			/* servicing of the endpoint correct transfer interrupt */
			/* clear of the CTR flag into the sub */
			CTR_ISR();
#ifdef CTR_Callback
		CTR_Callback();
#endif
	}
#endif
} /* USB_Istr */
/*==============================================================================*/
/*==============================================================================*/
#ifdef CTR_Callback
void count_CTR()
{
	wIstrCnt[CTRcnt]++;
}
#endif
/*==============================================================================*/
#ifdef DOVR_Callback
void count_DOVR()
{
	wIstrCnt[DOVRcnt]++;
}
#endif
/*==============================================================================*/
#ifdef ERR_Callback
void count_ERR()
{
	wIstrCnt[ERRcnt]++;
}
#endif
/*==============================================================================*/
#ifdef WKUP_Callback
void count_WKUP()
{
	wIstrCnt[WKUPcnt]++;
}
#endif
/*==============================================================================*/
#ifdef SUSP_Callback
void count_SUSP()
{
	wIstrCnt[SUSPcnt]++;
}
#endif
/*==============================================================================*/
#ifdef RESET_Callback
void count_RESET()
{
	wIstrCnt[RESETcnt]++;
}
#endif
/*==============================================================================*/
#ifdef SOF_Callback
void count_SOF()
{
	wIstrCnt[SOFcnt]++;
}
#endif
/*==============================================================================*/
#ifdef ESOF_Callback
void count_ESOF()
{
	wIstrCnt[ESOFcnt]++;
}
#endif

/******************* (C) COPYRIGHT 2006 STMicroelectronics *****END OF FILE****/

