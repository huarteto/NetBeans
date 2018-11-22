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
#include <iMX27.h>                      /* i.MX27 definitions                */

OS_TID t_turnON;                        /* assigned task id of task: turnON  */
OS_TID t_turnOFF;                       /* assigned task id of task: turnOFF */

enum LED_ENUM {OFF, ON};

#define LED_PIN     (1 << 18)

#define LED_INIT    PLLCLK_PCCR0  |=  1 << 25; \
                    GPIO_PTE_DDIR |=  LED_PIN; \
                    GPIO_PTE_GPR  &= ~LED_PIN; \
                    GPIO_PTE_OCR2 |=  3 <<  4; \
                    GPIO_PTE_GIUS |=  LED_PIN; \
                    GPIO_PTE_DR   &= ~LED_PIN;
#define LED(x)      ((x) ? (GPIO_PTE_DR|=LED_PIN) : (GPIO_PTE_DR&=~LED_PIN));


/*----------------------------------------------------------------------------
 *        Task 1 'turnON': Turn LED ON
 *---------------------------------------------------------------------------*/
__task void turnON (void) {
  for (;;) {
    LED(ON)
    os_dly_wait (200);                   /* delay 200 clock ticks            */
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'turnOFF': Turn LED OFF
 *---------------------------------------------------------------------------*/
__task void turnOFF (void) {
  os_dly_wait (100);                     /* delay 100 clock ticks            */
  for (;;) {
    LED(OFF)
    os_dly_wait (200);                   /* delay 200 clock ticks            */
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  LED_INIT                               /* Define LED pins as Outputs       */

  t_turnON  = os_tsk_create (turnON,  1);/* start task turnON                */
  t_turnOFF = os_tsk_create (turnOFF, 1);/* start task turnOFF               */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/

int main (void) {

  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

