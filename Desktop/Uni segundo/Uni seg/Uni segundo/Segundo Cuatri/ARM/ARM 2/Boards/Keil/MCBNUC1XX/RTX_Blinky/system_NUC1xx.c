/**************************************************************************//**
 * @file     system_NUC1xx.c
 * @brief    CMSIS Cortex-M0 Device Peripheral Access Layer Source File
 *           for the Nuvoton NUC1xx Device Series
 * @version  V1.00
 * @date     21. April 2010
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
#include "NUC1xx.h"

/*
//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/

/*--------------------- Clock Configuration ----------------------------------*/
//
// <e0> Clock Configuration
//   <o1.0> XTL12M_EN: External 12MHz Crystal Oscillator Enable
//   <o1.1> XTL32K_EN: External 32.768 KHz Crystal Enable
//   <o1.2> OSC22M_EN: Internal 22MHz Oscillator Enable
//   <o1.3> OSC10K_EN: Internal 10KHz Oscillator Enable
//
//   <h> System Clock Configuration
//     <h> PLL Configuration
//                     <i> FOUT = FIN * (NF/NR) * (1/NO)
//       <o7.19>     FIN: PLL Source Clock
//                     <0=> ext. 12MHz crystal
//                     <1=> int. 22MHz osc.
//       <o7.0..8>   NF:  PLL Feedback Divider
//                     <2-513><#-2>
//       <o7.9..13>  NR:  PLL Input Divider
//                     <2-17><#-2>
//       <o7.14..15> NO: PLL Output Divider
//                     <0=> 1
//                     <1=> 2
//                     <2=> 2
//                     <3=> 4
//       <o7.16>     PD:     PLL Power Down Mode Enable
//                     <i> If set the IDLE bit “1” in PWRCON register, the PLL will enter power down mode too
//       <o7.17>     BP:     PLL Bypass Enable
//       <o7.18>     OE:     PLL FOUT Disable
//     </h>
//     <o4.0..2>  HCLK_S:  HCLK Clock Source
//                     <0=> ext. 12MHz crystal clock
//                     <1=> ext. 32KHz crystal clock
//                     <2=> PLL clock
//                     <3=> int. 10KHz osc. clock
//                     <4=> int. 22MHz osc. clock
//     <o6.0..3>  HCLK_N:  HCLK Clock Source Devider
//                     <i> HCLK = (HCLK Clock Source) / Devider
//                     <1-16><#-1>
//   </h>
//
//   <h> SysTick Clock Configuration
//                     <i> Note: CMSIS uses Core Clock as Clock Source
//     <o4.3..5>  STCLK_S: SysTick External Reference Clock Source
//                     <0=> ext. 12MHz crystal clock
//                     <1=> ext. 32KHz crystal clock
//                     <2=> 12MHz crystal clock / 2
//                     <3=> HCLK / 2
//                     <4=> int. 22MHz osc. clock / 2
//   </h>
//
//   <h> Peripheral Clock Configuration
//     <o5.0..1>   WDG_S:  Watchdog CLK Clock Source
//                       <0=> ext. crystal 1 clock
//                       <1=> ext. 32KHz crystal clock
//                       <2=> HCLK/2048 clock
//                       <3=> int. 10KHz osc. clock
//     <o3.0>      WDG_EN: Watchdog Clock Enable
//     <o3.1>      RTC_EN: Real-Time-Clock Enable
//     <o5.8..10>  TMR0_S: TIMER0 Clock Source
//                       <0=> ext. 12MHz crystal clock
//                       <1=> ext. 32KHz crystal clock
//                       <2=> HCLK
//                       <3=> ext. trigger
//                       <4=> int. 22MHz osc. clock
//     <o3.2>      TMR0_EN: Timer0 Clock Enable
//     <o5.12..14> TMR1_S: TIMER1 Clock Source
//                       <0=> ext. 12MHz crystal clock
//                       <1=> ext. 32KHz crystal clock
//                       <2=> HCLK
//                       <3=> ext. trigger
//                       <4=> int. 22MHz osc. clock
//     <o3.3>      TMR1_EN: Timer1 Clock Enable
//     <o5.16..18> TMR2_S: TIMER2 Clock Source
//                       <0=> ext. 12MHz crystal clock
//                       <1=> ext. 32KHz crystal clock
//                       <2=> HCLK
//                       <3=> ext. trigger
//                       <4=> int. 22MHz osc. clock
//     <o3.4>      TMR2_EN: Timer2 Clock Enable
//     <o5.20..22> TMR3_S: TIMER3 Clock Source
//                       <0=> ext. 12MHz crystal clock
//                       <1=> ext. 32KHz crystal clock
//                       <2=> HCLK
//                       <3=> ext. trigger
//                       <4=> int. 22MHz osc. clock
//     <o3.5>       TMR3_EN: Timer3 Clock Enable
//     <o3.8>  I2C0_EN: I2C0 Clock Enable
//     <o3.9>  I2C1_EN: I2C1 Clock Enable
//     <o3.12> SPI0_EN: SPI0 Clock Enable
//     <o3.13> SPI1_EN: SPI1 Clock Enable
//     <o3.14> SPI2_EN: SPI2 Clock Enable
//     <o3.15> SPI3_EN: SPI3 Clock Enable
//     <o5.24..25> UART_S: UART Clock Source
//                       <0=> ext. 12MHz crystal clock
//                       <1=> PLL clock
//                       <2=> int. 22MHz osc. clock
//     <o6.8..11>  UART_N:  UART Clock Source Devider
//                       <i> UART Clock = (UART Clock Source) / Devider
//                       <1-16><#-1>
//     <o3.16>     UART0_EN: UART0 Clock Enable
//     <o3.17>     UART1_EN: UART1 Clock Enable
//     <o5.28..29> PWM01_S:  PWM1 and PWM0 Clock Source
//                       <0=> ext. 12MHz crystal clock
//                       <1=> ext. 32KHz crystal clock
//                       <2=> HCLK
//                       <3=> int. 22MHz osc. clock
//     <o3.20>     PWM01_EN: PWM_0,PWM_1 Clock Enable
//     <o5.30..31> PWM13_S:  PWM2 and PWM3 Clock Source
//                       <0=> ext. 12MHz crystal clock
//                       <1=> ext. 32KHz crystal clock
//                       <2=> HCLK
//                       <3=> int. 22MHz osc. clock
//     <o3.21>     PWM23_EN: PWM_2,PWM_3 Clock Enable
//     <o5.26..27> CAN_S:  CAN Clock Source Select
//                       <0=> ext. 12MHz crystal clock
//                       <1=> ext. 32KHz crystal clock
//                       <2=> PLL clock
//                       <3=> int. 22MHz osc. clock
//     <o6.12..15> CAN_N:  CAN Clock Source Devider
//                       <i> APU clock = (CAN Clock Source) / Devider
//                       <1-16><#-1>
//     <o3.24>     CAN0_EN: CAN Bus Controller-0 Clock Enable
//     <o3.25>     CAN1_EN: CAN Bus Controller-1 Clock Enable
//     <o6.4..7>   USB_N:  USB Clock Source Devider
//                       <i> USB clock  = (PLL) / Devider
//                       <1-16><#-1>
//     <o3.27>     USBD_EN: USB FS Device Controller Clock Enable
//     <o5.2..3>   ADC_S:  ADC Clock Source Select
//                       <0=> ext. 12MHz crystal clock
//                       <1=> PLL clock
//                       <2=> int. 22MHz osc. clock
//     <o6.16..23>  ADC_N:  ADC Clock Source Devider
//                       <i> ADC Clock = (ADC Clock Source) / Devider
//                       <1-256><#-1>
//     <o3.28>     ADC_EN: Analog-Digital-Converter (ADC) Clock Enable
//     <o3.30>     ACMP_EN: Analog Comparator Clock Enable
//     <o3.31>     PS2_EN: PS2 Clock Enable
//   </h>
//   <o2.1>  PDMA_EN: PDMA Controller Clock Enable
//   <o2.2>  ISP_EN: Flash ISP Controller Clock Enable
// </e>

#define CLOCK_SETUP           1
#define PWRCON_VAL            0x0000000F
#define AHBCLK_VAL            0x00000001
#define APBCLK_VAL            0x00000000
#define CLKSEL0_VAL           0x0000001A
#define CLKSEL1_VAL           0x00000000
#define CLKDIV_VAL            0x00000000
#define PLLCON_VAL            0x0000C22E

/*--------------------- Unlock System Register -------------------------------
//
// <e> Unlock System Register
// </e>
*/
#define UNLOCK_SETUP          0

/*
//-------- <<< end of configuration section >>> ------------------------------
*/

/*----------------------------------------------------------------------------
  Check the register settings
 *----------------------------------------------------------------------------*/
#define CHECK_RANGE(val, min, max)                ((val < min) || (val > max))
#define CHECK_RSVD(val, mask)                     (val & mask)

/* Clock Configuration -------------------------------------------------------*/
#if (CHECK_RSVD((PWRCON_VAL),  ~0x000001FF))
   #error "PWRCON: Invalid values of reserved bits!"
#endif

#if (CHECK_RSVD((AHBCLK_VAL),  ~0x00000003))
   #error "AHBCLK: Invalid values of reserved bits!"
#endif

#if (CHECK_RSVD((APBCLK_VAL),  ~0xBB33F3FF))
   #error "APBCLK: Invalid values of reserved bits!"
#endif

#if (CHECK_RSVD((CLKSEL0_VAL),  ~0x0000003F))
   #error "CLKSEL0: Invalid values of reserved bits!"
#endif

#if (CHECK_RSVD((CLKSEL1_VAL),  ~0xFF33330F))
   #error "CLKSEL1: Invalid values of reserved bits!"
#endif

#if (CHECK_RSVD((CLKDIV_VAL),  ~0x00FFFFFF))
   #error "CLKDIV: Invalid values of reserved bits!"
#endif

#if (CHECK_RSVD((PLLCON_VAL),  ~0x000FFFFF))
   #error "PLLCON: Invalid values of reserved bits!"
#endif



/*----------------------------------------------------------------------------
  DEFINES
 *----------------------------------------------------------------------------*/
    
/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define __XTL32K          (   32768UL)       /* External 32KHz Crystal        */
#define __XTL12M          (12000000UL)       /* External 12MHz Crystal        */
#define __OSC22M          (22118400UL)       /* Internal 22MHz RC Oscillator  */
#define __OSC10K          (   10000UL)       /* Internal 10KHz RC Oscillator  */


#define __HCLK_SEL     ((CLKSEL0_VAL >>  0) & 0x07)
#define __STCLK_SEL    ((CLKSEL0_VAL >>  3) & 0x07)
#define __PLL_NO_SEL   ((PLLCON_VAL  >> 14) & 0x03)

#if (CLOCK_SETUP)                            /* Clock Setup                   */

    #if   (__HCLK_SEL == 0)                  /* external 12MHz crystal clock  */
       #define __HCLK              (__XTL12M)
    #elif (__HCLK_SEL == 1)                  /* external 32KHz crystal clock  */
       #define __HCLK              (__XTL32K)
    #elif (__HCLK_SEL == 2)                  /* PLL clock                     */
       #if (PLLCON_VAL & (1<<19))
         #define __PLL_FIN         (__OSC22M)
       #else
         #define __PLL_FIN         (__XTL12M)
       #endif
       #define __PLL_NR            (((PLLCON_VAL >> 9) & 0x01F) + 2)
       #define __PLL_NF            (((PLLCON_VAL >> 0) & 0x1FF) + 2)

       #if   (__PLL_NO_SEL == 0)
         #define __PLL_NO          (1)
       #elif (__PLL_NO_SEL == 3)
         #define __PLL_NO          (4)
       #else
         #define __PLL_NO          (2)
       #endif
       #define __HCLK              ((__PLL_FIN / __PLL_NO) * (__PLL_NF / __PLL_NR))
    #elif (__HCLK_SEL == 3)                  /* internal 10KHz osc. clock     */
       #define __HCLK              (__OSC10K)
    #else                                    /* internal 22MHz osc. clock     */
       #define __HCLK              (__OSC22M)
    #endif

    #define __SYSTEM_CLK           (__HCLK / ((CLKDIV_VAL & 0x0F) + 1))  

    #if   (__STCLK_SEL == 0)                 /* external 12MHz crystal clock  */
       #define __STCLK             (__XTL12M)
    #elif (__STCLK_SEL == 1)                 /* external 32KHz crystal clock  */
       #define __STCLK             (__XTL32K)
    #elif (__STCLK_SEL == 2)                 /* external 12MHz / 2            */
       #define __STCLK             (__XTL12M / 2)
    #elif (__STCLK_SEL == 3)                 /* HCLK / 2                      */
       #define __STCLK             (__SYSTEM_CLK / 2)
    #else                                    /* internal 22MHz osc. clock     */
       #define __STCLK             (__OSC22M / 2)
    #endif

    #define __SYSTICK_EXTREF_CLK   (__STCLK)  

#else
  #define __SYSTEM_CLK             (__OSC22M)
  #define __SYSTICK_EXTREF_CLK     (__OSC22M / 2)
#endif  // CLOCK_SETUP 


/*----------------------------------------------------------------------------
  Clock Variable definitions
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock    = __SYSTEM_CLK;          /*!< System Clock Frequency (Core Clock)*/
uint32_t SysTickExtRefClock = __SYSTICK_EXTREF_CLK;  /*!< SysTick External Reference Clock   */


/*----------------------------------------------------------------------------
  Clock functions
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
  uint32_t pllFIN, pllNO, pllNF, pllNR;

  /* Determine clock frequency according to clock register values             */
  switch (NUC_CLK->CLKSEL0 & 0x07) {         /* select system clock source    */ 
    case 0:                                  /* external 12MHz crystal clock  */
       SystemCoreClock =     __XTL12M;
      break;
    case 1:                                  /* external 32KHz crystal clock  */
       SystemCoreClock =     __XTL32K;
      break;
    case 2:                                  /* PLL clock                     */
       pllFIN = ((NUC_CLK->PLLCON & (1<<19)) != 0) ? __OSC22M : __XTL12M;
       pllNR  = ((NUC_CLK->PLLCON >> 9) & 0x01F) + 2;
       pllNF  = ((NUC_CLK->PLLCON >> 0) & 0x1FF) + 2;
       switch ((NUC_CLK->PLLCON >> 14) & 0x03) {
         case 0:
           pllNO = 1;
           break;
         case 3:
           pllNO = 4;
           break;
         default:
            pllNO = 2;
          break;
       }
       SystemCoreClock =     (pllFIN / pllNO) * (pllNF / pllNR);
      break;
    case 3:                                  /* internal 10KHz osc. clock     */
       SystemCoreClock =     __OSC10K;
      break;
    default:                                 /* internal 22MHz osc. clock     */
       SystemCoreClock =     __OSC22M;
      break;
  }

  SystemCoreClock /= ((NUC_CLK->CLKDIV & 0x0F) + 1);  

  /* Determine Systick clock frequency according to clock register values     */
  switch ((NUC_CLK->CLKSEL0 >> 3) & 0x07) {  /* select SysTick clock source   */ 
    case 0:                                  /* external 12MHz crystal clock  */
       SysTickExtRefClock =  __XTL12M;
      break;
    case 1:                                  /* external 32KHz crystal clock  */
       SysTickExtRefClock =  __XTL32K;
      break;
    case 2:                                  /* external 12MHz / 2            */
       SysTickExtRefClock =  __XTL12M / 2;
      break;
    case 3:                                  /* HCLK / 2                      */
       SysTickExtRefClock =  SystemCoreClock / 2;
      break;
    default:                                 /* internal 22MHz / 2            */
       SysTickExtRefClock =  __OSC22M / 2;
      break;
  }

}

/**
 * Initialize the system
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system.
 *         Initialize the System.
 */
void SystemInit (void)
{
#if (CLOCK_SETUP)                            /* Clock Setup                   */
  int delay;

  NUC_GCR->RegLockAddr = 0x59;               /* unlock System Register        */
  NUC_GCR->RegLockAddr = 0x16;
  NUC_GCR->RegLockAddr = 0x88;

  NUC_CLK->PWRCON = (NUC_CLK->PWRCON & 0x0F) | PWRCON_VAL;
  delay = 100; while (delay--);

  NUC_CLK->PLLCON = PLLCON_VAL;
  delay = 100; while (delay--);

  NUC_CLK->CLKDIV  = CLKDIV_VAL;
  NUC_CLK->AHBCLK  = AHBCLK_VAL | 0x01;
  NUC_CLK->CLKSEL0 = CLKSEL0_VAL;
  delay = 100; while (delay--);

  NUC_CLK->CLKSEL1 = CLKSEL1_VAL;

  NUC_CLK->APBCLK  = APBCLK_VAL;

  NUC_GCR->RegLockAddr = 0x00;               /* lock System Register          */
#endif

#if (UNLOCK_SETUP)               /* unlock System Register */
  NUC_GCR->RegLockAddr = 0x59;
  NUC_GCR->RegLockAddr = 0x16;
  NUC_GCR->RegLockAddr = 0x88;
#endif

}
