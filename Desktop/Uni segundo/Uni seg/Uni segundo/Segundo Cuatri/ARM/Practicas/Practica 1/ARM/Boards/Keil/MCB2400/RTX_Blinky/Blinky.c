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
#include <LPC23xx.H>                    /* LPC23xx/LPC24xx definitions       */

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_led;                           /* assigned task id of task: lcd     */

#define LED_A      0x01
#define LED_B      0x02
#define LED_C      0x04
#define LED_D      0x08

#define I2C_AA     0x00000004
#define I2C_SI     0x00000008
#define I2C_STO    0x00000010
#define I2C_STA    0x00000020
#define I2C_I2EN   0x00000040

static U32 leds;

/*----------------------------------------------------------------------------
 *        Function that initializes LEDs
 *---------------------------------------------------------------------------*/
static void LED_Init (void) {
  PCONP    |=  (1 << 7);                /* Enable clock for I2C0             */

  /* Initialize pins for SDA (P0.27) and SCL (P0.28) functions               */
  PINSEL1  &= ~0x03C00000;
  PINSEL1  |=  0x01400000;

  /* Clear I2C state machine                                                 */
  I20CONCLR =  I2C_AA | I2C_SI | I2C_STA | I2C_I2EN;

  /* Setup I2C clock speed                                                   */
  I20SCLL   =  0x80;
  I20SCLH   =  0x80;

  I20CONSET =  I2C_I2EN;
  I20CONSET =  I2C_STO;
}

/*----------------------------------------------------------------------------
 *        Function that outputs value to LEDs
 *---------------------------------------------------------------------------*/
static void LED_Out (U32 val) {
  U32 v;

  v =  (val & 0x01)       | ((val << 1) & 0x04) | 
      ((val << 2) & 0x10) | ((val << 3) & 0x40);
  I20CONCLR =  I2C_AA | I2C_SI | I2C_STA | I2C_STO;
  I20CONSET =  I2C_STA;
  while (!(I20CONSET & I2C_SI));         /* Wait for START                   */
  I20DAT    =  0xC0;
  I20CONCLR =  I2C_SI | I2C_STA;
  while (!(I20CONSET & I2C_SI));         /* Wait for ADDRESS send            */
  I20DAT    =  0x18;
  I20CONCLR =  I2C_SI;
  while (!(I20CONSET & I2C_SI));         /* Wait for DATA send               */
  I20DAT    =  v;
  I20CONCLR =  I2C_SI;
  while (!(I20CONSET & I2C_SI));         /* Wait for DATA send               */
  I20CONSET =  I2C_STO;
  I20CONCLR =  I2C_SI;
  while (I20CONSET & I2C_STO);           /* Wait for STOP                    */
}

/*----------------------------------------------------------------------------
 *        Function 'signal_func' called from multiple tasks
 *---------------------------------------------------------------------------*/
void signal_func (OS_TID task)  {
  os_dly_wait (100);                     /* delay 50 clock ticks             */
  os_evt_set (0x0001, task);             /* send event to task 'task'        */
  os_dly_wait (50);                      /* delay 50 clock ticks             */
}

/*----------------------------------------------------------------------------
 *        Task 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
__task void phaseA (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    leds |= LED_A;
    signal_func (t_phaseB);              /* call common signal function      */
    leds &= ~LED_A;
  }
}

/*----------------------------------------------------------------------------
 *        Task 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
__task void phaseB (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    leds |= LED_B;
    signal_func (t_phaseC);              /* call common signal function      */
    leds &= ~LED_B;
  }
}

/*----------------------------------------------------------------------------
 *        Task 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
__task void phaseC (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    leds |= LED_C;
    signal_func (t_phaseD);              /* call common signal function      */
    leds &= ~LED_C;
  }
}

/*----------------------------------------------------------------------------
 *        Task 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
__task void phaseD (void) {
  for (;;) {
    os_evt_wait_and (0x0001, 0xffff);    /* wait for an event flag 0x0001    */
    leds |= LED_D;
    signal_func (t_phaseA);              /* call common signal function      */
    leds &= ~LED_D;
  }
}

/*----------------------------------------------------------------------------
 *        Task 5 'led': LED Control task
 *---------------------------------------------------------------------------*/
__task void led (void) {

  LED_Init ();                           /* Initialize LED display module    */
  for (;;) {
    LED_Out (leds);                      /* Update LED Driver                */
    os_dly_wait (2);
  }
}

/*----------------------------------------------------------------------------
 *        Task 6 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 0);  /* start task phaseC                */
  t_phaseD = os_tsk_create (phaseD, 0);  /* start task phaseD                */
  t_led    = os_tsk_create (led, 0);     /* start task led                   */
  os_evt_set (0x0001, t_phaseA);         /* send signal event to task phaseA */
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

