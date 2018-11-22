/*----------------------------------------------------------------------------
 *      R T L   T C P N E T   E x a m p l e
 *----------------------------------------------------------------------------
 *      Name:    CLIENT.C
 *      Purpose: Example LED Control Client application
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

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------

//   <h>Remote IP Address
//   ====================
//
//     <o>IP1: Address byte 1 <0-255>
//     <i> Default: 192
#define IP1            192

//     <o>IP2: Address byte 2 <0-255>
//     <i> Default: 168
#define IP2            168

//     <o>IP3: Address byte 3 <0-255>
//     <i> Default: 0
#define IP3            0

//     <o>IP4: Address byte 4 <0-255>
//     <i> Default: 100
#define IP4            100

//   </h>

//   <o>Remote Port <1-65535>
//   <i> Do not set number of port too small,
//   <i> maybe it is already used.
//   <i> Default: 1001
#define PORT_NUM       1001

//   <o>Communication Protocol <0=> TCP <1=> UDP
//   <i> Selecet a protocol for sending data.
#define PROTOCOL       1

//   <o>LED Blinking speed <1-100>
//   <i> Blinking speed = SPEED * 100ms
//   <i> Default: 2
#define SPEED          2


//------------- <<< end of configuration section >>> -----------------------


#define BLINKLED 0x01  /* Command for blink the leds on board */
#define SENDLEN  2     /* Number of bytes to send */
#define TCP      0
#define UDP      1

BOOL tick;
U8 socket_tcp;
U8 socket_udp;
U8 Rem_IP[4] = {IP1,IP2,IP3,IP4};


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
   /* LCD Module.2x16 init*/
  ;	  // no display available
}


/*--------------------------- timer_poll ------------------------------------*/

static void timer_poll () {
   /* System tick timer running in poll mode */

   if (TIM0->SR & TIM_FLAG_OC2) {
      TIM0->SR = ~TIM_FLAG_OC2;
      /* Timer tick every 100 ms */
      timer_tick ();
      tick = __TRUE;
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


/*--------------------------- UDP socket ------------------------------------*/

U16 udp_callback (U8 soc, U8 *rip, U16 rport, U8 *buf, U16 len) {
   /* This function is called by the UDP module when UDP packet is received. */

   /* Make a reference to suppress compiler warnings. */
   buf  = buf;
   rip  = rip;
   rport= rport;
   len  = len;
   soc  = soc;
   return (0);
}


/*--------------------------- TCP socket ------------------------------------*/

U16 tcp_callback (U8 soc, U8 evt, U8 *ptr, U16 par) {
   /* This function is called by the TCP module on TCP event */
   /* Check the Net_Config.h for possible events.            */

   /* Make a reference to suppress compiler warnings. */
   soc = soc;
   par = par;
   evt = evt;
   ptr = ptr;
   return (0);
}


/*--------------------------- TCP send --------------------------------------*/

void send_data (U8 p2val) {
   U8 *sendbuf;
   U8 p2;

   /* UDP */
   if (socket_udp != 0) {
      /* Start Connection */
      sendbuf = udp_get_buf (SENDLEN);
      sendbuf[0] = BLINKLED;
      sendbuf[1] = p2val;
      udp_send (socket_udp, Rem_IP, 1001, sendbuf, SENDLEN);
   }

   /* TCP */
   if (socket_tcp != 0) {
      /* Start Connection */
      p2 = p2val;
      switch (tcp_get_state(socket_tcp)) {
         case TCP_STATE_FREE:
         case TCP_STATE_CLOSED:
            tcp_connect (socket_tcp, Rem_IP, PORT_NUM, 0);
            break;
         case TCP_STATE_CONNECT:
            if (tcp_check_send (socket_tcp) == __TRUE) {
               sendbuf = tcp_get_buf(SENDLEN);
               sendbuf[0] = BLINKLED;
               sendbuf[1] = p2;
               tcp_send (socket_tcp, sendbuf, SENDLEN);
            }
            break;
      }
   }
}


/*--------------------------- main ------------------------------------------*/

int main (void) {
   /* Main Thread of the TcpNet */
   U8 p2val, cnt, lshf;
   U8 protocol;

   init ();
   init_display ();
   init_TcpNet ();

   protocol = PROTOCOL;
   switch (protocol) {
      case TCP:
         socket_tcp = tcp_get_socket (TCP_TYPE_CLIENT, 0, 10, tcp_callback);
         break;
      case UDP:
         socket_udp = udp_get_socket (0, UDP_OPT_SEND_CS | UDP_OPT_CHK_CS, udp_callback);
         if (socket_udp != 0) {
            udp_open (socket_udp, PORT_NUM);
         }
         break;
   }

   p2val = 1;
   cnt   = 0;
   lshf  = 1;  /* left shift */
   while (1) {
      timer_poll ();
      main_TcpNet ();
      if (tick == __TRUE) {
         if (++cnt == SPEED) {
            GPIO6->DR[0x0F*4] = ~(p2val & 0x0F);
            send_data(p2val);
            p2val = lshf ? (p2val << 1) : (p2val >> 1);
            if (p2val == 0x80) lshf = 0;
            if (p2val == 0x01) lshf = 1;
            cnt = 0;
        }
        tick = __FALSE;
     }
   }
}


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/


