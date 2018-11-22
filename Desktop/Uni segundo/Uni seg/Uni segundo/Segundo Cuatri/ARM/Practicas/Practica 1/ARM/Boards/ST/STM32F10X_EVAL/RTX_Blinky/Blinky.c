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
#include <string.h>
#include <stm32f10x_lib.h>              /* STM32F10x Library Definitions     */
#include "lcd.h"
#include "spi_flash.h"

OS_TID t_phaseA;                        /* assigned task id of task: phase_a */
OS_TID t_phaseB;                        /* assigned task id of task: phase_b */
OS_TID t_phaseC;                        /* assigned task id of task: phase_c */
OS_TID t_phaseD;                        /* assigned task id of task: phase_d */
OS_TID t_lcd;                           /* assigned task id of task: lcd     */

#define LED_A   0x01
#define LED_B   0x02
#define LED_C   0x04
#define LED_D   0x08

#define LED_On(led)     GPIOC->ODR |=  (led << 6)
#define LED_Off(led)    GPIOC->ODR &= ~(led << 6)

extern const U8 Keil_Logo[];

/* Import functions from Setup.c                                             */
extern void SetupClock  (void);
extern void SetupLED    (void);
extern void SetupKBD    (void);

void Delay (U16 tick) {
  os_dly_wait(tick);
}

/*----------------------------------------------------------------------------
 *        Function 'signal_func' called from multiple tasks
 *---------------------------------------------------------------------------*/
void signal_func (OS_TID task)  {
  os_dly_wait (100);                     /* delay 100 clock ticks            */
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
 *        Task 5 'lcd': LCD task
 *---------------------------------------------------------------------------*/
__task void lcd (void) {
  U32 cnt;
  U8  buf[6];

  SPI_FLASH_Init();                      /* Initialize the SPI FLASH driver  */
  LCD_Init();                            /* Initialize the LCD               */
  LCD_Clear(White);                      /* Clear the LCD                    */

  /* Display ST Logo (if exists on SPI Flash)                                */
  SPI_FLASH_BufferRead(buf, 0x0064FD00, 6);
  if (strncmp((const char *)buf, "BM\x42\x58\x02\x00", 6) == 0) {
    LCD_SetDisplayWindow(239, 319, 240, 320);
    LCD_DrawBMP(0x0064FD00);
    Delay(100);
  }
  
  /* Display STM32 Slide (if exists on SPI Flash)                            */
  SPI_FLASH_BufferRead(buf, 0x00414400, 6);
  if (strncmp((const char *)buf, "BM\x42\x58\x02\x00", 6) == 0) {
    LCD_SetDisplayWindow(239, 319, 240, 320);
    LCD_DrawBMP(0x00414400);
    Delay(200);  
  }

  /* Display Keil Logo                                                       */
  LCD_Clear(White);                      /* Clear the LCD                    */
  LCD_SetBackColor(White);               /* Set the Back Color               */
  LCD_SetTextColor(0x0238);              /* Set the Text Color               */

  LCD_SetDisplayWindow(144, 240, 48, 160);
  LCD_DrawMonoBMP((U32)Keil_Logo);
  Delay(200);

  os_evt_set (0x0001, t_phaseA);         /* send signal event to task phaseA */

  LCD_SetDisplayWindow(239, 319, 240, 320);
  LCD_Clear(White);                      /* Clear the LCD                    */

  LCD_SetBackColor(Blue);                /* Set the Back Color               */
  LCD_SetTextColor(White);               /* Set the Text Color               */
  LCD_DisplayStringLine(Line0, "STM32 Demonstration ");
  LCD_SetBackColor(White);               /* Set the Back Color               */
  LCD_SetTextColor(Blue);                /* Set the Text Color               */
  LCD_DisplayStringLine(Line2, "Designed with RV-MDK");
  LCD_DisplayStringLine(Line3, "Real-Time OS Kernel ");

  LCD_SetBackColor(White);               /* Set the Back Color               */
  LCD_SetTextColor(Green);               /* Set the Text Color               */

  LCD_DisplayStringLine(Line6, "   Keyboard Test:   ");

  cnt = 0;

  for (;;) {

    if ((cnt & 0x07) == 0) {
      LCD_SetBackColor(Cyan);            /* Set the Back Color               */
      LCD_DisplayChar(Line4+12, 319-2*cnt, ' ');
      LCD_SetBackColor(White);           /* Set the Back Color               */
    }
    if (++cnt == 160) {
      cnt = 0;
      LCD_ClearLine(Line4+12);
    }

    LCD_SetTextColor(Red);               /* Set the Text Color               */

    /* Read "Left" Key */
    if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1)) {
      LCD_DisplayChar(Line8, 319-136, 0x80+9);
    } else {
      LCD_DisplayChar(Line8, 319-136, 0x80+8);
    }

    /* Read "Right" Key */
    if(!GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0)) {
      LCD_DisplayChar(Line8, 319-168, 0x80+11);
    } else {
      LCD_DisplayChar(Line8, 319-168, 0x80+10);
    }

    /* Read "Up" Key */
    if(!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_8)) {
      LCD_DisplayChar(Line7, 319-152, 0x80+5);
    } else {
      LCD_DisplayChar(Line7, 319-152, 0x80+4);
    }

    /* Read "Down" Key */
    if(!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_14)) {
      LCD_DisplayChar(Line9, 319-152, 0x80+7);
    } else {
      LCD_DisplayChar(Line9, 319-152, 0x80+6);
    }

    /* Read "Sel" Key */
    if(!GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12)) {
      LCD_DisplayChar(Line8, 319-152, 0x80+1);
    } else {
      LCD_DisplayChar(Line8, 319-152, 0x80+0);
    }

    LCD_SetTextColor(Magenta);           /* Set the Text Color               */

    /* Read "Key" Key */
    if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)) {
      LCD_DisplayChar(Line8, 319-206, 0x80+3);
    } else {
      LCD_DisplayChar(Line8, 319-206, 0x80+2);
    }

    os_dly_wait (5);                     /* delay 5 clock ticks              */
  }
}

/*----------------------------------------------------------------------------
 *        Task 8 'init': Initialize
 *---------------------------------------------------------------------------*/
__task void init (void) {

  t_phaseA = os_tsk_create (phaseA, 0);  /* start task phaseA                */
  t_phaseB = os_tsk_create (phaseB, 0);  /* start task phaseB                */
  t_phaseC = os_tsk_create (phaseC, 0);  /* start task phaseC                */
  t_phaseD = os_tsk_create (phaseD, 0);  /* start task phaseD                */
  t_lcd    = os_tsk_create (lcd, 0);     /* start task lcd                   */
  os_tsk_delete_self ();
}

/*----------------------------------------------------------------------------
 *        Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {

  SetupClock();
  SetupLED  ();
  SetupKBD  ();
  os_sys_init (init);                    /* Initialize RTX and start init    */
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

