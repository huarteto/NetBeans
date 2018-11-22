/*
* Automotive Body/DashBoard Application MCU,S3F4A0K,
* Developed by Microcontroller Design Team
* 
* Copyright 2005 by MCU System Team, Samsung Electronics CO.,LTD.
* San #24 Nongseo-Dong,Giheung-Gu,
* 449-711 Youngin-City, Gyeongi-Do, Korea.
* All rights reserved.
*
* This software is the confidential and proprietary information of Samsung
* Electronics CO.,LTD. ("Confidential Information"). 
* You shall not disclose such Confidential Inforamtion and shall use it only
* in accordance with the terms of the license agreement you entered into
* with Samsung.
* 
* The software is delivered "AS IS" without warranty or condition of any 
* kind, either express, implied or statutory. This includes without limitation
* any warranty or condition with respect to merchantability or fitness for
* any particular purpose, or against the infringements of intellectual property
* rights of others. 
*/

/**
* @file name	csp_ifc.c
* @description	Function declarations for IFC module
*                
* @author	Juil Kim(82-31-209-2379, prodakim@samsung.com)   
*			Younghee Jin(82-31-209-4956, como.jin@samsung.com)
*			Minseok Jeong(82-31-209-3865, ms3525.jeong@samsung.com)		
*
* @version	Preliminary 0.0  
* @history	History type - NEW/MODify/ADD/DELete  
*  			 -------------------------------------------------------------------
*			|ver type when       who                       what             
*			|---+---+----------+-------------------------+----------------------
*			|0.0 NEW 01/07/2005  Samsung MCU System Team   Creation			
*	 		 -------------------------------------------------------------------
*
* @see	ADS Compiler Tool
*		Module/Drivers/Processor/Board -
*		IFC/ /S3F4A0K/AGP1 EVB Rev1.0
*/ 


/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"

/*******************************************************************************
**********************  G L O B A L  V A R I A B L E S  ************************
*******************************************************************************/
CSP_IFC_STATUS_T  CSP_IFCStatus_t;


/*******************************************************************************
****************************  F U N C T I O N S  *******************************
*******************************************************************************/
/**
********************************************************************************
* @description	Configure IFC Ip & CSP Driver
*
* @return	None		
*
* @param    mode_u32	[in] Setup IFC Mode register field
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_IFCInit(CSP_IFC_T *const ifc,U32_T mode_u32)
{
	/* Configure IFC mode register */
   	CSP_IFC_SET_MR(ifc,mode_u32);
   
   	/* CSP Driver Initialization */
   	CSP_IFCStatus_t.status_e = CSP_IFC_NOT_BUSY;	
}

/**
********************************************************************************
* @description	Configure IFC Interrupts
*
* @return	None		
*
* @param    intMode_u32	[in] Configure the priority level and source type
*			intMask_u32 [in] Configure which interrupt bits are activated
*			callback_u32[in] Function called through the assembler 
*                            interrupt handler
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_IFCConfigInterrupt(CSP_IFC_T *const ifc,
								U32_T intMode_u32, 
                            	U32_T intMask_u32, 
                            	U32_T callback_u32)
{
	/* Local Variables */
   	U8_T intNum_u8 = 0;

   	/* Get Peripheral Interrupt Number */
	intNum_u8 = CSP_GET_INT_NUMBER(ifc);
	
	/* Disable all interrupt */
   	CSP_IFC_SET_IDR(ifc,0xFFFFFFFF);

  	/* Interrupt Enable */
	CSP_IFC_SET_IER(ifc,intMask_u32);

   	/* Configure PWM4C controller interrupt mode in GIC module */
   	CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}
/**
********************************************************************************
* @description	Erase the entire chip
*
* @return	None		
*
* @param    mode_e	[in] The mode to wait the end of erase
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
CSP_IFC_ERROR_E CSP_IFCChipErase(CSP_IFC_T *const ifc, CSP_IFC_MODE_E mode_e)
{
	/* check flash not busy */
   	if( ((CSP_IFC_GET_SR(ifc) & IFC_BUSY) == IFC_BUSY) || 
	   	    (CSP_IFCStatus_t.status_e != CSP_IFC_NOT_BUSY) )
	{
   		return( CSP_IFC_BUSY );
	}

	if( mode_e == CSP_IFC_POLLING_ON_BUSY )
  	{
    	CSP_IFCStatus_t.status_e = CSP_IFC_ERASING;
		CSP_IFC_SET_IDR(ifc,IFC_ENDERASE);
     	CSP_IFC_SET_CR(ifc,(IFC_CRKEY | IFC_CE));
		while( (CSP_IFC_GET_SR(ifc) & IFC_BUSY) == IFC_BUSY)
     		; /*NULL*/
		
      	CSP_IFCStatus_t.status_e = CSP_IFC_NOT_BUSY;
      	return(CSP_IFC_OK);
 	}
   	else if( mode_e == CSP_IFC_POLLING )
   	{
    	CSP_IFCStatus_t.status_e = CSP_IFC_ERASING;
      	CSP_IFC_SET_IDR(ifc, IFC_ENDERASE);
      	CSP_IFC_SET_CR(ifc,(IFC_CRKEY | IFC_CE));
		while( (CSP_IFC_GET_SR(ifc) & IFC_ENDERASE) != IFC_ENDERASE)
			; /*NULL*/
      	CSP_IFCStatus_t.status_e = CSP_IFC_NOT_BUSY;
		return(CSP_IFC_OK);
   	}
   	else if( mode_e == CSP_IFC_INTERRUPT )
   	{
      	CSP_IFCStatus_t.status_e = CSP_IFC_ERASING;
      	CSP_IFC_SET_IER(ifc,IFC_ENDERASE); 
		CSP_IFC_SET_CR(ifc,(IFC_CRKEY | IFC_CE));
		return(CSP_IFC_OK);
  	}
   	else
   	{
    	/* Invalid Parameter*/
      	return( CSP_IFC_INVALID_PARAMETER);
  	}
}
/**
********************************************************************************
* @description	Erase a specific sector
*
* @return	None		
*
* @param    mode_e		[in] The mode to wait the end of erase
*			sector_u8	[in] The sector number to be erased
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
CSP_IFC_ERROR_E CSP_IFCSectorErase( CSP_IFC_T *const ifc,
									   CSP_IFC_MODE_E mode_e,
									   U8_T sector_u8)
{
	/* check flash not busy */
   	if( ((CSP_IFC_GET_SR(ifc) & IFC_BUSY) == IFC_BUSY) || 
	       (CSP_IFCStatus_t.status_e != CSP_IFC_NOT_BUSY) )
   	{
    	 return( CSP_IFC_BUSY );
  	}

   	/* Polling on IFC busy bit */
   	if( mode_e == CSP_IFC_POLLING_ON_BUSY )
   	{
    	CSP_IFCStatus_t.status_e = CSP_IFC_ERASING;
      	CSP_IFC_SET_IDR(ifc, IFC_ENDERASE);
		CSP_IFC_SET_CR(ifc,(IFC_SECTOR(sector_u8) | IFC_CRKEY | IFC_SE));
		while( (CSP_IFC_GET_SR(ifc) & IFC_BUSY) == IFC_BUSY)
			; /*NULL*/
		CSP_IFCStatus_t.status_e = CSP_IFC_NOT_BUSY;
      	return(CSP_IFC_OK);
   	}
   	/* Polling on IFC ENDERASE bit */
   	else if( mode_e == CSP_IFC_POLLING )
   	{
    	CSP_IFCStatus_t.status_e = CSP_IFC_ERASING;
      	CSP_IFC_SET_IDR(ifc,IFC_ENDERASE);
		CSP_IFC_SET_CR(ifc,(IFC_SECTOR(sector_u8) | IFC_CRKEY | IFC_SE));
 		while( (CSP_IFC_GET_SR(ifc) & IFC_ENDERASE) != IFC_ENDERASE)
			; /*NULL*/
		CSP_IFCStatus_t.status_e = CSP_IFC_NOT_BUSY;
     	return(CSP_IFC_OK);
   	}
   	/* Interrupt Mode -> ENDERASE bit */
   	else if( mode_e == CSP_IFC_INTERRUPT )
   	{
    	CSP_IFCStatus_t.status_e = CSP_IFC_ERASING;
      	CSP_IFC_SET_IER(ifc, IFC_ENDERASE);
		CSP_IFC_SET_CR(ifc,(IFC_SECTOR(sector_u8) | IFC_CRKEY | IFC_SE));
		return(CSP_IFC_OK);
   	}
   	else
   	{
    	/* Invalid Parameter*/
      	return( CSP_IFC_INVALID_PARAMETER);
   	}
}

/**
********************************************************************************
* @description	Write data into flash
*
* @return	None		
*
* @param    mode_e		[in] The mode to wait the end of write
*			dest_ptr_u32[in] Pointer to the destination address
*			src_ptr_u32	[in] Pointer to the source address
*			nb_data_u32 [in] Number of data to be written	
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
CSP_IFC_ERROR_E CSP_IFCWrite( CSP_IFC_T *const ifc,
								CSP_IFC_MODE_E mode_e, 
                             	U32_T *dest_ptr_u32, 
                              	U32_T *src_ptr_u32, 
                              	U32_T nb_data_u32)
{
	/* Local variable */
   	U32_T i_u32;
  
   	/* check flash not busy */
   	if( ((CSP_IFC_GET_SR(ifc) & IFC_BUSY) == IFC_BUSY) || 
			(CSP_IFCStatus_t.status_e != CSP_IFC_NOT_BUSY) )
	{
    	return(CSP_IFC_BUSY);
   	}
   
   	/* initialize pointer */   
   	CSP_IFCStatus_t.source_ptr_u32      = src_ptr_u32;
   	CSP_IFCStatus_t.destination_ptr_u32 = dest_ptr_u32;
   	CSP_IFCStatus_t.nb_data_u32         = nb_data_u32;

  	/* Polling on IFC busy bit */
   	if( mode_e == CSP_IFC_POLLING_ON_BUSY )
   	{
    	CSP_IFCStatus_t.status_e = CSP_IFC_WRITING;
      	for(i_u32=0; i_u32<CSP_IFCStatus_t.nb_data_u32; i_u32++)
      	{
        	*(CSP_IFCStatus_t.destination_ptr_u32) =
        			*(CSP_IFCStatus_t.source_ptr_u32);
         	CSP_IFCStatus_t.destination_ptr_u32++;
         	CSP_IFCStatus_t.source_ptr_u32++;            
         	while( (CSP_IFC_GET_SR(ifc) & IFC_BUSY) == IFC_BUSY)
     			;/*NULL*/
    	}
   		CSP_IFCStatus_t.status_e = CSP_IFC_NOT_BUSY;
   		return(CSP_IFC_OK);
   	}
   	/* Polling on IFC ENDWR bit */
   	else if( mode_e == CSP_IFC_POLLING )
   	{
   		CSP_IFCStatus_t.status_e = CSP_IFC_WRITING;
      	for(i_u32=0; i_u32<nb_data_u32; i_u32++)
      	{
        	*(CSP_IFCStatus_t.destination_ptr_u32) = 
        		*(CSP_IFCStatus_t.source_ptr_u32);
         	CSP_IFCStatus_t.destination_ptr_u32++;
         	CSP_IFCStatus_t.source_ptr_u32++;            
         	while( (CSP_IFC_GET_SR(ifc) & IFC_ENDWR) != IFC_ENDWR)
     			; /*NULL*/
     	}
     	CSP_IFCStatus_t.status_e = CSP_IFC_NOT_BUSY;
      	return(CSP_IFC_OK);
   	}
   	/* Interrupt Mode -> ENDERASE bit */
   	else if( mode_e == CSP_IFC_INTERRUPT )
   	{
    	CSP_IFCStatus_t.status_e = CSP_IFC_WRITING;
      	CSP_IFC_SET_IER(ifc,IFC_ENDWR);
		*(CSP_IFCStatus_t.destination_ptr_u32) = 
			*(CSP_IFCStatus_t.source_ptr_u32);
      	CSP_IFCStatus_t.destination_ptr_u32++;
      	CSP_IFCStatus_t.source_ptr_u32++;            
      	CSP_IFCStatus_t.nb_data_u32--;  
      	return(CSP_IFC_OK);
   	}
  	else
   	{
    	/* Invalid Parameter*/
      	return( CSP_IFC_INVALID_PARAMETER);
   	}
}
