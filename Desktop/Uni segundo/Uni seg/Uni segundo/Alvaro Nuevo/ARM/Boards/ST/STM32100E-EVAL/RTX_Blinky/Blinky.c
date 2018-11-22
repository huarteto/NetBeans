/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2009 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include "stm32f10x.h"                  /* STM32F10x.h definitions            */
#include "GLCD.h"

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_clock;                         /* assigned task id of task: clock   */
OS_TID t_lcd;                           /* assigned task id of task: lcd     */

OS_MUT mut_GLCD;                        /* Mutex to controll GLCD access     */

#define LED_A      0
#define LED_B      1
#define LED_C      2
#define LED_D      3
#define LED_CLK    7

#define __FI        1                   /* Font index 16x24                   */

#define LED_NUM     4                   /* Number of user LEDs                */
const unsigned long led_mask[] = { 1UL <<  6, 1UL <<  7, 1UL << 8, 1UL << 9};


/*----------------------------------------------------------------------------
 * switch LED on
 *---------------------------------------------------------------------------*/
void LED_On  (unsigned char led) {
  if (led != LED_CLK) GPIOF->BSRR = led_mask[led];

  os_mut_wait(mut_GLCD, 0xffff);
#ifdef __USE_LCD
  GLCD_SetBackColor(White);                         /* Set the Back Color */
  GLCD_SetTextColor(Green);                         /* Set the Text Color */
  GLCD_DisplayChar(4, 5+led, __FI, 0x80+1);         /* Circle Full        */
#endif // __USE_LCD
  os_mut_release(mut_GLCD);
}

/*----------------------------------------------------------------------------
 * switch LED off
 *---------------------------------------------------------------------------*/
void LED_Off (unsigned char led) {
  if (led != LED_CLK) GPIOF->BRR = led_mask[led];

  os_mut_wait(mut_GLCD, 0xffff);
#ifdef __USE_LCD
  GLCD_SetBackColor(White);                         /* Set the Back Color */
  GLCD_SetTextColor(Green);                         /* Set the Text Color */
  GLCD_DisplayChar(4, 5+led, __FI, 0x80+0);         /* Circle Empty       */
#endif // __USE_LCD
  os_mut_release(mut_GLCD);
}


/*----------------------------------------------------------------------------
 *        Function 'signal_func' called from multiple tasks
 *---------------------------------------------------------------------------*/
void signal_func (OS_TID task)  {
  os_evt_set (0x0100, t_clock);          /* send event signal to clock task  */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
  os_evt_set (0x0100, t_clock);          /* send event signal to clock task  */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
  os_evt_set (0x0001, task);             /* send event to task 'task'        */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
}

/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_On (LED_A);
    signal_func (t_phaseB);              /* call common signal function      */
    LED_Off(LED_A);
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_On (LED_B);
    signal_func (t_phaseC);              /* call common signal function      */
    LED_Off(LED_B);
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_On (LED_C);
    signal_func (t_phaseD);              /* call common signal function      */
    LED_Off(LED_C);
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    LED_On (LED_D);
    signal_func (t_phaseA);              /* call common signal function      */
    LED_Off(LED_D);
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
__task void clock (void) {
  for (;;) {
    os_evt_wait_and (0x0100, 0xffff);    /* wait for an event flag 0x0100    */
    LED_On (LED_CLK);
    os_dly_wait (8);                     /* delay 8 clock ticks              */
    LED_Off(LED_CLK);
  }
}

/*----------------------------------------------------------------------------
 *        Task 6 'lcd': LCD Control task
 *---------------------------------------------------------------------------*/
__task void lcd (void) {

  for (;;) {
    os_mut_wait(mut_GLCD, 0xffff);
#ifdef __USE_LCD
    GLCD_SetBackColor(Blue);                           /* Set the Text Color */
    GLCD_SetTextColor(White);                          /* Set the Text Color */
    GLCD_DisplayString(0, 0, __FI, "STM32100E-EVAL Demo ");
    GLCD_DisplayString(1, 0, __FI, "    RTX Blinky      ");
    GLCD_DisplayString(2, 0, __FI, "   www.keil.com     ");
#endif // __USE_LCD
    os_mut_release(mut_GLCD);
    os_dly_wait (400);

    os_mut_wait(mut_GLCD, 0xffff);
#ifdef __USE_LCD
    GLCD_SetBackColor(Blue);                           /* Set the Text Color */
    GLCD_SetTextColor(Red);                            /* Set the Text Color */
    GLCD_DisplayString(0, 0, __FI, "STM32100E-EVAL Demo ");
    GLCD_DisplayString(1, 0, __FI, "    RTX Blinky      ");
    GLCD_DisplayString(2, 0, __FI, "   www.keil.com     ");
#endif // __USE_LCD
    os_mut_release(mut_GLCD);
    os_dly_wait (400);
  }
}

/*----------------------------------------------------------------------------
 *        Task 7 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  os_mut_init(mut_GLCD);

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 0);  /* start task phaseC                */
  t_phaseD = os_tsk_create (phaseD, 0);  /* start task phaseD                */
  t_clock  = os_tsk_create (clock, 0);   /* start task clock                 */
  t_lcd    = os_tsk_create (lcd, 0);     /* start task lcd                   */
  os_evt_set (0x0001, t_phaseA);         /* send signal event to task phaseA */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {

  /* Setup GPIO for LEDs                                                     */
  RCC->APB2ENR |= (1 << 7);             /* Enable GPIOF clock                 */
  GPIOF->ODR   &= ~0x000003C0;          /* switch off LEDs                    */
  GPIOF->CRL   &= ~0xFF000000;          /* Configure the GPIO for LEDs        */
  GPIOF->CRL   |=  0x33000000;
  GPIOF->CRH   &= ~0x000000FF;
  GPIOF->CRH   |=  0x00000033;
 
#ifdef __USE_LCD
  GLCD_Init();                           /* Initialize the GLCD              */
  GLCD_Clear(White);                     /* Clear the GLCD                   */
#endif // __USE_LCD

  os_sys_init(init);                     /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
