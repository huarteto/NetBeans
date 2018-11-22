/*----------------------------------------------------------------------------
 *      R T L   T C P N E T   E x a m p l e
 *----------------------------------------------------------------------------
 *      Name:    HTTP_DEMO.C
 *      Purpose: HTTP Server demo example
 *      Rev.:    V3.12
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2007 KEIL - An ARM Company. All rights reserved. 
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <RTL.h>
#include <Net_Config.h>
#include <91x_lib.h>                    /* STR912F definitions               */

#include "91x_mems.h"                            // driver for MEMS sensor

#define PCLK       48000000                   /* Timer Clock 48 MHz          */
#define TCLK       10                         /* Timer Clock rate 10/s       */
#define TCNT       (PCLK/TCLK/100-5)          /* Timer Counts                */

#define LED_RED     0x02
#define LED_GREEN   0x04
#define LED_YELLOW1 0x01
#define LED_YELLOW2 0x08

U8 LED_val;
U8 LED_mask;
U8 LED_blink_rate;
BOOL LEDblink;

BOOL LEDrun;
BOOL LCDupdate;
BOOL tick;
U32  dhcp_tout;
U8   lcd_text[2][16+1] = {"    RTL-ARM",      /* Buffer for LCD text         */
                          "  HTTP example" };

extern LOCALM localm[];                       /* Local Machine Settings      */
#define MY_IP localm[NETIF_ETH].IpAdr
#define DHCP_TOUT   100                       /* DHCP timeout 10 seconds      */

#ifdef RTX_KERNEL
 U64 tcp_stack[800/8];                        /* A bigger stack for tcp_task */

 /* Forward references */
 __task void init       (void);
 __task void blink_led  (void);
 __task void timer_task (void);
 __task void tcp_task   (void);
 static void init_display (void);
#endif

/*--------------------------- init ------------------------------------------*/

#ifndef RTX_KERNEL

static void init () {
   /* Add System initialisation code here */

   /* Configure peripherals for MEMs */
   Periph_Config_MEMs();                      

   /* I/O Ports configured as Output (Push-pull) */
   SCU->GPIOOUT[6]   = 0x0055;
   GPIO6->DDR        = 0x0F;
   GPIO6->DR[0x0f*4] = 0x0F;

   /* Timer 0 as interval timer, reload to 100ms. */
   TIM0->OC2R  =  TCNT;
   TIM0->CR2   =  TIM_FLAG_OC2 | (100 - 1);
   TIM0->CR1   =  TIM_PWM | 0x8000;

#if 0 // on STR9_DONGLE no RS232 Connector mounted
   /* UART1 configured for 115200 baud. */
   SCU->GPIOOUT[3] &= 0xFF3F;                /* Enable TX1 and RX1 pins      */
   SCU->GPIOOUT[3] |= 0x0080;
   SCU->GPIOIN[3]  |= 0x04;
   UART1->IBRD = 0x1A;                       /* Integer divider for 115kBaud */
   UART1->FBRD = 0x03;                       /* Fractional divider           */
   UART1->LCR  = 0x0060;                     /* 8 bits, no Parity, 1 Stop bit*/
   UART1->CR   = 0x0301;                     /* Enable UART                  */
#endif
}

#else

__task void init (void) {
   /* Add System initialisation code here */ 

   /* Configure peripherals for MEMs */
   Periph_Config_MEMs();                      

   /* I/O Ports configured as Output (Push-pull) */
   SCU->GPIOOUT[6]   = 0x0055;
   GPIO6->DDR        = 0x0F;
   GPIO6->DR[0x0f*4] = 0x0F;

#if 0 // on STR9_DONGLE no RS232 Connector mounted
   /* UART1 configured for 115200 baud. */
   SCU->GPIOOUT[3] &= 0xFF3F;                /* Enable TX1 and RX1 pins      */
   SCU->GPIOOUT[3] |= 0x0080;
   SCU->GPIOIN[3]  |= 0x04;
   UART1->IBRD = 0x1A;                       /* Integer divider for 115kBaud */
   UART1->FBRD = 0x03;                       /* Fractional divider           */
   UART1->LCR  = 0x0060;                     /* 8 bits, no Parity, 1 Stop bit*/
   UART1->CR   = 0x0301;                     /* Enable UART                  */
#endif

   LED_val  = LED_RED;
   LED_mask = LED_RED;
   LED_blink_rate = 1;
   LEDblink = __TRUE;                // yellow#1 is blinking with 100ms
   LEDrun = __FALSE;                 // start without running lights

   init_display ();
   init_TcpNet ();
   SPI_MEMS_Init();

   /* Initialize Tasks */
   os_tsk_prio_self (100);
   os_tsk_create (blink_led, 20);
   os_tsk_create (timer_task, 30);
   os_tsk_create_user (tcp_task, 0, &tcp_stack, sizeof(tcp_stack));
   os_tsk_delete_self();
}

#endif

/*--------------------------- timer_poll ------------------------------------*/

#ifndef RTX_KERNEL

static void timer_poll () {
   /* System tick timer running in poll mode */

   if (TIM0->SR & TIM_FLAG_OC2) {
      TIM0->SR = ~TIM_FLAG_OC2;
      /* Timer tick every 100 ms */
      timer_tick ();
      tick = __TRUE;
   }
}

#else

__task void timer_task (void) {
   /* System tick timer task */
   os_itv_set (10);
   while (1) {
      timer_tick ();
      tick = __TRUE;
      os_itv_wait ();
   }
}

#endif


/*--------------------------- sendchar --------------------------------------*/

int sendchar (int ch)  {
#if 0 // on STR9_DONGLE no RS232 Connector mounted
   /* Debug output to serial port. */

   if (ch == '\n')  {
      while (!(UART1->FR & 0x0080));
      UART1->DR = '\r';                      /* output CR                    */
   }
   while (!(UART1->FR & 0x0080));
   return (UART1->DR = ch);
#endif
   return (ch);
}


/*--------------------------- LED_out ---------------------------------------*/

void LED_out (U32 mask, U32 val) {
   GPIO6->DR[mask*4] = ~(val & 0x0F);
}


/*--------------------------- upd_display -----------------------------------*/

static void upd_display () {
   /* Update LCD Module display text. */

   ;	  // no display available
   LCDupdate =__FALSE;
}


/*--------------------------- init_display ----------------------------------*/

static void init_display () {
   /* LCD Module.2x16 init*/
   ;	  // no display available
}


/*--------------------------- dhcp_check ------------------------------------*/

static void dhcp_check () {
   /* Monitor DHCP IP address assignment. */

   if (tick == __FALSE || dhcp_tout == 0) {
      return;
   }
#ifdef RTX_KERNEL
   tick = __FALSE;
#endif
   if (mem_test (&MY_IP, 0, IP_ADRLEN) == __FALSE) {
      /* Success, DHCP has already got the IP address. */
      dhcp_tout = 0;

      LED_val  = LED_GREEN;
      LED_mask = LED_GREEN;
      LED_blink_rate = 10;
      LED_out (LED_RED, ~LED_RED);
      return;
   }
   if (--dhcp_tout == 0) {
      /* A timeout, disable DHCP and use static IP address. */
      dhcp_disable ();

      LED_val  = LED_YELLOW2;
      LED_mask = LED_YELLOW2;
      LED_blink_rate = 10;
      LED_out (LED_RED, ~LED_RED);
   }
}


/*--------------------------- blink_led -------------------------------------*/

#ifndef RTX_KERNEL

static void blink_led () {
   /* Blink the LEDs on MCB-STR9 board */
   const U8 led_val[14] = { 0x02,0x01,0x08,0x04,0x08,0x01,0x02,
                            0x02,0x03,0x09,0x0c,0x09,0x03,0x02 };
   static U32 tick_cnt;
   static U32 cnt;

   if (tick == __TRUE) {
      /* Every 100 ms */
      tick = __FALSE;

      if (++tick_cnt > LED_blink_rate) {
        tick_cnt = 0;
        if (LEDblink == __TRUE) {
           LED_out (LED_mask, LED_val);
           LED_val = ~LED_val & LED_mask;                // toggle led
        }
      }
      if (LEDrun == __TRUE) {
         LED_out (0x0F, led_val[cnt]);
         if (++cnt >= sizeof(led_val)) {
            cnt = 0;
         }
      }
      if (LCDupdate == __TRUE) {
         upd_display ();
      }
   }
}

#else

__task void blink_led () {
   /* Blink the LEDs on MCB-STR9 board */
   const U8 led_val[14] = { 0x02,0x01,0x08,0x04,0x08,0x01,0x02,
                            0x02,0x03,0x09,0x0c,0x09,0x03,0x02 };
   static U32 tick_cnt;
   static U32 cnt;

   while(1) {
      /* Every 100 ms */
      if (++tick_cnt > LED_blink_rate) {
        tick_cnt = 0;
        if (LEDblink == __TRUE) {
           LED_out (LED_mask, LED_val);
           LED_val = ~LED_val & LED_mask;                // toggle led
        }
      }
      if (LEDrun == __TRUE) {
         LED_out (0x0F, led_val[cnt]);
         if (++cnt >= sizeof(led_val)) {
            cnt = 0;
         }
      }
      if (LCDupdate == __TRUE) {
         upd_display ();
      }
      os_dly_wait(10);
   }
}

#endif

/*--------------------------- read_MEMS -------------------------------------*/

#ifndef RTX_KERNEL

char mc=0;
static void read_MEMS () {
//  if (mc++ ==0) // do not read MEMS data everytime - it is time consuming and useless here
    SPI_MEMS_Read(&mems_data);        
}

#else

__task void read_MEMS() {
 // SPI_MEMS_Read(&mems_data);        
    
}

#endif


/*---------------------------------------------------------------------------*/

#ifndef RTX_KERNEL

int main (void) {
   /* Main Thread of the TcpNet */

   init ();
   init_display ();
   init_TcpNet ();
   LED_out(0x0F, 0xFF);
   SPI_MEMS_Init();
   LED_out(0x0F, 0x00);

   LED_val  = LED_RED;
   LED_mask = LED_RED;
   LED_blink_rate = 1;
   LEDblink = __TRUE;                // yellow#1 is blinking with 100ms
   LEDrun = __FALSE;                 // start without running lights

   dhcp_tout = DHCP_TOUT;
   while (1) {
      timer_poll ();
      main_TcpNet ();
      dhcp_check ();
      blink_led ();
      read_MEMS();
   }
}

#else

__task void tcp_task (void) {
   /* Main Thread of the TcpNet. This task should have */
   /* the lowest priority because it is always READY. */
   dhcp_tout = DHCP_TOUT;
   while (1) {
      main_TcpNet();
      dhcp_check ();
      os_tsk_pass();
   }
}


int main (void) {
   /* Start with 'init' task. */
   os_sys_init(init);
   while(1);
}

#endif


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/


