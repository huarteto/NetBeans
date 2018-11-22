/******************************************************************************
 * @file:    system_MPS_CM0.c
 * @purpose: CMSIS Cortex-M0 Device Peripheral Access Layer Source File
 *           for the ARM 'Microcontroller Prototyping System' 
 * @version: V1.01
 * @date:    19. Aug. 2009
 *----------------------------------------------------------------------------
 *
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * ARM Limited (ARM) is supplying this software for use with Cortex-M0 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/


#include <stdint.h>
#include "MPS_CM0.h"


//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------


/*--------------------- Clock Configuration ----------------------------------
//
// <h> Clock Configuration
//   <i> Clock configuration can not be done from in-system, it can only be 
//   <i> done by using Gleichmann Clock Factory GUI utility!
// </h>
*/


/*--------------------- Flash Configuration ----------------------------------
//
// <h> Flash Configuration
//   <i> Flash Controller has no configuration options as all timing are 
//   <i> hardcoded at syntesis!  
// </h>
*/


/*--------------------- Memory Configuration ---------------------------------
//
// <e> Memory Configuration
//   <o1.1> ALIAS: Alias Flash
//   <o1.0> REMAP: Remap SSRAM
// </e>
*/
#define MEM_SETUP             1
#define MEMCFG_Val            0x00000002


//-------- <<< end of configuration section >>> ------------------------------


/*----------------------------------------------------------------------------
  Check the register settings
 *----------------------------------------------------------------------------*/
#define CHECK_RANGE(val, min, max)                ((val < min) || (val > max))
#define CHECK_RSVD(val, mask)                     (val & mask)

/* Memory Configuration ------------------------------------------------------*/
#if (CHECK_RSVD((MEMCFG_Val), ~0x00000003))
   #error "MEMCFG: Invalid values of reserved bits!"
#endif


/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/
    
/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define OSC         (50000000UL)        /* Main oscillator frequency          */


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemFrequency = OSC;     /*!< System Clock Frequency (Core Clock)  */


/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System and update the SystemFrequency variable.
 */
void SystemInit (void)
{
#if (MEM_SETUP)                         /* Memory Setup                       */
#ifdef REMAP                            /* If remap RAM to 0                  */
  CPU_SYS->MEMCFG = MEMCFG_Val | 0x01;
#else
  CPU_SYS->MEMCFG = MEMCFG_Val;
#endif
#endif
}
