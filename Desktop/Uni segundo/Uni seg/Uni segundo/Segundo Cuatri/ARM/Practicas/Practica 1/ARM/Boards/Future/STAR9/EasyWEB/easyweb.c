/******************************************************************
 *****                                                        *****
 *****  Name: easyweb.c                                       *****
 *****  Ver.: 1.0                                             *****
 *****  Date: 07/05/2001                                      *****
 *****  Auth: Andreas Dannenberg                              *****
 *****        HTWK Leipzig                                    *****
 *****        university of applied sciences                  *****
 *****        Germany                                         *****
 *****  Func: implements a dynamic HTTP-server by using       *****
 *****        the easyWEB-API                                 *****
 *****                                                        *****
 ******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define extern            // Keil: Line added for modular project management

#include "easyweb.h"

#include "ENET.h"         // Keil: *.c -> *.h    // ethernet packet driver
#include "tcpip.h"        // Keil: *.c -> *.h    // easyWEB TCP/IP stack
#include <91x_lib.h>      // Keil: Register definition file for STR912

#include "webpage.h"      // webside for our HTTP server (HTML)

#define LED_RED   0x04
#define LED_YEL   0x40

#define LED_On(led)     GPIO2->DR[led*4] = ~led
#define LED_Off(led)    GPIO2->DR[led*4] =  led

void RTC_init(void) {
  
  RTC->CR  |=  0x00000080;                       // write enable
  RTC->TR   =  0x00000000;                       // reset time
  RTC->DTR  =  0x00000101;                       // reset date
  RTC->CR  &= ~0x00000080;                       // write disable

}

void RTC_getTime (unsigned int *hour, 
                  unsigned int *min, 
                  unsigned int *sec  ) {

  unsigned int tr;

  tr = RTC->TR;                                  // read time register

  *sec  = (((tr >> 4)& 0x0F)*10) + (tr & 0x0F);  // prepare seconds
  tr = tr >> 8;
  *min  = (((tr >> 4)& 0x0F)*10) + (tr & 0x0F);  // prepare minutes
  tr = tr >> 8;
  *hour = (((tr >> 4)& 0x0F)*10) + (tr & 0x0F);  // prepare hours
}

unsigned int sec, min, hour;

int main(void)
{

  /* configure GPIO6 for LEDs */
  SCU->GPIOOUT[2]   = 0x1010;                    // P2.2, P2.6 output - mode 1
  SCU->GPIOTYPE[2]  = 0x44;
  GPIO2->DDR        = 0x44;                      // P2.2, P2.6 Outputs (LED Data)
  GPIO2->DR[0x44*4] = 0x44;	                     // clear LEDs

  RTC_init();                                    // RealTimeClock init
 
  TCPLowLevelInit();
  LED_On(LED_YEL);	                             // Initialization finished

  HTTPStatus = 0;                                // clear HTTP-server's flag register
  TCPLocalPort = TCP_PORT_HTTP;                  // set port we want to listen to
  
  while (1)                                      // repeat forever
  {

    if (!(SocketStatus & SOCK_ACTIVE)) TCPPassiveOpen();   // listen for incoming TCP-connection
    DoNetworkStuff();                                      // handle network and easyWEB-stack
                                                           // events
    HTTPServer();

    RTC_getTime (&hour, &min, &sec);
    if (sec & 0x01) {
      LED_On(LED_RED);
    }
    else {
      LED_Off(LED_RED);
    }
  }
}

// This function implements a very simple dynamic HTTP-server.
// It waits until connected, then sends a HTTP-header and the
// HTML-code stored in memory. Before sending, it replaces
// some special strings with dynamic values.
// NOTE: For strings crossing page boundaries, replacing will
// not work. In this case, simply add some extra lines
// (e.g. CR and LFs) to the HTML-code.

void HTTPServer(void)
{
  if (SocketStatus & SOCK_CONNECTED)             // check if somebody has connected to our TCP
  {
    if (SocketStatus & SOCK_DATA_AVAILABLE)      // check if remote TCP sent data
      TCPReleaseRxBuffer();                      // and throw it away

    if (SocketStatus & SOCK_TX_BUF_RELEASED)     // check if buffer is free for TX
    {
      if (!(HTTPStatus & HTTP_SEND_PAGE))        // init byte-counter and pointer to webside
      {                                          // if called the 1st time
        HTTPBytesToSend = sizeof(WebSide) - 1;   // get HTML length, ignore trailing zero
        PWebSide = (unsigned char *)WebSide;     // pointer to HTML-code
      }

      if (HTTPBytesToSend > MAX_TCP_TX_DATA_SIZE)     // transmit a segment of MAX_SIZE
      {
        if (!(HTTPStatus & HTTP_SEND_PAGE))           // 1st time, include HTTP-header
        {
          memcpy(TCP_TX_BUF, GetResponse, sizeof(GetResponse) - 1);
          memcpy(TCP_TX_BUF + sizeof(GetResponse) - 1, PWebSide, MAX_TCP_TX_DATA_SIZE - sizeof(GetResponse) + 1);
          HTTPBytesToSend -= MAX_TCP_TX_DATA_SIZE - sizeof(GetResponse) + 1;
          PWebSide += MAX_TCP_TX_DATA_SIZE - sizeof(GetResponse) + 1;
        }
        else
        {
          memcpy(TCP_TX_BUF, PWebSide, MAX_TCP_TX_DATA_SIZE);
          HTTPBytesToSend -= MAX_TCP_TX_DATA_SIZE;
          PWebSide += MAX_TCP_TX_DATA_SIZE;
        }
          
        TCPTxDataCount = MAX_TCP_TX_DATA_SIZE;   // bytes to xfer
        InsertDynamicValues();                   // exchange some strings...
        TCPTransmitTxBuffer();                   // xfer buffer
      }
      else if (HTTPBytesToSend)                  // transmit leftover bytes
      {
        memcpy(TCP_TX_BUF, PWebSide, HTTPBytesToSend);
        TCPTxDataCount = HTTPBytesToSend;        // bytes to xfer
        InsertDynamicValues();                   // exchange some strings...
        TCPTransmitTxBuffer();                   // send last segment
        TCPClose();                              // and close connection
        HTTPBytesToSend = 0;                     // all data sent
      }

      HTTPStatus |= HTTP_SEND_PAGE;              // ok, 1st loop executed
    }
  }
  else
    HTTPStatus &= ~HTTP_SEND_PAGE;               // reset help-flag if not connected
}


// searches the TX-buffer for special strings and replaces them
// with dynamic values 

void InsertDynamicValues(void)
{
  unsigned int sec, min, hour;
  unsigned char *Key;
           char NewKey[10];
  unsigned int i;
  
  if (TCPTxDataCount < 4) return;                     // there can't be any special string

  Key = TCP_TX_BUF;
  
  for (i = 0; i < (TCPTxDataCount - 3); i++)
  {
    if (*Key == '_')
      if (*(Key + 7) == '_') {
        RTC_getTime (&hour, &min, &sec);
        sprintf(NewKey, "%02d:%02d:%02d", hour, min, sec);     // insert special value
        memcpy(Key, NewKey, 8);                  
    }
    Key++;
  }
}
