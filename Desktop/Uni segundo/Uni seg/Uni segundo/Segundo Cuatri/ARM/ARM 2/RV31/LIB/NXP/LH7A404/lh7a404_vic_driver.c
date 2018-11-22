/***********************************************************************
 * $Id:: lh7a404_vic_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 vectored interrupt driver
 *
 * Description:
 *     This file contains driver support for the LH7A404 vectored
 *     interrupt driver.
 *
 * Notes:
 *     This driver requires that the CP15 MMU driver is correctly
 *     working.
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
***********************************************************************/

#include "lpc_arm_arch.h"
#include "lpc_arm922t_cp15_driver.h"
#include "lh7a404_vic_driver.h"

#ifdef __ICCARM__ 
#include "inarm.h"
#endif

/***********************************************************************
 * Interrupt driver package data
***********************************************************************/

/* External vector jump addresses - setting one of these addresses with
   a new address of a function will cause the new function to be called
   when the interrupt or exception occurs */
extern UNS_32 lh7a404_reset_vector;
extern UNS_32 vec_reset_handler;
extern UNS_32 vec_undefined_handler;
extern UNS_32 vec_swi_handler;
extern UNS_32 vec_prefetch_handler;
extern UNS_32 vec_abort_handler;
extern UNS_32 vec_irq_handler;
extern UNS_32 vec_fiq_handler;

/* Interrupt vectors for VIC */
static PFV vic_func_ptrs[VIC_END_OF_INTERRUPTS];

/* Pointer to logical interrupt vector area (writable) */
UNS_32 *vecarea;

/***********************************************************************
 * Vectored Interrupt driver private functions
***********************************************************************/

#ifdef __ghs__
/* Inline assembly function to return the MMU control register (GHS) */
asm UNS_32 get_mmu_control(void)
{
    MRC ARM922T_MMU_CP, 0, r0, ARM922T_MMU_REG_CONTROL, c0, 0
}
#endif

/***********************************************************************
 *
 * Function: vic_int_write_table
 *
 * Purpose: Writes the vector table and jump addresses to vector area
 *
 * Processing:
 *     Copy the shadowed image of the interrupt and exception vector
 *     table from memory to the vector jump area (usually at location
 *     0x00000000). Force out any cached values to external memory.
 * 
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Ideally, we would check the state of the V bit in the CP15
 *     coprocessor register 1 to determine the address of the where
 *     the vector area is located. If that bit was set, the vectors
 *     would be located at address 0xFFFF0000 instead of 0x00000000.
 *     This function assumes that the vector area is at 0x00000000.
 *
 **********************************************************************/
void vic_int_write_table(void)
{
    UNS_32 *dst, *dstsave, *src;
    INT_32 vecsize;
    UNS_32 high_vector;

    /* If vector address is automatic address, compute address */
    dst = vecarea;
    if ((UNS_32) dst == 0xFFFFFFFF)
    {
        /* Assume that vector table is located at low vector
           (0x00000000) address */
        dst = (UNS_32 *) ARM_RESET_VEC;

        /* Check status of high vector bit in MMU control register and
           set destination address of vector table to high vector
           address if bit is set */
#ifdef __ICCARM__
        high_vector = __MRC(15, 0, 1, 0, 0);
#endif

#ifdef __GNUC__
        asm ("MRC p15, 0, %0, c1, c0, 0" : : "r" (high_vector));
#endif

#ifdef __ghs__
        high_vector = get_mmu_control();
#endif

#ifdef __arm
        __asm
        {
            MRC ARM922T_MMU_CP, 0, high_vector, \
                ARM922T_MMU_REG_CONTROL, c0, 0
        }
#endif

        /* If high bit is set, use high vector addresses instead */
        if ((high_vector & ARM922T_MMU_CONTROL_V) != 0)
        {
            dst = (UNS_32 *) 0xFFFF0000;
        }
    }

    /* Copy vector block to interrupt vector area */
    dstsave = dst;
    for (src = (UNS_32 *) &lh7a404_reset_vector;
        src <= (UNS_32 *) &vec_fiq_handler; src++)
    {
        *dst = *src;
        dst++;
    }

    /* Write out cached vector table to memory */
    vecsize = ((INT_32) &vec_fiq_handler -
        (INT_32) &lh7a404_reset_vector) / 4;
    cp15_force_cache_coherence(dstsave, (dstsave + vecsize));
}

/***********************************************************************
 *
 * Function: vic_check_and_install
 *
 * Purpose: Checks, updates, and installs a new vector
 *
 * Processing:
 *     If the interrupt source is in the range of the first VIC, then
 *     use the VIC1 base registers as the vic base pointer, else use
 *     the VIC2 base registers and adjust the source by 32 to align with
 *     the bits in VIC2. LLoop throuogh all the interrupt vector
 *     control words and verify that the source is not used in any
 *     of the enabled vector control words. If it is used, just
 *     update the vector control word with the new address that it
 *     is already used with, enable the vector, and return TRUE to the
 *     caller. If none of the vector control words are used with that
 *     source, then loop through the control words and find the first
 *     available unused control word. If one isn't available, return
 *     FALSE to the caller. Otherwise, update the vector control word
 *     with the new address, enable the vector, and return TRUE to the
 *     caller
 * 
 * Parameters:
 *     source   : Interrupt source of type VIC_INT_SOURCE_T
 *     itype    : Must be VIC_IRQ, VIC_FIQ, or VIC_VECTORED
 *     func_ptr : Pointer to a void function
 *
 * Outputs: None
 *
 * Returns: TRUE if the vector was installed, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 vic_check_and_install(VIC_INT_SOURCE_T source,
                              VIC_INT_T itype,
                              PFV func_ptr)
{
    VIC_REGS_T *vicp;
    INT_32 idx;
    BOOL_32 vicinst;

    if ((source >= VIC_FIRST_VIC1_INTERRUPT) &&
        (source <= VIC_LAST_VIC1_INTERRUPT))
    {
        /* Use VIC1 */
        vicp = VIC1;
    }
    else
    {
        /* Use VIC2 */
        vicp = VIC2;

        /* Adjust source value to get bits for VIC2 */
        source = source - VIC_FIRST_VIC2_INTERRUPT;
    }

    /* VIC vectored interrupt - first, make sure the requested
       interrupt is not already installed */
    vicinst = FALSE;
    idx = 0;
    while ((idx < 16) && (vicinst == FALSE))
    {
        /* Does this vector use this source? */
        if ((vicp->vec_cntl[idx] & VIC_VEC_SEL_MSK) == (UNS_32) source)
        {
            /* It does, so just re-install the new vectored jump address
               in this vector and make sure the vector is enabled */
            vicp->vecaddrs[idx] = func_ptr;
            vicp->vec_cntl[idx] |= VIC_VEC_EN;

            /* The vector is installed */
            vicinst = TRUE;
        }
        else
        {
            /* Check next vector */
            idx++;
        }
    }

    /* If the vector wasn't installed, install it in the first
       available slot */
    if (vicinst == FALSE)
    {
        idx = 0;
        while ((idx < 16) && (vicinst == FALSE))
        {
            if ((vicp->vec_cntl[idx] & VIC_VEC_EN) == 0)
            {
                /* Free slot, use it */
                vicp->vecaddrs[idx] = func_ptr;
                vicp->vec_cntl[idx] = ((UNS_32) source | VIC_VEC_EN);

                /* The vector is installed */
                vicinst = TRUE;
            }
            else
            {
                /* Check next vector */
                idx++;
            }
        }
    }

    return vicinst;
}

/***********************************************************************
 * Vectored Interrupt driver public functions
***********************************************************************/

/***********************************************************************
 *
 * Function: vic_initialize
 *
 * Purpose: Initialize the vectored interrupt controller
 *
 * Processing:
 *     Prior to any processing, make sure the VIC protection is
 *     disabled by clearing the protection bit in the VIC protection
 *     enable register. Loop through all of the VIC1 and VIC2
 *     interrupt sources and set the default interrupt vector for
 *     that source to 0x00000000, disable the interrupt, and clear
 *     and pending software interrupts. For VICs 1 and 2, set all
 *     the interrupt types to IRQ, clear any pending vectored
 *     interrupts, set the non-vectored interrupt addresses to the
 *     vic1_irq_dispatcher function (for VIC1) and the
 *     vic2_irq_dispatcher (for VIC2). For VIC1 and VIC2, disable
 *     each vectored interrupt and set the default vectored interrupt
 *     address to the default IRQ handler (vec_irq_handler) for safety.
 *     Before exiting, write the ARM vector table and jump addresses
 *     to the ARM vector area with a call to vic_int_write_table.
 * 
 * Parameters:
 *     vectbladdr: Pointer to interrupt vector area, or 0xFFFFFFFF to
 *                 have driver determine address
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void vic_initialize(UNS_32 vectbladdr)
{
    VIC_INT_SOURCE_T source;
    INT_32 idx;

    /* Disable VIC protection mode */
    VIC1->prot_en &= ~VIC_PROT_EN;

    /* For VIC1, set all interrupts as IRQ type, disabled, and
       cleared */
    for (source = VIC_FIRST_VIC1_INTERRUPT;
        source <= VIC_LAST_VIC1_INTERRUPT; source++)
    {
        /* Set all jump addresses to 0x00000000 */
        vic_func_ptrs[source] = (PFV) 0x00000000;

        /* Set interrupt to disabled */
        VIC1->intenclr = VIC_INT_SELECT((UNS_32)(source));

        /* Clear any pending software interrupts */
        VIC1->swint_clr = VIC_INT_SELECT((UNS_32)(source));
    }

    /* For VIC2, set all interrupts as IRQ type, disabled, and
       cleared */
    for (source = VIC_FIRST_VIC2_INTERRUPT;
        source <= VIC_LAST_VIC2_INTERRUPT; source++)
    {
        /* Set all jump addresses to 0x00000000 */
        vic_func_ptrs[source] = (PFV) 0x00000000;

        /* Set interrupt to disabled */
        VIC2->intenclr = VIC_INT_SELECT((UNS_32)(source -
            VIC_FIRST_VIC2_INTERRUPT));

        /* Clear any pending software interrupts */
        VIC2->swint_clr = VIC_INT_SELECT((UNS_32)(source -
            VIC_FIRST_VIC2_INTERRUPT));
    }

    /* All interrupt types are IRQ type */
    VIC1->intsel = 0x00000000;
    VIC2->intsel = 0x00000000;

    /* Clear any pending vectored interrupts */
    VIC1->vecaddr = 0x00000000;
    VIC2->vecaddr = 0x00000000;

    /* Set the default address for IRQs as the default handler */
    VIC1->nv_vecaddr = (PFV) &vic1_irq_dispatcher;
    VIC2->nv_vecaddr = (PFV) &vic2_irq_dispatcher;

    for (idx = 0; idx < 16; idx++)
    {
        /* Disable all vectored interrupts */
        VIC1->vec_cntl[idx] = 0x00000000;
        VIC2->vec_cntl[idx] = 0x00000000;

        /* Make all the default vectored handlers the default IRQ
           handler (safety) until they are correctly initialized */
        VIC1->vecaddrs[idx] = (PFV) &vec_irq_handler;
        VIC2->vecaddrs[idx] = (PFV) &vec_irq_handler;
    }

    /* Save user passed vector area pointer */
    vecarea = (UNS_32 *) vectbladdr;
    
    /* Put interrupt and exception code at ARM vector area */
    vic_int_write_table();
}

/***********************************************************************
 *
 * Function: vic_install_arm_handler
 *
 * Purpose: Install an new ARM interrupt or exception handler
 *
 * Processing:
 *     If the passed fiq_handler_ptr pointer is not 0x00000000, then
 *     set the handler jump address for the specific interrupt or
 *     exception to handler_ptr.  Recopy the vector table and vector
 *     branch instructions to the interrupt and exception area with a
 *     call to int_write_table.
 * 
 * Parameters:
 *     handler_id  : Must be an enumeration of type VECTOR_T
 *     handler_ptr : Pointer to new interrupt or exception handler
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     For handler_id values of VIC1_IRQ_VEC and VIC2_IRQ_VEC, the
 *     handler addresses will be updated in the VIC1 and VIC2
 *     non-vectored address registers. These are used by the main ARM
 *     IRQ handler to determine the default IRQ jump addresses for
 *     VIC1 and VIC2.
 *
 **********************************************************************/
void vic_install_arm_handler(VECTOR_T handler_id,
                             PFV handler_ptr)
{
    /* Update address only if it is not NULL */
    if (handler_ptr != (PFV) 0x00000000)
    {
        switch (handler_id)
        {
            case RESET_VEC:
                vec_reset_handler = (UNS_32) handler_ptr;
                cp15_force_cache_coherence(
                    (UNS_32 *) &vec_reset_handler,
                    (UNS_32 *) &vec_reset_handler);
                break;

            case UNDEFINED_INST_VEC:
                vec_undefined_handler = (UNS_32) handler_ptr;
                cp15_force_cache_coherence(
                    (UNS_32 *) &vec_undefined_handler,
                    (UNS_32 *) &vec_undefined_handler);
                break;

            case SWI_VEC:
                vec_swi_handler = (UNS_32) handler_ptr;
                cp15_force_cache_coherence(
                    (UNS_32 *) &vec_swi_handler,
                    (UNS_32 *) &vec_swi_handler);
                break;

            case PREFETCH_ABORT_VEC:
                vec_prefetch_handler = (UNS_32) handler_ptr;
                cp15_force_cache_coherence(
                    (UNS_32 *) &vec_prefetch_handler,
                    (UNS_32 *) &vec_prefetch_handler);
                break;

            case DATA_ABORT_VEC:
                vec_abort_handler = (UNS_32) handler_ptr;
                cp15_force_cache_coherence(
                    (UNS_32 *) &vec_abort_handler,
                    (UNS_32 *) &vec_abort_handler);
                break;
    
            case IRQ_VEC:
                vec_irq_handler = (UNS_32) handler_ptr;
                cp15_force_cache_coherence(
                    (UNS_32 *) &vec_irq_handler,
                    (UNS_32 *) &vec_irq_handler);
                break;

            case FIQ_VEC:
                vec_fiq_handler = (UNS_32) handler_ptr;
                cp15_force_cache_coherence(
                    (UNS_32 *) &vec_fiq_handler,
                    (UNS_32 *) &vec_fiq_handler);
                break;

            case VIC1_IRQ_VEC:
                /* This is the vector that gets called from the first
                   vectored interrupt controller when not configured as
                   and FIQ or vectored interrupt */
                VIC1->nv_vecaddr = handler_ptr;
                break;

            case VIC2_IRQ_VEC:
                /* This is the vector that gets called from the second
                   vectored interrupt controller when not configured as
                   and FIQ or vectored interrupt */
                VIC2->nv_vecaddr = handler_ptr;
                break;

            default:
                break;
        }

        /* Update table in vector area */
        vic_int_write_table();
    }
}

/***********************************************************************
 *
 * Function: vic_install_handler
 *
 * Purpose: Install a function for a specific interrupt
 *
 * Processing:
 *     Save the function address for the interrupt source in the
 *     interrupt function jump table (used for IRQ interrupts only).
 *     If the interrupt type is VIC_IRQ, then clear the interrupt
 *     select bit for the interrupt source in the VIC1 or VIC2 select
 *     register to make it an IRQ interrupt type and return TRUE to the
 *     caller. If the interrupt type is VIC_FIQ, then set the interrupt
 *     select bit for the interrupt source in the VIC1 or VIC2 select
 *     register to make it an FIQ interrupt type and return TRUE to
 *     the caller. If the interrupt type is VIC_VECTORED, then call
 *     the vic_check_and_install function to check and install the
 *     vectored interrupt, using the return status from the function
 *     as the return status to the caller. If any interrupt type is
 *     passed, it is invalid and FALSE is returned to the caller.
 * 
 * Parameters:
 *     source   : Interrupt source of type VIC_INT_SOURCE_T
 *     itype    : Must be VIC_IRQ, VIC_FIQ, or VIC_VECTORED
 *     func_ptr : Pointer to a void function
 *
 * Outputs: None
 *
 * Returns: TRUE if the handler installation was good, otherwise FALSE
 *
 * Notes:
 *     This function will not install FIQ handlers. This interrupt
 *     driver design only allows 1 FIQ interrupt. Use the
 *     vic_install_arm_handler to install the FIQ interrupt, but use
 *     this function to change the interrupt to an FIQ type.
 *
 **********************************************************************/
BOOL_32 vic_install_handler(VIC_INT_SOURCE_T source,
                            VIC_INT_T itype,
                            PFV func_ptr)
{
    BOOL_32 installed = TRUE;

    /* Save a copy of the function pointer */
    vic_func_ptrs[source] = func_ptr;

    if (itype == VIC_IRQ)
    {
        if ((source >= VIC_FIRST_VIC1_INTERRUPT) &&
            (source <= VIC_LAST_VIC1_INTERRUPT))
        {
            /* Make the interrupt an IRQ type */
            VIC1->intsel &= ~VIC_INT_SELECT((UNS_32) source);
        }
        else
        {
            /* Make the interrupt an IRQ type */
            VIC2->intsel &= ~VIC_INT_SELECT((UNS_32) source -
                VIC_FIRST_VIC2_INTERRUPT);
        }
    }
    else if (itype == VIC_FIQ)
    {
        if ((source >= VIC_FIRST_VIC1_INTERRUPT) &&
            (source <= VIC_LAST_VIC1_INTERRUPT))
        {
            /* Make the interrupt an FIQ type */
            VIC1->intsel |= VIC_INT_SELECT((UNS_32) source);
        }
        else
        {
            /* Make the interrupt an FIQ type */
            VIC2->intsel |= VIC_INT_SELECT((UNS_32) source -
                VIC_FIRST_VIC2_INTERRUPT);
        }
    }
    else if (itype == VIC_VECTORED)
    {
        installed = vic_check_and_install(source, itype, func_ptr);
    }
    else
    {
        /* Invalid interrupt type */
        installed = FALSE;
    }

    return installed;
}

/***********************************************************************
 *
 * Function: vic_int_enable
 *
 * Purpose: Enable an interrupt
 *
 * Processing:
 *     If the interrupt source is in the range of the first VIC, then
 *     check the value of enable. If enable is TRUE, then enable the
 *     selected interrupt source by setting the appropriate enable bit
 *     in the VIC1 interrupt enable register. If enable is FALSE,
 *     disable the selected interrupt source by setting the appropriate
 *     disable bit in the VIC1 interrupt disable register. Otherwise
 *     (if the interrupt source is not in the range of the first VIC),
 *     perform the same logic to enable or disable the interrupt for
 *     the second VIC.
 * 
 * Parameters:
 *     source : Interrupt source of type VIC_INT_SOURCE_T
 *     enable : Must be TRUE to enable an interrupt or FALSE
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void vic_int_enable(VIC_INT_SOURCE_T source,
                    BOOL_32 enable)
{
    if ((source >= VIC_FIRST_VIC1_INTERRUPT) &&
        (source <= VIC_LAST_VIC1_INTERRUPT))
    {
        /* VIC1 interrupt */
        if (enable == TRUE)
        {
            /* Enable the interrupt */
            VIC1->inten = VIC_INT_SELECT((UNS_32)(source));
        }
        else
        {
            /* Disable the interrupt */
            VIC1->intenclr = VIC_INT_SELECT((UNS_32)(source));
        }
    }
    else
    {
        /* VIC2 interrupt */
        if (enable == TRUE)
        {
            /* Enable the interrupt */
            VIC2->inten = VIC_INT_SELECT((UNS_32)(source -
                VIC_FIRST_VIC2_INTERRUPT));
        }
        else
        {
            /* Disable the interrupt */
            VIC2->intenclr = VIC_INT_SELECT((UNS_32)(source -
                VIC_FIRST_VIC2_INTERRUPT));
        }
    }
}

/***********************************************************************
 *
 * Function: vic_int_pending
 *
 * Purpose: Check to see if an interrupt is pending
 *
 * Processing:
 *     If the interrupt source is in the range of the first VIC, then
 *     get the pending interrupt status from the IRQ status register
 *     for VIC1 and mask it with the inverted value of the VIC1
 *     interrupt select register. Get the pending interrupt status from
 *     the FIQ status register for VIC1 and mask it with the value of
 *     the VIC1 interrupt select register. 'OR' the two masked values
 *     together to get a pending interrupt bitfield of enabled
 *     interrupts. Mask the 'OR'ed value with the bit fromt the
 *     interrupt source. If the resulting value is not zero, then
 *     return TRUE to the caller. Otherwise, return FALSE.
 *
 *     If the interrupt source is in the range of the VIC2, then
 *     get the pending interrupt status from the IRQ status register
 *     for VIC2 and mask it with the inverted value of the VIC2
 *     interrupt select register. Get the pending interrupt status from
 *     the FIQ status register for VIC2 and mask it with the value of
 *     the VIC2 interrupt select register. 'OR' the two masked values
 *     together to get a pending interrupt bitfield of enabled
 *     interrupts. Mask the 'OR'ed value with the bit fromt the
 *     interrupt source. If the resulting value is not zero, then
 *     return TRUE to the caller. Otherwise, return FALSE.
 * 
 * Parameters:
 *     source : Interrupt source of type VIC_INT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Returns TRUE if the interrupt is pending, otherwise FALSE.
 *
 * Notes:
 *     This will only report the pending status of IRQ or FIQ
 *     configured interrupts.
 *
 **********************************************************************/
BOOL_32 vic_int_pending(VIC_INT_SOURCE_T source)
{
    UNS_32 status_irq, status_fiq, pstatus;
    BOOL_32 pending = FALSE;

    if ((source >= VIC_FIRST_VIC1_INTERRUPT) &&
        (source <= VIC_LAST_VIC1_INTERRUPT))
    {
        status_irq = VIC1->irqstatus & ~(VIC1->intsel);
        status_fiq = VIC1->fiqstatus & (VIC1->intsel);
        pstatus = (status_fiq | status_irq) &
            VIC_INT_SELECT((UNS_32)(source));
    }
    else
    {
        status_irq = VIC2->irqstatus & ~(VIC2->intsel);
        status_fiq = VIC2->fiqstatus & (VIC2->intsel);
        pstatus = (status_fiq | status_irq) &
            VIC_INT_SELECT((UNS_32)(source - VIC_FIRST_VIC2_INTERRUPT));
    }

    /* Was the unmasked IRQ or FIQ interrupt pending? */
    if (pstatus != 0)
    {
        /* Yes, it is pending */
        pending = TRUE;
    }

    return pending;
}

/***********************************************************************
 *
 * Function: vic_int_enabled
 *
 * Purpose: Check to see if an interrupt is enabled
 *
 * Processing:
 *     If the interrupt source is in the range of the first VIC, then
 *     check the VIC1 interrupt enabled register masked by the enable
 *     bit for the passed interrupt source. If the bit is set, then
 *     return TRUE to the caller. Otherwise, check the VIC2 interrupt
 *     enabled register masked by the enable bit for the passed
 *     interrupt source, adjusted for the 2nd VIC bit sift. If the bit
 *     is set, then return TRUE to the caller. Otherwise, return FALSE
 *     to the caller.
 * 
 * Parameters:
 *     source : Interrupt source of type VIC_INT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Returns TRUE if the interrupt is enabled, otherwise FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 vic_int_enabled(VIC_INT_SOURCE_T source)
{
    UNS_32 intreg;
    BOOL_32 enabled = FALSE;

    if ((source >= VIC_FIRST_VIC1_INTERRUPT) &&
        (source <= VIC_LAST_VIC1_INTERRUPT))
    {
        intreg = VIC1->inten & VIC_INT_SELECT((UNS_32)source);
    }
    else
    {
        intreg = VIC2->inten & VIC_INT_SELECT((UNS_32)source -
            VIC_FIRST_VIC2_INTERRUPT);
    }

    /* Was the interrupt enabled? */
    if (intreg != 0)
    {
        /* Yes, it is enabled */
        enabled = TRUE;
    }

    return enabled;
}

/***********************************************************************
 *
 * Function: vic_clear_int
 *
 * Purpose: Clears the vectored interrupt (called after a vectored
 *          interrupt)
 *
 * Processing:
 *     If the interrupt source is in the range of the first VIC, then
 *     clear the interrupt from the first VIC. Otherwise, clear the
 *     interrupt from the second VIC. The interrupt is cleared by
 *     reading the vectored address register.
 * 
 * Parameters:
 *     source : Interrupt source of type VIC_INT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void vic_clear_int(VIC_INT_SOURCE_T source)
{
    if ((source >= VIC_FIRST_VIC1_INTERRUPT) &&
        (source <= VIC_LAST_VIC1_INTERRUPT))
    {
        VIC1->vecaddr = 0x00000000;
    }
    else
    {
        VIC2->vecaddr = 0x00000000;
    }
}

/***********************************************************************
*
* Function: vic_arm_irq_dispatcher
*
* Purpose: Default ARM core IRQ interrupt dispatcher
*
* Processing:
*     This interrupt dispatch function is called when an IRQ interrupt
*     is generated at the ARM core. The VIC vectored address register
*     is read for both VICs. If they are the same, then the interrupt
*     source was VIC2 and the handler address (also the VIC2 vectored
*     address register) is used as the function address and called.
*     The VIC(2) is then cleared and the interrupt is exited. If the
*     addresses are not the same, then the interrupt source was VIC1
*     and the handler address (also the VIC1 vectored address register)
*     is used as the function address and called. The VIC(1) is then
*     cleared and the interrupt is exited.
* 
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes: None
*
***********************************************************************/
#ifndef __GNUC__
#ifndef __ICCARM__

#ifdef __ghs__
__interrupt void vic_arm_irq_dispatcher(void)
#endif

#ifdef __arm
__irq void vic_arm_irq_dispatcher(void)
#endif
{
    PFV vic_vecaddr;

    /* Check VIC1 IRQ status - if it is 0, then there are no VIC1
       interrupts pending. That means, go service VIC2 interrupts
       instead. */
    if (VIC1->irqstatus != 0)
    {
        /* Use VIC1 for the jump address */
        vic_vecaddr = (PFV) VIC1->nv_vecaddr;
    }
    else
    {
        /* Use VIC2 for the jump address */
        vic_vecaddr = (PFV) VIC2->nv_vecaddr;
    }

    /* Jump to interrupt handler */
    vic_vecaddr();
}
#endif
#endif

/***********************************************************************
*
* Function: vic1_irq_dispatcher
*
* Purpose: Default IRQ interrupt dispatcher for VIC1
*
* Processing:
*     This function is called by the vic_arm_irq_dispatcher function
*     when a vectored interrupt is configured as a standard IRQ type
*     interrupt and the interrupt source is VIC1. All interrupts for
*     this VIC are checked against the interrupt status and verified
*     that it is not masked. If it is pending and the interrupt
*     function is not 0x00000000, then the interrupt function is
*     called.
* 
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes: This dispatcher does not support nesting or prioritization
*
***********************************************************************/
void vic1_irq_dispatcher(void)
{
    UNS_32 selints;
    VIC_INT_SOURCE_T source = VIC_FIRST_VIC1_INTERRUPT;

    /* Get mask for valid IRQ interrupts */
    selints = ~(VIC1->intsel);

    /* Loop through all interrupts looking for the first active
       interrupt */
    while (source <= VIC_LAST_VIC1_INTERRUPT)
    {
        if ((VIC1->irqstatus & selints & VIC_INT_SELECT(source)) != 0)
        {
            if (vic_func_ptrs[source] != (PFV) 0x00000000)
            {
                /* Jump to interrupt handler */
                vic_func_ptrs[source]();
            }

            /* Force loop exit */
            source += VIC_LAST_VIC1_INTERRUPT;
        }
        else
        {
            source++;
        }
    }
}

/***********************************************************************
*
* Function: vic2_irq_dispatcher
*
* Purpose: Default IRQ interrupt dispatcher for VIC2
*
* Processing:
*     This function is called by the vic_arm_irq_dispatcher function
*     when a vectored interrupt is configured as a standard IRQ type
*     interrupt and the interrupt source is VIC2. All interrupts for
*     this VIC are checked against the interrupt status and verified
*     that it is not masked. If it is pending and the interrupt
*     function is not 0x00000000, then the interrupt function is
*     called.
* 
* Parameters: None
*
* Outputs: None
*
* Returns: Nothing
*
* Notes: This dispatcher does not support nesting or prioritization
*
***********************************************************************/
void vic2_irq_dispatcher(void)
{
    UNS_32 selints;
    VIC_INT_SOURCE_T source = VIC_FIRST_VIC2_INTERRUPT;

    /* Get mask for valid IRQ interrupts */
    selints = ~(VIC2->intsel);

    /* Loop through all interrupts looking for the first active
       interrupt */
    while (source <= VIC_LAST_VIC2_INTERRUPT)
    {
        if ((VIC2->irqstatus & selints &
            VIC_INT_SELECT(source - VIC_FIRST_VIC2_INTERRUPT)) != 0)
        {
            if (vic_func_ptrs[source] != (PFV) 0x00000000)
            {
                /* Jump to interrupt handler */
                vic_func_ptrs[source]();
            }

            /* Force loop exit */
            source += VIC_LAST_VIC2_INTERRUPT;
        }
        else
        {
            source++;
        }
    }
}
