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
* @file name	csp_pio.c
* @description	Function declarations for Peripheral Input Output module
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
*		PIO/ /S3F4A0K/AGP1 EVB Rev1.0
*/ 



/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"


/******************************************************************************
****************************  F U N C T I O N S  ******************************
******************************************************************************/

/**
********************************************************************************
* @description	Configure PIO
*
* @return	None		
*
* @param    pio				[in] Pointer to PIO structure.
* 			pioMask_u32		[in] Configure which pins are activated.
* 			pioOutput_u32	[in] Configure which pins are configured as output.

*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PIOInit(CSP_PIO_T *const pio, U32_T pioOutput_u32, U32_T pioMultidriver_u32)
{
      /* Enable PIO block */
   CSP_PIO_SET_ECR(pio, PIO_PIO);

   /* PIO Software Reset */
   CSP_PIO_SET_CR(pio, PIO_SWRST);

   /* Set Multi-Driver PIO */
   CSP_PIO_SET_MDER(pio, pioMultidriver_u32);

   /* Set Output PIO */
   CSP_PIO_SET_OER(pio, pioOutput_u32);
}

/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None		
*
* @param    pio				[in] Pointer to PIO structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PIOClose(CSP_PIO_T *const pio)
{
   /* PIO Software Reset */
   CSP_PIO_SET_CR(pio, PIO_SWRST);

   /* Disable PIO Clock */  
   CSP_PIO_SET_DCR(pio, PIO_PIO);
}

/**
********************************************************************************
* @description	Configure the PIO Interrupts
*
* @return	None		
*
* @param    pio				[in] Pointer to PIO structure.
*			intMode_u32		[in] Configure the priority level and source type.
*			intMask_u32		[in] Configure which interrupt bits are activated.
*			callback_u32	[in] Function called through the assembler 
*                                interrupt handler.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PIOConfigInterrupt(CSP_PIO_T *const pio, 
                             U32_T intMode_u32, 
                             U32_T intMask_u32, 
                             U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum = 0;

   /* Get Peripheral Interrupt Number */
   intNum = CSP_GET_INT_NUMBER(pio);

   /* Disable all interrupt */
   CSP_PIO_SET_IDR(pio, 0xFFFFFFFF);
      
   /* Interrupt Enable */
   CSP_PIO_SET_IER(pio, intMask_u32);

   /* Configure PIOURE controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum, intMode_u32, callback_u32);
}

/**
********************************************************************************
* @description	Read the pin data status 
*
* @return	None		
*
* @param    pio				[in] Pointer to PIO structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_PIOGetStatus(CSP_PIO_T *const pio)
{
   /* Return PIO State */
   return (CSP_PIO_GET_PDSR(pio));
}

/**
********************************************************************************
* @description	Set the PIO to low level 
*
* @return	None		
*
* @param    pio				[in] Pointer to PIO structure.
*			pioMask_u32		[in] Configure which pins are set to low level.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PIOClear(CSP_PIO_T *const pio, U32_T pioMask_u32)
{
   /* Set PIO State */
   CSP_PIO_SET_CODR(pio, pioMask_u32);
}

/**
********************************************************************************
* @description	Set the PIO to high level 
*
* @return	None		
*
* @param    pio				[in] Pointer to PIO structure.
*			pioMask_u32		[in] Configure which pins are set to low level.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PIOSet(CSP_PIO_T *const pio, U32_T pioMask_u32)
{
   /* Set PIO State */
   CSP_PIO_SET_SODR(pio, pioMask_u32);
}

/**
********************************************************************************
* @description	Get Pio Block MASK 
*
* @return	Pio Block Mask (null if  not found).	
*
* @param    pio				[in] Pointer to PIO structure.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/

CSP_REGISTER_T CSP_PIOGetBlockMask(CSP_PIO_T *const pio)
{
    U8_T Pio_Block_number_u8 = 0;
    CSP_REGISTER_T Pio_Block_Mask_u32 = 0;
    
    while (PIO_BLOCK_MASK[Pio_Block_number_u8].Pio_Block_Base_Address != ((CSP_PIO_PTR)UNUSED_ADDRESS))
    {
        if (PIO_BLOCK_MASK[Pio_Block_number_u8].Pio_Block_Base_Address == pio)
        {
            Pio_Block_Mask_u32 = PIO_BLOCK_MASK[Pio_Block_number_u8].Pio_Block_Mask;
        }    
        
        Pio_Block_number_u8++;    
    }  
    return Pio_Block_Mask_u32;
}

/**
********************************************************************************
* @description	Get Pio Block NUMBER
*
* @return	Pio Block Number (UNUSED if  not found).	
*
* @param    pio				[in] Pointer to PIO structure.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U8_T CSP_PIOGetBlockNumber(CSP_PIO_T *const pio)
{
    U8_T Pio_Block_number_u8 = 0;
    U8_T Pio_Block_number_found_u8 = UNUSED;
    
    while (PIO_BLOCK_MASK[Pio_Block_number_u8].Pio_Block_Base_Address != ((CSP_PIO_PTR)UNUSED_ADDRESS))
    {
        if (PIO_BLOCK_MASK[Pio_Block_number_u8].Pio_Block_Base_Address == pio)
        {
            Pio_Block_number_found_u8 = Pio_Block_number_u8;
        }    
        
        Pio_Block_number_u8++;    
    }  
    
    return Pio_Block_number_found_u8;
}
