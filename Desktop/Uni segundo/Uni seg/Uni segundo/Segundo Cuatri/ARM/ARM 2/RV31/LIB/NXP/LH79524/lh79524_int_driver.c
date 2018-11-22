/***********************************************************************
 * $Id:: lh79524_int_driver.c 27 2007-08-31 19:35:06Z kevinw           $
 *
 * Project: LH79524 INT driver
 *
 * Description:
 *     This file contains driver support for the INT module on the
 *     LH79524
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only  
 * which provides customers with programming information regarding the  
 * products. This software is supplied "AS IS" without any warranties.  
 * NXP Semiconductors assumes no responsibility or liability for the 
 * use of the software, conveys no license or title under any patent, 
 * copyright, or mask work right to the product. NXP Semiconductors 
 * reserves the right to make changes in the software without 
 * notification. NXP Semiconductors also make no representation or 
 * warranty that such application will be suitable for the specified 
 * use without further testing or modification. 
 *
 ***********************************************************************/

#include "lh79524_int_driver.h"

#define ALL_SOURCES         0xFFFFFFFF
#define ARM_SWI_VEC         0x08 /* ARM software interrupt vector address */
#define ARM_RESERVED_VEC    0x14 /* ARM reserved vector address */
#define ARM_IRQ_VEC         0x18 /* ARM IRQ vector address */
#define ARM_FIQ_VEC         0x1C /* ARM FIQ vector address */

#define VEC_ADR_SWI         0x20 /* store SWI int handling routine addr */
#define VEC_ADR_FIQ         0x24 /* store FIQ int handling routine addr */

#if !defined NULL
#define NULL   (void *)0
#endif

/***********************************************************************
 * INT driver private data
 **********************************************************************/

/* INT device configuration structure */
STATIC INT_CFG_T irqcfg = {0};
STATIC INT_CFG_T fiqcfg = {0};
STATIC INT_CFG_T swicfg = {0};

/***********************************************************************
 * Driver static  data
 **********************************************************************/
/* static variable for the VIC source to be executed currently */
static INT_32 irq_current_source = VIC_BAD_SOURCE;      

/***********************************************************************
 * Forward function declaration
 **********************************************************************/
static void int_enable_irq(void);
static void int_enable_fiq(void);
static void int_disable_irq(void);
static void int_disable_fiq(void);

/* Default fiq handling routine */
static void LH79524_default_fiq_handler(void);

/* Default swi handling routine */
static void LH79524_default_swi_handler(void);

/* Default unvectored interruption handling routine */
static void LH79524_default_unvectored_handler(void);

/***********************************************************************
 * Global variables
 **********************************************************************/
// fiq service handing routine pointer to default fiq handling routine
// it can be changed by the user program to setup user fiq routine
volatile UNS_32 LH79524_fiq_handler_addr = 
    (UNS_32)LH79524_default_fiq_handler;

// swi service handing routine pointer to default swi handling routine
// it can be changed by the user program to setup user swi routine
volatile UNS_32 LH79524_swi_handler_addr = 
    (UNS_32)LH79524_default_swi_handler;


/***********************************************************************
 * External function declaration
 **********************************************************************/
/* irq handler function address */
extern volatile UNS_32 LH79524_irq_handler;

/* ARM irq jump address */
extern volatile UNS_32 LH79524_irq_vec;
 
/* ARM fiq jump address */
extern volatile UNS_32 LH79524_fiq_vec;

/* ARM swi jump address */
extern volatile UNS_32 LH79524_swi_vec;

/* fiq handler function address */
extern volatile UNS_32 LH79524_fiq_handler;

/* swi handler function address */
extern volatile UNS_32 LH79524_swi_handler;

/***********************************************************************
 * INT driver private functions
 **********************************************************************/
/**********************************************************************
*
* Function: enable_IRQ, disable_IRQ...
*
* Purpose:
*  tool specific inline assembly funtion
*
* Processing:
*  enable or disable IRQ or FIQ
* 
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes: Not portable code
*
**********************************************************************/
static void int_enable_irq(void)
{
#ifdef __arm
    {
    	UNS_32 rx;

        __asm
        {
           MRS rx, CPSR
           BIC rx, rx, #0x80
           MSR CPSR_c, rx
        }
    }
#else
    __asm ("MRS r0, cpsr");
    __asm ("BIC r0, r0, #0x80");
    __asm ("MSR CPSR_c, r0");
#endif  
}


static void int_enable_fiq(void)
{
#ifdef __arm
    {
    	UNS_32 rx;

        __asm
        {
           MRS rx, CPSR
           BIC rx, rx, #0x40
           MSR CPSR_c, rx
        }
    }
#else
    __asm ("MRS r0, cpsr");
    __asm ("BIC r0, r0, #0x40");
    __asm ("MSR CPSR_c, r0");
#endif  
}

static void int_disable_irq(void)
{
#ifdef __arm
    {
    	UNS_32 rx;

        __asm
        {
            MRS rx, CPSR
            ORR rx, rx, #0x80
            MSR CPSR_c, rx
        }
    }
#else
    __asm ("MRS r0, cpsr");
    __asm ("ORR r0, r0, #0x80");
    __asm ("MSR CPSR_c, r0");
#endif  
}

static void int_disable_fiq(void)
{
#ifdef __arm
    {
    	UNS_32 rx;

        __asm
        {
            MRS rx, CPSR
            ORR rx, rx, #0x40
            MSR CPSR_c, rx
        }
    }
#else
    __asm ("MRS r0, cpsr");
    __asm ("ORR r0, r0, #0x40");
    __asm ("MSR CPSR_c, r0");
#endif  
}

/**********************************************************************
*
* Function: LH79524_default_unvectored_handler
*
* Purpose:
*  Default unvectored interruption handling routine
*
* Processing: None
* 
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes:
*
**********************************************************************/
static void LH79524_default_unvectored_handler(void)
{
}

/**********************************************************************
*
* Function: LH79524_default_fiq_handler
*
* Purpose:
*  Default FIQ interruption handling routine
*
* Processing: None
* 
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes:
*
**********************************************************************/
static void LH79524_default_fiq_handler(void)
{
}

/**********************************************************************
*
* Function: LH79524_default_swi_handler
*
* Purpose:
*  Default SWI interruption handling routine
*
* Processing: None
* 
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes:
*
**********************************************************************/
static void LH79524_default_swi_handler(void)
{
}


/***********************************************************************
 * INT driver public functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: irq_open
 *
 * Purpose: Open the IRQ controller
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE .
 *
 * Parameters:
 *     ipbase: void, pass 0 
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a IRQ structure or 0
 *
 * Notes: IRQ is disabled after open, use ioctl to enable IRQ
 *
 **********************************************************************/
INT_32 irq_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;
    INT_32 priority;

    if (irqcfg.init == FALSE)
    {
        /* Device is valid and not previously initialized */
        irqcfg.init = TRUE;
        
        /* Clear VIC registers */
        VIC->intenclear = ALL_SOURCES;
        VIC->vectoraddr = 0;
   
        for (priority = 0; 
            priority <= 15; priority++)
        {
            VIC->vectcntl[priority] = 0;
            VIC->vectaddr[priority] = 0;
        }

        VIC->softintclear = ALL_SOURCES;
        VIC->intselect = 0;
        VIC->defvectaddr = 0;

        VIC->defvectaddr = (UNS_32)LH79524_default_unvectored_handler;

        *((volatile UNS_32 *)ARM_IRQ_VEC) = LH79524_irq_vec;

        *((volatile UNS_32 *)ARM_RESERVED_VEC) = 
                          (UNS_32)&LH79524_irq_handler;       
        
        /* Return pointer to IRQ configuration structure */
        status = (INT_32) &irqcfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: fiq_open
 *
 * Purpose: Open the FIQ controller
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE .
 *
 * Parameters:
 *     ipbase: void, pass 0 
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a FIQ structure or 0
 *
 * Notes: FIQ is disabled after open, use ioctl to enable FIQ
 *
 **********************************************************************/
INT_32 fiq_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;

    if (fiqcfg.init == FALSE)
    {
        /* Device is valid and not previously initialized */
        fiqcfg.init = TRUE;

        /* Set up FIQ vector address */
        *((volatile UNS_32 *)ARM_FIQ_VEC) = LH79524_fiq_vec;

        *((volatile UNS_32 *)VEC_ADR_FIQ) = 
                          (UNS_32)&LH79524_fiq_handler;       
        
        /* Return pointer to FIQ configuration structure */
        status = (INT_32) &fiqcfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: swi_open
 *
 * Purpose: Open the SWI controller
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE .
 *
 * Parameters:
 *     ipbase: void, pass 0 
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a SWI structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 swi_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;

    if (swicfg.init == FALSE)
    {
        /* Device is valid and not previously initialized */
        swicfg.init = TRUE;

        /* Install SWI vector handler address */
        *((volatile UNS_32 *)ARM_SWI_VEC) = LH79524_swi_vec;

        *((volatile UNS_32 *)VEC_ADR_SWI) = 
                          (UNS_32)&LH79524_swi_handler;       
        
        /* Return pointer to SWI configuration structure */
        status = (INT_32) &swicfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: irq_close
 *
 * Purpose: Close the IRQ operation
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to
 *     FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to IRQ config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS irq_close(INT_32 devid)
{
    INT_CFG_T *intcfgptr = (INT_CFG_T *) devid;
    STATUS status = _ERROR;
    INT_32 priority;

    if (intcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        
        /* Clear VIC registers */
        VIC->intenclear = ALL_SOURCES;
        VIC->vectoraddr = 0;
   
        for (priority = 0; 
            priority <= 15; priority++)
        {
            VIC->vectcntl[priority] = 0;
            VIC->vectaddr[priority] = 0;
        }

        VIC->softintclear = ALL_SOURCES;
        VIC->intselect = 0;
        VIC->defvectaddr = 0;

        VIC->defvectaddr = 0x0;

        *((volatile UNS_32 *)ARM_IRQ_VEC) = 0x0;

        *((volatile UNS_32 *)ARM_RESERVED_VEC) = 0x0; 
        
        /* Disable IRQ */
        int_disable_irq();
        
        intcfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: fiq_close
 *
 * Purpose: Close the FIQ operation
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to
 *     FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to FIQ config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS fiq_close(INT_32 devid)
{
    INT_CFG_T *intcfgptr = (INT_CFG_T *) devid;
    STATUS status = _ERROR;

    if (intcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        intcfgptr->init = FALSE;
        /* Set up FIQ vector address */
        *((volatile UNS_32 *)ARM_FIQ_VEC) = (UNS_32)LH79524_default_fiq_handler;
        /* Disable FIQ */
        int_disable_fiq();
    }

    return status;
}

/***********************************************************************
 *
 * Function: swi_close
 *
 * Purpose: Close the SWI operation
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to
 *     FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to SWI config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS swi_close(INT_32 devid)
{
    INT_CFG_T *intcfgptr = (INT_CFG_T *) devid;
    STATUS status = _ERROR;

    if (intcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        intcfgptr->init = FALSE;
        /* Install SWI vector handler address */
        *((volatile UNS_32 *)ARM_SWI_VEC) = (UNS_32)LH79524_default_swi_handler;
    }

    return status;
}

/***********************************************************************
 *
 * Function: irq_ioctl
 *
 * Purpose: IRQ configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate int timer
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to IRQ config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: 
 *      VIC source -
 *      VIC_EXINT0      = 0,      external interrupt 0 
 *      VIC_EXINT1      = 1,      external interrupt 1 
 *      VIC_EXINT2      = 2,      external interrupt 2 
 *      VIC_EXINT3      = 3,      external interrupt 3 
 *      VIC_EXINT4      = 4,      external interrupt 4 
 *      VIC_EXINT5      = 5,      external interrupt 5 
 *      VIC_EXINT6      = 6,      external interrupt 6 
 *      VIC_EXINT7      = 7,      external interrupt 7 
 *      VIC_SPAREINT0   = 8,      spare interrupt 0 
 *      VIC_COMRX       = 9,      debug channel receive interrupt 
 *      VIC_COMTX       = 10,     debug channel transmit interrupt 
 *      VIC_SSPRXTO     = 11,     SSP receive timeout interrupt 
 *      VIC_CLCD        = 12,     LCD controller combined interrupt 
 *      VIC_SSPTX       = 13,     SSP transmit interrupt 
 *      VIC_SSPRX       = 14,     SSP receive interrupt 
 *      VIC_SSPROR      = 15,     SSP receive overrun interrupt 
 *      VIC_SSPINT      = 16,     SSP combined interrupt 
 *      VIC_TIMER0      = 17,     counter-timer0 nterrupt 
 *      VIC_TIMER1      = 18,     counter-timer1 interrupt 
 *      VIC_TIMER2      = 19,     counter-timer2 interrupt 
 *      VIC_TIMER3      = 20,     counter-timer3 interrupt 
 *      VIC_UARTRX0     = 21,     UART0 Receive interrupt 
 *      VIC_UARTTX0     = 22,     UART0 Transmit interrupt 
 *      VIC_UARTINT0    = 23,     UART0 combined interrupt 
 *      VIC_UARTINT1    = 24,     UART1 combined interrupt 
 *      VIC_UARTINT2    = 25,     UART2 combined interrupt 
 *      VIC_DMA         = 26,     DMA combined interrupt 
 *      VIC_SPAREINT1   = 27,     spare interrupt 1 
 *      VIC_SPAREINT2   = 28,     spare interrupt 2 
 *      VIC_SPAREINT3   = 29,     spare interrupt 3 
 *      VIC_RTCINT      = 30,     Real-time Clock interrupt 1 
 *      VIC_WDTINT      = 31,     Watchdog timer interrupt 1 
 *
 **********************************************************************/
STATUS irq_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    INT_CFG_T *intcfgptr = (INT_CFG_T *) devid;
    STATUS status = _ERROR;
    INT_32 temp;

    if (intcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        
        switch (cmd)
        {
            /* Enable global FIQ, arg = 1 enable, arg = 0 disable */
            case IRQ_GLOBAL_ENABLE:
                if (arg == 1)
                {
                    /* Enable IRQ */
                    int_enable_irq();
                }
                else if (arg == 0)
                {
                    /* Disable IRQ */
                    int_disable_irq();
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Enable specific VIC source as IRQ, arg = specific VIC source
                range from 0 - 31 as described in the notes section of the 
                comments */
            case IRQ_ENABLE_SOURCE:
                if ((arg >= 0) && (arg <= 31))
                {
                    /* configure source as irq */
                    VIC->intselect &= (~_SBF((UNS_32)arg, 1));
                    /* enable int generation for the source */
                    VIC->intenable |= _SBF((UNS_32)arg, 1);
                    irq_current_source = arg;        
                    
                    /* Assign default priority as 14 */
                    VIC->vectcntl[14] = (UNS_32)irq_current_source;    
                    /* Enable VIC control */
                    VIC->vectcntl[14] |= VIC_VECTCNTL_ENABLE; 
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Disable specific VIC, arg = specific VIC source
                range from 0 - 31 as described in the notes section of the 
                comments */
            case IRQ_DISABLE_SOURCE:
                if ((arg >= 0) && (arg <= 31))
                {
                    /* disable int generation for the source */
                    VIC->intenclear |= _SBF((UNS_32)arg, 1);
                    irq_current_source = arg;        
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Set up current operation source before 
            IRQ_SET_PRIORITY */
            case IRQ_SET_CURRENT_SOURCE:
                if ((arg >= 0) & (arg <= 31))
                {
                    irq_current_source = arg;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Set the priority of the interruption for current 
                VIC source, arg from 0 to 15, 0 is the highest
                priority, 15 is the lowest.  status 
                holds the current irq source. */
            case IRQ_SET_PRIORITY:
                if (irq_current_source == VIC_BAD_SOURCE)
                {
                    status = _ERROR;
                }
                else if ((arg >= 0) & (arg <=15))
                {
                    /* Assign priority */
                    VIC->vectcntl[arg] = (UNS_32)irq_current_source;    
                    /* Enable VIC control */
                    VIC->vectcntl[arg] |= VIC_VECTCNTL_ENABLE; 
                    status = irq_current_source;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Set up IRQ interruption handling routine, arg is the 
                address of handling routine, this func. can only be called
                after IRQ_SET_CURRENT_SOURCE and IRQ_SET_PRIORITY.
                status holds the priority of current irq source */
            case IRQ_SET_HANDLER:
                /* Set up user IRQ interruption handler */
                for (temp=0; temp<=15; temp++)
                {
                    if( VIC->vectcntl[temp] == ((UNS_32)irq_current_source | 
                        VIC_VECTCNTL_ENABLE))
                    {
                        break;
                    }
                }
                VIC->vectaddr[temp] = (UNS_32)arg;
                status = temp;

                break;
                
            /* Remove IRQ interruption handling routine, arg is the VIC
                source, return the priority of VIC source */
            case IRQ_REMOVE_HANDLER:
                if ((arg >= 0) && (arg <= 31))
                {
                    for (temp=0; temp<=15; temp++)
                    {
                        if( VIC->vectcntl[temp] == ((UNS_32)arg | 
                            VIC_VECTCNTL_ENABLE))
                        {
                            VIC->vectcntl[temp] = 0x0;  
                            VIC->vectaddr[temp] = 0x0;  
                            status = temp;
                            break;
                        }
                    }
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            case IRQ_GET_STATUS:
                switch (arg)
                {
                    /* Return holds the 32 VIC source masked IRQ status, 
                        bit set if corresponding VIC is set as IRQ and 
                        enabled */
                    case IRQ_GET_ENABLE:
                        /* return bit is 1 only when enabled and set as IRQ */
                        status = (VIC->intenable & (~VIC->intselect));
                        break;
                    /* Return holds which source is generating IRQ, bit
                        set if corresponding VIC is set as IRQ and enabled
                        and there is a int pending */
                    case IRQ_GET_PENDING:
                        /* Return pending irq */
                        status = VIC->irqstatus;
                        break;
                    /* Returns raw interruptions that is set as irq */
                    case IRQ_GET_RAW:
                        /* Return raw irq */
                        status = (VIC->rawintr & (~VIC->intselect));
                        break;
                    case IRQ_GET_HANDLER:
                        /* Return irq handler routine address for the source,
                           arg is the VIC source, IRQ_SET_CURRENT_SOURCE must 
                           be called before this */
                        if (irq_current_source == VIC_BAD_SOURCE)
                        {
                            status = _ERROR;
                        }
                        else 
                        {
                            status = _ERROR;
                            for (temp=0; temp<=15; temp++)
                            {
                                if( VIC->vectcntl[temp] == ((UNS_32)irq_current_source | 
                                    VIC_VECTCNTL_ENABLE))
                                {
                                    status = VIC->vectaddr[temp];
                                    break;
                                }
                            }
                        }
                        break;
                    default:
                        status = _ERROR;
                        break;
                }
            default:
                status = _ERROR;
                break;
        }
    }
    return status;
}   

/***********************************************************************
 *
 * Function: fiq_ioctl
 *
 * Purpose: FIQ configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate int timer
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to FIQ config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: 
 *      VIC source -
 *      VIC_EXINT0      = 0,      external interrupt 0 
 *      VIC_EXINT1      = 1,      external interrupt 1 
 *      VIC_EXINT2      = 2,      external interrupt 2 
 *      VIC_EXINT3      = 3,      external interrupt 3 
 *      VIC_EXINT4      = 4,      external interrupt 4 
 *      VIC_EXINT5      = 5,      external interrupt 5 
 *      VIC_EXINT6      = 6,      external interrupt 6 
 *      VIC_EXINT7      = 7,      external interrupt 7 
 *      VIC_SPAREINT0   = 8,      spare interrupt 0 
 *      VIC_COMRX       = 9,      debug channel receive interrupt 
 *      VIC_COMTX       = 10,     debug channel transmit interrupt 
 *      VIC_SSPRXTO     = 11,     SSP receive timeout interrupt 
 *      VIC_CLCD        = 12,     LCD controller combined interrupt 
 *      VIC_SSPTX       = 13,     SSP transmit interrupt 
 *      VIC_SSPRX       = 14,     SSP receive interrupt 
 *      VIC_SSPROR      = 15,     SSP receive overrun interrupt 
 *      VIC_SSPINT      = 16,     SSP combined interrupt 
 *      VIC_TIMER0      = 17,     counter-timer0 nterrupt 
 *      VIC_TIMER1      = 18,     counter-timer1 interrupt 
 *      VIC_TIMER2      = 19,     counter-timer2 interrupt 
 *      VIC_TIMER3      = 20,     counter-timer3 interrupt 
 *      VIC_UARTRX0     = 21,     UART0 Receive interrupt 
 *      VIC_UARTTX0     = 22,     UART0 Transmit interrupt 
 *      VIC_UARTINT0    = 23,     UART0 combined interrupt 
 *      VIC_UARTINT1    = 24,     UART1 combined interrupt 
 *      VIC_UARTINT2    = 25,     UART2 combined interrupt 
 *      VIC_DMA         = 26,     DMA combined interrupt 
 *      VIC_SPAREINT1   = 27,     spare interrupt 1 
 *      VIC_SPAREINT2   = 28,     spare interrupt 2 
 *      VIC_SPAREINT3   = 29,     spare interrupt 3 
 *      VIC_RTCINT      = 30,     Real-time Clock interrupt 1 
 *      VIC_WDTINT      = 31,     Watchdog timer interrupt 1 
 *
 *
 **********************************************************************/
STATUS fiq_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    INT_CFG_T *intcfgptr = (INT_CFG_T *) devid;
    STATUS status = _ERROR;

    if (intcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        
        switch (cmd)
        {
            /* Enable global FIQ, arg = 1 enable, arg = 0 disable */
            case FIQ_GLOBAL_ENABLE:
                if (arg == 1)
                {
                    /* Enable FIQ */
                    int_enable_fiq();
                }
                else if (arg == 0)
                {
                    /* Disable FIQ */
                    int_disable_fiq();
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Enable specific VIC source as FIQ, arg = specific VIC source
                range from 0 - 31 as described in the notes section of the 
                comments */
            case FIQ_ENABLE_SOURCE:
                if ((arg >= 0) && (arg <= 31))
                {
                    /* configure source as fiq */
                    VIC->intselect |= _SBF((UNS_32)arg, 1);
                    /* enable int generation for the source */
                    VIC->intenable |= _SBF((UNS_32)arg, 1);
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Disable specific VIC, arg = specific VIC source
                range from 0 - 31 as described in the notes section of the 
                comments */
            case FIQ_DISABLE_SOURCE:
                if ((arg >= 0) && (arg <= 31))
                {
                    /* disable int generation for the source */
                    VIC->intenclear |= _SBF((UNS_32)arg, 1);
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            /* Set up FIQ interruption handling routine, arg is the 
                address of handling routine */
            case FIQ_SET_HANDLER:
                /* Set up user FIQ interruption handler */
                LH79524_fiq_handler_addr = (UNS_32)arg;
                break;
                
            /* Remove FIQ interruption handling routine, no arg */
            case FIQ_REMOVE_HANDLER:
                /* Set up FIQ handler as default address */
                LH79524_fiq_handler_addr = 
                    (UNS_32)LH79524_default_fiq_handler;
                break;
                
            case FIQ_GET_STATUS:
                switch (arg)
                {
                    /* Return holds the 32 VIC source masked FIQ status, 
                        bit set if corresponding VIC is set as FIQ and 
                        enabled */
                    case FIQ_GET_ENABLE:
                        /* return bit is 1 only when enabled and set as FIQ */
                        status = (VIC->intenable & VIC->intselect);
                        break;
                    /* Return holds which source is generating FIQ, bit
                        set if corresponding VIC is set as FIQ and enabled
                        and there is a int pending */
                    case FIQ_GET_PENDING:
                        /* Return pending fiq */
                        status = VIC->fiqstatus;
                        break;
                    /* Returns raw interruptions that is set as fiq */
                    case FIQ_GET_RAW:
                        /* Return raw fiq */
                        status = (VIC->rawintr & VIC->intselect);
                        break;
                    case FIQ_GET_HANDLER:
                        /* Return fiq handler routine address */
                        status = *((volatile UNS_32 *)ARM_FIQ_VEC);
                        break;
                    default:
                        status = _ERROR;
                        break;
                }
            default:
                status = _ERROR;
                break;
        }
    }
    return status;
}   

/***********************************************************************
 *
 * Function: swi_ioctl
 *
 * Purpose: SWI configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate int timer
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to SWI config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: 
 *      VIC source -
 *      VIC_EXINT0      = 0,      external interrupt 0 
 *      VIC_EXINT1      = 1,      external interrupt 1 
 *      VIC_EXINT2      = 2,      external interrupt 2 
 *      VIC_EXINT3      = 3,      external interrupt 3 
 *      VIC_EXINT4      = 4,      external interrupt 4 
 *      VIC_EXINT5      = 5,      external interrupt 5 
 *      VIC_EXINT6      = 6,      external interrupt 6 
 *      VIC_EXINT7      = 7,      external interrupt 7 
 *      VIC_SPAREINT0   = 8,      spare interrupt 0 
 *      VIC_COMRX       = 9,      debug channel receive interrupt 
 *      VIC_COMTX       = 10,     debug channel transmit interrupt 
 *      VIC_SSPRXTO     = 11,     SSP receive timeout interrupt 
 *      VIC_CLCD        = 12,     LCD controller combined interrupt 
 *      VIC_SSPTX       = 13,     SSP transmit interrupt 
 *      VIC_SSPRX       = 14,     SSP receive interrupt 
 *      VIC_SSPROR      = 15,     SSP receive overrun interrupt 
 *      VIC_SSPINT      = 16,     SSP combined interrupt 
 *      VIC_TIMER0      = 17,     counter-timer0 nterrupt 
 *      VIC_TIMER1      = 18,     counter-timer1 interrupt 
 *      VIC_TIMER2      = 19,     counter-timer2 interrupt 
 *      VIC_TIMER3      = 20,     counter-timer3 interrupt 
 *      VIC_UARTRX0     = 21,     UART0 Receive interrupt 
 *      VIC_UARTTX0     = 22,     UART0 Transmit interrupt 
 *      VIC_UARTINT0    = 23,     UART0 combined interrupt 
 *      VIC_UARTINT1    = 24,     UART1 combined interrupt 
 *      VIC_UARTINT2    = 25,     UART2 combined interrupt 
 *      VIC_DMA         = 26,     DMA combined interrupt 
 *      VIC_SPAREINT1   = 27,     spare interrupt 1 
 *      VIC_SPAREINT2   = 28,     spare interrupt 2 
 *      VIC_SPAREINT3   = 29,     spare interrupt 3 
 *      VIC_RTCINT      = 30,     Real-time Clock interrupt 1 
 *      VIC_WDTINT      = 31,     Watchdog timer interrupt 1 
 *
 **********************************************************************/
STATUS swi_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    INT_CFG_T *intcfgptr = (INT_CFG_T *) devid;
    STATUS status = _ERROR;

    if (intcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        
        switch (cmd)
        {
            /* Enable specific VIC source as SWI, arg = VIC source */
            case SWI_ENABLE_SOURCE:
                VIC->softint |= _SBF((UNS_32)arg, 1);
                break;
                
            /* Disable specific VIC source as SWI, arg = VIC source */
            case SWI_DISABLE_SOURCE:
                VIC->softintclear |= _SBF((UNS_32)arg, 1);
                break;
                
            /* Set up SWI interruption handling routine, arg = handler 
                addr */
            case SWI_SET_HANDLER:
                LH79524_swi_handler_addr = (UNS_32)arg;
                break;
                
            /* Remove SWI interruption handling routine, no arg */
            case SWI_REMOVE_HANDLER:
                LH79524_swi_handler_addr = 
                    (UNS_32)LH79524_default_swi_handler;
                break;
                
            case SWI_GET_STATUS:
                switch (arg)
                {
                    /* Return holds the 32 VIC source masked SWI status, 
                        bit set if corresponding VIC is set as SWI and 
                        enabled */
                    case SWI_GET_ENABLE:
                        status = VIC->softint;
                        break;
                    /* Return holds which source is generating SWI, bit
                        set if corresponding VIC is set as SWI and enabled
                        and there is a int pending */
                    case SWI_GET_PENDING:
                        /* Return pending swi fiq */
                        status = (VIC->rawintr & VIC->softint);
                        break;
                    case SWI_GET_HANDLER:
                        /* Get the current SWI handler address */
                        status = *((volatile UNS_32 *)ARM_SWI_VEC);
                        break;
                    default:
                        status = _ERROR;
                        break;
                }
            default:
                status = _ERROR;
                break;
        }
    }
    return status;
}   

/***********************************************************************
 *
 * Function: int_read
 *
 * Purpose: INT read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to int timer config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 int_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: int_write
 *
 * Purpose: INT write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to int timer config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 int_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}
