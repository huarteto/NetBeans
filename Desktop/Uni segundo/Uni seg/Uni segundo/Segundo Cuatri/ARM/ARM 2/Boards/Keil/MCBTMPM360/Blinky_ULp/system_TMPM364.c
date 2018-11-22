/******************************************************************************
 * @file     system_TMPM364.c
 * @brief    CMSIS Cortex-M3 Device Peripheral Access Layer Source File for the
 *           Toshiba 'TMPM364F10' Device Series 
 * @version  V1.02
 * @date     2. June 2010
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
#include <TMPM364.h>


//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------


/*--------------------- Watchdog Configuration -------------------------------
//
// <e> Watchdog Configuration
//   <o1.7>      WDTE: Watchdog Enable
//   <o1.4..6>   WDTP: Watchdog Detection Time
//                     <0=> 2^15 / fSYS
//                     <1=> 2^17 / fSYS
//                     <2=> 2^29 / fSYS
//                     <3=> 2^21 / fSYS
//                     <4=> 2^23 / fSYS
//                     <5=> 2^25 / fSYS
//                     <6=> Reserved
//                     <7=> Reserved
//   <o1.2>      I2WDT: IDLE Start
//   <o1.1>      RESCR: Watchdog Timer out Control
//                     <0=> Generates NMI interrupt
//                     <1=> Connects WDT out to reset
// </e>
*/
#define WD_SETUP        1
#define WDMOD_Val       0x00000000


/*--------------------- Clock Configuration ----------------------------------
//
// <e> Clock Configuration
//   <h> System Control Register (SYSCR)
//     <o1.23>     USBHRES: USB Host Controller reset Enable
//                 <i> Exists only on Devices with USB Host Controller
//     <o1.20>     FCSTOP: ADC Clock Disable
//     <o1.16..17> SCOSEL: SCOUT pin output
//                     <0=> fs
//                     <1=> fsys / 2
//                     <2=> fsys
//                     <3=> phiT0
//     <o1.12..13> FPSEL: fperiph Source Clock
//                     <0=> fgear
//                     <1=> fc
//                     <2=> fsys
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
//     <o2.9>      XTEN: Low-speed Oscillator Enable
//     <o2.8>      XEN: High-speed Oscillator Enable
//     <o2.3>      WUPSEL: Oscillator for Warm-up
//                     <0=> X1
//                     <1=> XT1
//     <o2.2>      PLLON: PLL Enable
//   </h>
//   <h> Standby Control Register (STBYCR)
//     <o3.17>     PTKEEP: IO Port Control in Backup Mode
//                     <0=> Controllable
//                     <1=> Hold On
//     <o3.16>     DRVE: Pin Status in STOP Mode
//                     <0=> Inactive
//                     <1=> Active
//     <o3.9>      RXTEN: Low-speed Oscillator after releasing STOP Mode Enable
//     <o3.8>      RXEN: High-speed Oscillator after releasing STOP Mode Enable
//     <o3.0..2>   STBY: Low Power Consumption Mode
//                     <1=> STOP 
//                     <2=> SLEEP
//                     <3=> IDLE2
//                     <4=> Reserved
//                     <5=> BACKUP STOP
//                     <6=> BACKUP SLEEP
//                     <7=> IDLE1
//   </h>
//   <h> PLL Selection Register (PLLSEL)
//     <o4.3..15>   PLL Multiplier
//                     <0x0E43=> fosc * 4
//                     <0x1427=> fosc * 8
//     <o4.0>      PLLSEL: Use of PLL
//                     <0=> X1
//                     <1=> PLL
//   </h>
//   <h> System Clock Selection Register (CKSEL)
//     <o5.1>      SYSCK: System Clock
//                     <0=> High-speed (fc)
//                     <1=> Low-speed (fs)
//   </h>
// </e>
*/
#define CLOCK_SETUP     1
#define SYSCR_Val       0x00010000
#define OSCCR_Val       0x09400334
#define STBYCR_Val      0x00000103
#define PLLSEL_Val      0x0000721F
#define CKSEL_Val       0x00000000


//-------- <<< end of configuration section >>> ------------------------------


/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/
    
/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define XTALH          (12000000UL)    /* External high-speed oscillator freq */
#define XTALL          (   32768UL)    /* External low-speed  oscillator freq */


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
/* Determine core clock frequency according to settings */
#if (CKSEL_Val & (1U << 1))             /* If system clock is low-speed clock */
  #define __CORE_CLK (XTALL)
#else                                   /* If system clock is high-speed clock*/
  #if ((PLLSEL_Val & (1U<<0)) && (OSCCR_Val & (1U<<2))) /* If PLL is sel & en */
    #if   ((PLLSEL_Val & 0x0000FFF8) == 0x0000A138UL)   /* If PLL set "x8"    */
      #if   ((SYSCR_Val & 7U) == 0U)              /* Gear -> fc               */
        #define __CORE_CLK   (XTALH * 8U)
      #elif ((SYSCR_Val & 7U) == 4U)              /* Gear -> fc/2             */
        #define __CORE_CLK   (XTALH * 8U / 2U)
      #elif ((SYSCR_Val & 7U) == 5U)              /* Gear -> fc/4             */
        #define __CORE_CLK   (XTALH * 8U / 4U)
      #elif ((SYSCR_Val & 7U) == 6U)              /* Gear -> fc/8             */
        #define __CORE_CLK   (XTALH * 8U / 8U)
      #else                                       /* Gear -> reserved         */
        #define __CORE_CLK   (0U)
      #endif
    #elif ((PLLSEL_Val & 0x0000FFF8) == 0x00007218UL)   /* If PLL set "x4"    */
      #if   ((SYSCR_Val & 7U) == 0U)              /* Gear -> fc               */
        #define __CORE_CLK   (XTALH * 4U)
      #elif ((SYSCR_Val & 7U) == 4U)              /* Gear -> fc/2             */
        #define __CORE_CLK   (XTALH * 4U / 2U)
      #elif ((SYSCR_Val & 7U) == 5U)              /* Gear -> fc/4             */
        #define __CORE_CLK   (XTALH * 4U / 4U)
      #elif ((SYSCR_Val & 7U) == 6U)              /* Gear -> fc/8             */
        #define __CORE_CLK   (XTALH * 4U / 8U)
      #else                                       /* Gear -> reserved         */
        #define __CORE_CLK   (0U)
      #endif
    #else
      #define __CORE_CLK   (0U)
    #endif
  #else
    #if   ((SYSCR_Val & 7U) == 0U)                /* Gear -> fc               */
      #define __CORE_CLK   (XTALH)
    #elif ((SYSCR_Val & 7U) == 4U)                /* Gear -> fc/2             */
      #define __CORE_CLK   (XTALH / 2U)
    #elif ((SYSCR_Val & 7U) == 5U)                /* Gear -> fc/4             */
      #define __CORE_CLK   (XTALH / 4U)
    #elif ((SYSCR_Val & 7U) == 6U)                /* Gear -> fc/8             */
      #define __CORE_CLK   (XTALH / 8U)
    #else                                         /* Gear -> reserved         */
      #define __CORE_CLK   (0U)
    #endif
  #endif                                          /* If PLL not used          */
#endif

/* Clock Variable definitions */
uint32_t SystemCoreClock = __CORE_CLK;/*!< System Clock Frequency (Core Clock)*/


/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Update SystemCoreClock according register values.
 */
void SystemCoreClockUpdate (void)                 /* Get Core Clock Frequency */
{
  /* Determine clock frequency according to clock register values             */
  if (TSB_CG->CKSEL & (1 << 1)) {                 /* If sys clk low-speed clk */
    SystemCoreClock = XTALL;
  } else {                                        /* If sys clk high-speed clk*/
    if ((TSB_CG->PLLSEL & (1 << 0)) &&            /* If PLL selected          */
        (TSB_CG->OSCCR  & (1 << 2))) {            /* If PLL enabled           */
      if      ((TSB_CG->PLLSEL & 0x0000FFF8) == 0x0000A138UL){ /* If PLL "x8"*/
        switch (TSB_CG->SYSCR & 7U) {
          case 0U:                                /* Gear -> fc               */
            SystemCoreClock = XTALH * 8U;
            break;
          case 4U:                                /* Gear -> fc/2             */
            SystemCoreClock = XTALH * 8U / 2U;
            break;
          case 5U:                                /* Gear -> fc/4             */
            SystemCoreClock = XTALH * 8U / 4U;
            break;
          case 6U:                                /* Gear -> fc/8             */
            SystemCoreClock = XTALH * 8U / 8U;
            break;
          default:                                /* Gear -> reserved         */
            SystemCoreClock = 0U;
            break;
        }
      }
      else if ((TSB_CG->PLLSEL & 0x0000FFF8) == 0x00007218UL){ /* If PLL "x4" */
        switch (TSB_CG->SYSCR & 7U) {
          case 0U:                                /* Gear -> fc               */
            SystemCoreClock = XTALH * 4U;
            break;
          case 4U:                                /* Gear -> fc/2             */
            SystemCoreClock = XTALH * 4U / 2U;
            break;
          case 5U:                                /* Gear -> fc/4             */
            SystemCoreClock = XTALH * 4U / 4U;
            break;
          case 6U:                                /* Gear -> fc/8             */
            SystemCoreClock = XTALH * 4U / 8U;
            break;
          default:                                /* Gear -> reserved         */
            SystemCoreClock = 0U;
            break;
        }
      }
      else {
        SystemCoreClock = 0U;
      }
    } else {                                      /* PLL not selected         */
      switch (TSB_CG->SYSCR & 7U) {
        case 0U:                                  /* Gear -> fc               */
          SystemCoreClock = XTALH;
          break;
        case 4U:                                  /* Gear -> fc/2             */
          SystemCoreClock = XTALH / 2U;
          break;
        case 5U:                                  /* Gear -> fc/4             */
          SystemCoreClock = XTALH / 4U;
          break;
        case 6U:                                  /* Gear -> fc/8             */
          SystemCoreClock = XTALH / 8U;
          break;
        default:                                  /* Gear -> reserved         */
          SystemCoreClock = 0U;
          break;
      }
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
#if (WD_SETUP)                                    /* Watchdog Setup           */
  TSB_WD->MOD    = WDMOD_Val;
  if (!(WDMOD_Val & (1 << 7))) {                  /* If watchdog disabled     */
    TSB_WD->CR   = 0xB1;
  }
#endif

#if (CLOCK_SETUP)                                 /* Clock Setup              */
  TSB_CG->SYSCR  = SYSCR_Val;
  TSB_CG->STBYCR = STBYCR_Val;

  TSB_CG->OSCCR  = (OSCCR_Val  & 0x00000308) |    /* Enable  oscillator       */
                                (    3 <<  4) ;   /* Always write "0011"      */
#if (OSCCR_Val & (1 << 2))                        /* If PLL is used           */
  TSB_CG->PLLSEL = (PLLSEL_Val & 0x0000F7FE) |    /* Set PLL multiplier       */
                                (   3 <<  1) ;    /* Write "11"               */
#endif
#if (OSCCR_Val & (1 << 3))                        /* Determine warm-up timer  */
  TSB_CG->OSCCR  = (OSCCR_Val  & 0x00000308) |
                                (    1 << 20) |   /* Delay ~ 100us @ 32kHz    */
                                (    3 <<  4) |   /* Always write "0011"      */
                                (    1 <<  0) ;   /* Start warm-up timer      */
#else
  TSB_CG->OSCCR  = (OSCCR_Val  & 0x00000308) |
                                (   75 << 20) |   /* Delay ~ 100us @ 12MHz    */
                                (    3 <<  4) |   /* Always write "0011"      */
                                (    1 <<  0) ;   /* Start warm-up timer      */
#endif
  while (TSB_CG->OSCCR & (1 << 1));               /* Wait for warm-up         */


#if (OSCCR_Val & (1 << 2))                        /* If PLL is used           */
  TSB_CG->OSCCR  = (OSCCR_Val  & 0x0000030C) |    /* Enable  Oscillator / PLL */
                                (    3 <<  4) ;   /* Always write "0011"      */
#if (OSCCR_Val & (1 << 3))                        /* Determine warm-up timer  */
  TSB_CG->OSCCR  = (OSCCR_Val  & 0x0000030C) |
                                (    2 << 20) |   /* Delay ~ 200us @ 32kHz    */
                                (    3 <<  4) |   /* Always write "0011"      */
                                (    1 <<  0) ;   /* Start warm-up timer      */
#else
  TSB_CG->OSCCR  = (OSCCR_Val  & 0x0000030C) |
                                (    3 <<  4) |   /* Always write "0011"      */
                                (  150 << 20) |   /* Delay ~ 200us @ 12MHz    */
                                (    1 <<  0) ;   /* Start warmup timer       */
#endif
  while (TSB_CG->OSCCR & (1 << 1));               /* Wait for warm-up         */

  TSB_CG->PLLSEL = (PLLSEL_Val & 0x0000F7FF) |    /* Set PLL values           */
                                (    3 <<  1) ;   /* Write "11"               */
#endif
  TSB_CG->CKSEL  = CKSEL_Val;
#endif
}
