/*----------------------------------------------------------------------------
 *      R T L   T C P N E T   E x a m p l e
 *----------------------------------------------------------------------------
 *      Name:    LEDSWITCH.C
 *      Purpose: Example LED Control Server application
 *      Rev.:    V3.12
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2007 KEIL - An ARM Company. All rights reserved. 
 *---------------------------------------------------------------------------*/

#include <RTL.h>
#include <91x_lib.h>                    /* STR912F definitions               */

#define PCLK       48000000                   /* Timer Clock 48 MHz          */
#define TCLK       10                         /* Timer Clock rate 10/s       */
#define TCNT       (PCLK/TCLK/100-5)          /* Timer Counts                */

U8 socket_tcp;
U8 socket_udp;

#define PORT_NUM 1001
#define BLINKLED 0x01  // Command for blink the leds on board

/*--------------------------- init ------------------------------------------*/

static void init () {
   /* Add System initialisation code here */

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


/*--------------------------- init_display ----------------------------------*/

static void init_display () {
  ;	  // no display available
}


/*--------------------------- timer_poll ------------------------------------*/

static void timer_poll () {
   /* System tick timer running in poll mode */

   if (TIM0->SR & TIM_FLAG_OC2) {
      TIM0->SR = ~TIM_FLAG_OC2;
      /* Timer tick every 100 ms */
      timer_tick ();
   }
}


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


/*--------------------------- Process received data  ------------------------*/

void procrec (U8 *buf) {
   switch (buf[0]) {
      case BLINKLED:
         GPIO6->DR[0x0F*4] = ~buf[1];
         break;
   }
}
/*--------------------------- UDP socket ------------------------------------*/

U16 udp_callback (U8 soc, U8 *rip, U16 rport, U8 *buf, U16 len) {
   rip  = rip;
   rport= rport;
   len  = len;

   if (soc != socket_udp) {
      /* Check if this is the socket we are connected to */
      return (0);
   }

   procrec(buf);
   return (0);
}

/*--------------------------- TCP socket ------------------------------------*/

U16 tcp_callback (U8 soc, U8 evt, U8 *ptr, U16 par) {
   /* This function is called by the TCP module on TCP event */
   /* Check the 'Net_Config.h' for possible events.          */
   par = par;

   if (soc != socket_tcp) {
      return (0);
   }

   switch (evt) {
      case TCP_EVT_DATA:
         /* TCP data frame has arrived, data is located at *par1, */
         /* data length is par2. Allocate buffer to send reply.   */
         procrec(ptr);
         break;

      case TCP_EVT_CONREQ:
         /* Remote peer requested connect, accept it */
         return (1);

      case TCP_EVT_CONNECT:
         /* The TCP socket is connected */
         return (1);
   }
   return (0);
}


/*--------------------------- main ------------------------------------------*/

int main (void) {
   /* Main Thread of the TcpNet */

   init ();
   init_display ();
   init_TcpNet ();

   /* Initialize UDP Socket and start listening */
   socket_udp = udp_get_socket (0, UDP_OPT_SEND_CS | UDP_OPT_CHK_CS, udp_callback);
   if (socket_udp != 0) {
      udp_open (socket_udp, PORT_NUM);
   }

   /* Initialize TCP Socket and start listening */
   socket_tcp = tcp_get_socket (TCP_TYPE_SERVER, 0, 10, tcp_callback);
   if (socket_tcp != 0) {
      tcp_listen (socket_tcp, PORT_NUM);
   }

   while (1) {
      timer_poll ();
      main_TcpNet ();
   }
}


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/


