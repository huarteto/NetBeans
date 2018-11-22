/**************************************************************************//**
 * @file     system_TMPM370.c
 * @brief    CMSIS Cortex-M3 Device Peripheral Access Layer Source File for the
 *           Toshiba 'TMPM370' Device Series 
 * @version  V1.03
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
#include "TMPM370.h"


//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------


/*--------------------- Watchdog Configuration -------------------------------
//
// <e> Watchdog Configuration
//   <o1.7>      WDTE: Watchdog Enable
//   <o1.4..7>   WDTP: Watchdog Detection Time
//                     <0=> 2^16 / fSYS
//                     <1=> 2^18 / fSYS
//                     <2=> 2^20 / fSYS
//                     <3=> 2^22 / fSYS
//                     <4=> 2^24 / fSYS
//                     <5=> 2^26 / fSYS
//   <o1.2>      I2WDT: IDLE Start
//   <o1.1>      RESCR: Watchdog Timer out Control
//                     <0=> Generates NMI interrupt
//                     <1=> Connects WDT out to reset
// </e>
*/
#define WDT_SETUP             1
#define WDTMOD_Val            0x00000000


/*--------------------- Clock Configuration ----------------------------------
//
// <e> Clock Configuration
//   <h> System Control Register (SYSCR)
//     <o1.12>     FPSEL: fperiph source clock
//                     <0=> fgear
//                     <1=> fc
//     <o1.8..10>  PRCK: Prescaler clock
//                     <0=> fperiph
//                     <1=> fperiph / 2
//                     <2=> fperiph / 4
//                     <3=> fperiph / 8
//                     <4=> fperiph / 16
//                     <5=> fperiph / 32
//     <o1.0..2>   GEAR: High-speed clock (fc) gear
//                     <0=> fc
//                     <4=> fc / 2
//                     <5=> fc / 4
//                     <6=> fc / 8
//                     <7=> fc / 16
//   </h>
//   <h> Oscillation Control Register (OSCCR)
//     <o2.19>     WUPSEL: Oscillator for WarmUp
//                     <0=> OSC2 (internal)
//                     <1=> OSC1 (external)
//     <o2.18>     HOSCON: High-speed oscillator connection
//                     <0=> Port M
//                     <1=> X1 / X2
//     <o2.17>     OSCSEL: High-speed oscillator select
//                     <0=> OSC2 (internal)
//                     <1=> OSC1 (external)
//     <o2.16>     XEN2: OSC2 (internal High-speed oscillator) enable
//     <o2.8>      XEN1: OSC1 (external High-speed oscillator) enable
//     <o2.2>      PLLON: PLL enable
//   </h>
//   <h> Standby Control Register (STBYCR)
//     <o3.16>     DRVE: Pin status in STOP mode
//                     <0=> Active
//                     <1=> Inactive
//     <o3.8>      RXEN: High-speed oscillator after releasing STOP mode enable
//     <o3.0..2>   STBY: Low power consumption mode
//                     <1=> STOP 
//                     <3=> IDLE
//   </h>
//   <h> PLL Selection Register (PLLSEL)
//     <o4.0>      PLLSEL: PLL output select
//                     <0=> fOSC
//                     <1=> fPLL
//   </h>
//   <h> System Clock Selection Register (CKSEL)
//     <o5.1>      SYSCK: System clock
//                     <0=> High-speed (fc)
//                     <1=> Reserved
//   </h>
// </e>
*/
#define CLOCK_SETUP           1
#define SYSCR_Val             0x00001300
#define OSCCR_Val             0x000F0104
#define STBYCR_Val            0x00000103
#define PLLSEL_Val            0x00000001
#define CKSEL_Val             0x00000000


//-------- <<< end of configuration section >>> ------------------------------


/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/
    
/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define XTALE       (10000000UL)       /* External high-speed oscillator freq */
#define XTALI       (10000000UL)       /* Internal high-speed oscillator freq */


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
/* Determine core clock frequency according to settings */

/* Determine used oscillator */
#if (OSCCR_Val & (1U << 17))
  #define __OSC_CLK  (XTALE)
#else
  #define __OSC_CLK  (XTALI)
#endif

/* Determine if PLL is used */
#if ((PLLSEL_Val & (1U<<0)) && (OSCCR_Val & (1U<<2))) /* If PLL is selected & enabled */
  #if   ((SYSCR_Val & 7U) == 0U)              /* Gear -> fc   */
    #define __CORE_CLK   (__OSC_CLK * 8U)
  #elif ((SYSCR_Val & 7U) == 4U)              /* Gear -> fc/2 */
    #define __CORE_CLK   (__OSC_CLK * 8U /  2U)
  #elif ((SYSCR_Val & 7U) == 5U)              /* Gear -> fc/4 */
    #define __CORE_CLK   (__OSC_CLK * 8U /  4U)
  #elif ((SYSCR_Val & 7U) == 6U)              /* Gear -> fc/8 */
    #define __CORE_CLK   (__OSC_CLK * 8U /  8U)
  #elif ((SYSCR_Val & 7U) == 7U)              /* Gear -> fc/16 */
    #define __CORE_CLK   (__OSC_CLK * 8U / 16U)
  #else                                       /* Gear -> reserved */
    #define __CORE_CLK   (0U)
  #endif
#else
  #if   ((SYSCR_Val & 7U) == 0U)              /* Gear -> fc   */
    #define __CORE_CLK   (__OSC_CLK     )
  #elif ((SYSCR_Val & 7U) == 4U)              /* Gear -> fc/2 */
    #define __CORE_CLK   (__OSC_CLK      /  2U)
  #elif ((SYSCR_Val & 7U) == 5U)              /* Gear -> fc/4 */
    #define __CORE_CLK   (__OSC_CLK      /  4U)
  #elif ((SYSCR_Val & 7U) == 6U)              /* Gear -> fc/8 */
    #define __CORE_CLK   (__OSC_CLK      /  8U)
  #elif ((SYSCR_Val & 7U) == 7U)              /* Gear -> fc/16 */
    #define __CORE_CLK   (__OSC_CLK      / 16U)
  #else                                       /* Gear -> reserved */
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
void SystemCoreClockUpdate (void)                /* Get Core Clock Frequency */
{
  uint32_t osc_clk;

  /* Determine clock frequency according to clock register values */

  /* Determine used oscillator */
  if (TSB_CG->OSCCR & (1U << 17)) {
    osc_clk = (XTALE);
  } else {
    osc_clk = (XTALI);
  }

  if ((TSB_CG->PLLSEL & (1 << 0)) &&         /* If PLL selected */
      (TSB_CG->OSCCR  & (1 << 2))  ) {       /* If PLL enabled */
    switch (TSB_CG->SYSCR & 7U) {
      case 0U:                               /* Gear -> fc */
        SystemCoreClock = osc_clk * 8U;
        break;
      case 4U:                               /* Gear -> fc/2 */
        SystemCoreClock = osc_clk * 8U /  2U;
        break;
      case 5U:                               /* Gear -> fc/4 */
        SystemCoreClock = osc_clk * 8U /  4U;
        break;
      case 6U:                               /* Gear -> fc/8 */
        SystemCoreClock = osc_clk * 8U /  8U;
        break;
      case 7U:                               /* Gear -> fc/16 */
        SystemCoreClock = osc_clk * 8U / 16U;
        break;
      default:                               /* Gear -> reserved */
        SystemCoreClock = 0U;
        break;
    }
  }
  else {
    switch (TSB_CG->SYSCR & 7U) {
      case 0U:                               /* Gear -> fc */
        SystemCoreClock = osc_clk;
        break;
      case 4U:                               /* Gear -> fc/2 */
        SystemCoreClock = osc_clk      /  2U;
        break;
      case 5U:                               /* Gear -> fc/4 */
        SystemCoreClock = osc_clk      /  4U;
        break;
      case 6U:                               /* Gear -> fc/8 */
        SystemCoreClock = osc_clk      /  8U;
        break;
      case 7U:                               /* Gear -> fc/16 */
        SystemCoreClock = osc_clk      / 16U;
        break;
      default:                               /* Gear -> reserved */
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
#if (WDT_SETUP)                                  /* Watchdog Setup */
  TSB_WDT->MOD  = WDTMOD_Val;
  if (!(WDTMOD_Val & (1 << 7))) {                /* If watchdog is to be disabled */
    TSB_WDT->CR = 0xB1;
  }
#endif

#if (CLOCK_SETUP)                                /* Clock Setup */

  #if (OSCCR_Val & (1UL << 19))                  /* determine warmup timer */
    #define __WUCNT_100   (200UL * (XTALE / 1000000UL))
  #else
    #define __WUCNT_100   (200UL * (XTALI / 1000000UL))
  #endif


  TSB_CG->SYSCR  = SYSCR_Val;
  TSB_CG->STBYCR = STBYCR_Val;

  TSB_CG->OSCCR  = (OSCCR_Val  & 0x000F0100);    /* enable  oscillator */
  TSB_CG->OSCCR =  (OSCCR_Val  & 0x000F0104) |
                                                                  /* delay ca. 200us @32kHz */
                                (((__WUCNT_100 >>0 ) & 0x003UL) << 14) |  /* WUODR0..1  */            
                                (((__WUCNT_100 >> 2) & 0xFFFUL) << 20) |  /* WUODR2..13 */            
                                (  1UL                          <<  0) ;  /* start warmup timer */             
  while ( TSB_CG->OSCCR & (1UL << 1));           /* warm up */


  TSB_CG->PLLSEL = (PLLSEL_Val & 0x00000001) |   /* set PLL */
                                ( 0x0000A13E);   /* write ... */
  TSB_CG->CKSEL  = CKSEL_Val;
#endif
}
