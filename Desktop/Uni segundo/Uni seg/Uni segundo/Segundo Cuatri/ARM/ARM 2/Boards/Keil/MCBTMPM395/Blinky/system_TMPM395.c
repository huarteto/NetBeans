/**************************************************************************//**
 * @file     system_TMPM95.c
 * @brief    CMSIS Cortex-M3 Device Peripheral Access Layer Source File for the
 *           Toshiba 'TMPM395' Device Series 
 * @version  V1.01
 * @date     14. June 2010
 *
 * @note
 * Copyright (C) 2010 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Cortex-M 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/


#include <stdint.h>
#include "TMPM395.h"


//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------


/*--------------------- Watchdog Configuration -------------------------------
//
// <e> Watchdog Configuration
//   <o1.7>      WDTE: Watchdog Enable
//   <o1.4..6>   WDTP: Watchdog Detection Time
//                     <0=> 2^16 / fSYS
//                     <1=> 2^18 / fSYS
//                     <2=> 2^20 / fSYS
//                     <3=> 2^22 / fSYS
//                     <4=> 2^24 / fSYS
//                     <5=> 2^26 / fSYS
//                     <6=> Reserved
//                     <7=> Reserved
//   <o1.2>      I2WDT: IDLE Start
//   <o1.1>      RESCR: Watchdog Timer out Control
//                     <0=> Generates NMI interrupt
//                     <1=> Connects WDT out to reset
// </e>
*/
#define WDT_SETUP       1
#define WDTMOD_Val      0x00000000


/*--------------------- Clock Configuration ----------------------------------
//
// <e> Clock Configuration
//   <h> System Control Register (SYSCR)
//     <o1.16..17> SCOSEL: SCOUT pin output
//                     <0=> Reserved
//                     <1=> Reserved
//                     <2=> fsys
//                     <3=> phiT0
//     <o1.12>     FPSEL: fperiph Source Clock
//                     <0=> fgear
//                     <1=> fc
//     <o1.8..10>  PRCK: Prescaler Clock
//                     <0=> fperiph
//                     <1=> fperiph / 2
//                     <2=> fperiph / 4
//                     <3=> fperiph / 8
//                     <4=> fperiph / 16
//                     <5=> fperiph / 32
//                     <6=> Reserved
//                     <7=> Reserved
//     <o1.0..2>   GEAR: High-speed Clock (fc) Gear
//                     <0=> fc
//                     <1=> Reserved
//                     <2=> Reserved
//                     <3=> Reserved
//                     <4=> fc / 2
//                     <5=> fc / 4
//                     <6=> fc / 8
//                     <7=> Reserved
//   </h>
//   <h> Oscillation Control Register (OSCCR)
//     <o2.17>     OSCSEL: High-speed Oscillator Select
//                     <0=> Internal
//                     <1=> External
//     <o2.16>     XEN2: internal High-speed Oscillator Enable
//     <o2.8>      XEN: High-speed Oscillator Enable
//     <o2.3>      WUPSEL: Oscillator for Warm-up
//                     <0=> X1  (internal High OSC)
//                     <1=> XT1 (external Low OSC)
//   </h>
//   <h> Standby Control Register (STBYCR)
//     <o3.19>     ISOFLASH: Flash Isolation Control
//                     <0=> Inert
//                     <1=> Active
//     <o3.18>     SDFLASH: Flash Shutoff in Slow/Sleep
//                     <0=> On
//                     <1=> Shutoff
//     <o3.16>     DRVE: Pin Status in STOP Mode
//                     <0=> Inactive
//                     <1=> Active
//     <o3.8>      RXEN: High-speed Oscillator after releasing STOP Mode Enable
//     <o3.0..2>   STBY: Low Power Consumption Mode
//                     <0=> Reserved
//                     <1=> Reserved 
//                     <2=> SLEEP
//                     <3=> IDLE
//                     <4=> Reserved
//                     <5=> BACKUP STOP
//                     <6=> Reserved
//                     <7=> Reserved
//   </h>
//   <h> System Clock Selection Register (CKSEL)
//     <o4.1>      SYSCK: System Clock
//                     <0=> High-speed (fc)
//                     <1=> Low-speed (fs)
//   </h>
// </e>
*/
#define CLOCK_SETUP           1
#define SYSCR_Val             0x00021100
#define OSCCR_Val             0x000F0104
#define STBYCR_Val            0x00000103
#define CKSEL_Val             0x00000000


//-------- <<< end of configuration section >>> ------------------------------


/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/
    
/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define XTAL_EH     (20000000UL)       /* External high-speed oscillator freq */
#define XTAL_IH     (10000000UL)       /* Internal high-speed oscillator freq */
#define XTAL_EL     (   32768UL)       /* External low-speed oscillator freq  */

/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
/* Determine core clock frequency according to settings */
#if (CKSEL_Val & (1 << 1))                        /* If sys clk low-speed clk */
  #define __CORE_CLK  (XTAL_EL)
#else                                             /* If sys clk high-speed clk*/
  #if (OSCCR_Val & (1 <<17))                      /* external High_speed OSC  */
    #if (OSCCR_Val & (1 << 8))                    /* external OSC enabled     */
      #define __OSC_CLK  (XTAL_EH)
    #else
      #define __OSC_CLK  (0U)
    #endif
  #else                                           /* internal High_speed OSC  */
    #if (OSCCR_Val & (1 <<16))                    /* internal OSC enabled     */
        #define __OSC_CLK  (XTAL_IH)
    #else  
        #define __OSC_CLK  (0U)
    #endif
  #endif

  #if   ((SYSCR_Val & 7U) == 0U)                  /* Gear -> fc   */
    #define __CORE_CLK   (__OSC_CLK     )
  #elif ((SYSCR_Val & 7U) == 4U)                  /* Gear -> fc/2 */
    #define __CORE_CLK   (__OSC_CLK / 2U)
  #elif ((SYSCR_Val & 7U) == 5U)                  /* Gear -> fc/4 */
    #define __CORE_CLK   (__OSC_CLK / 4U)
  #elif ((SYSCR_Val & 7U) == 6U)                  /* Gear -> fc/8 */
    #define __CORE_CLK   (__OSC_CLK / 8U)
  #else                                           /* Gear -> reserved */
    #define __CORE_CLK   (0U)
  #endif
#endif



/* Clock Variable definitions */
uint32_t SystemCoreClock = __CORE_CLK;/*!< System Clock Frequency (Core Clock)*/


/**
 * Update SystemCoreClock variable
 *
 * @param  none
 * @return none
 *
 * @brief  Update SystemCoreClock according register values.
 */
void SystemCoreClockUpdate (void)                 /* Get Core Clock Frequency */
{

  /* Determine clock frequency according to clock register values             */
  if (TSB_CG->CKSEL & (1 << 1)) {                /* If sys clk low-speed clk */
    SystemCoreClock = XTAL_EL;
  } else {                                        /* If sys clk high-speed clk*/
    if (TSB_CG->OSCCR & (1 <<17)) {              /* external High_speed OSC  */
      if (TSB_CG->OSCCR & (1 << 8)) {            /* external OSC enabled     */
        SystemCoreClock = XTAL_EH;
      } else {
        SystemCoreClock = 0U;
      }
    } else {                                      /* internal High_speed OSC  */
      if (TSB_CG->OSCCR & (1 <<16)) {            /* internal OSC enabled     */
        SystemCoreClock = XTAL_IH;
      } else {
        SystemCoreClock = 0U;
      }
    }

    switch (TSB_CG->SYSCR & 7U) {
      case 0U:                                    /* Gear -> fc               */
        SystemCoreClock /= 1U;
        break;
      case 4U:                                    /* Gear -> fc/2             */
        SystemCoreClock /= 2U;
        break;
      case 5U:                                    /* Gear -> fc/4             */
        SystemCoreClock /= 4U;
        break;
      case 6U:                                    /* Gear -> fc/8             */
        SystemCoreClock /= 8U;
        break;
      default:                                    /* Gear -> reserved         */
        SystemCoreClock = 0U;
        break;
    }
  }

}


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
#if (WDT_SETUP)                                   /* Watchdog Setup           */
  TSB_WDT->MOD    = WDTMOD_Val;
  if (!(WDTMOD_Val & (1 << 7))) {                 /* If watchdog disabled     */
    TSB_WDT->CR   = 0xB1;
  }
#endif

#if (CLOCK_SETUP)                                 /* Clock Setup              */
  TSB_CG->SYSCR  = (SYSCR_Val  & 0x00031707);

  TSB_CG->STBYCR = (STBYCR_Val & 0x000D0107) |
                                (    1 <<  9) ;   /* Always write "1"         */

  TSB_CG->OSCCR  = (OSCCR_Val  & 0x00030108) |   /* Enable  oscillator       */
                                (    1 <<  9) ;   /* Always write "1"         */
#if (OSCCR_Val & (1 << 3))                        /* Determine warm-up timer  */
  TSB_CG->OSCCR  = (OSCCR_Val  & 0x00030108) |
                                (    1 << 20) |   /* Delay ~ 100us @ 32kHz    */
                                (    1 <<  9) |   /* Always write "1"         */
                                (    1 <<  0) ;   /* Start warm-up timer      */
#else
  TSB_CG->OSCCR  = (OSCCR_Val  & 0x00030108) |
                                (   80 << 20) |   /* Delay ~ 100us @ 10MHz    */
                                (    1 <<  9) |   /* Always write "1"         */
                                (    1 <<  0) ;   /* Start warm-up timer      */
#endif
  while (TSB_CG->OSCCR & (1 << 1));              /* Wait for warm-up         */


  TSB_CG->CKSEL  = CKSEL_Val;
#endif
}
