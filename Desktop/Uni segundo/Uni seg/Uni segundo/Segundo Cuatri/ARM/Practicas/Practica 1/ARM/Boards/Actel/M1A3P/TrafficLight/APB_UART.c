//**********************************************************************
// This code was created by SoC Solutions and is free for use with     *
// Actel products.  For support e-mail support@socsolutions.com.       *
//**********************************************************************

#include "socDefs.h"
#include "APB_UART.h"

//Poll for transmit holding register empty
void uartWaitXmt (unsigned baseAddr){
    unsigned char status;

    do
    {   GET_UINT8(baseAddr+UART_STATUS_OFFSET, &status);
    } while (!(status & UART_STATUS_TXREADY_MASK));
}

// Send a string to the UART
void uart_puts(unsigned baseAddr, unsigned char *str, unsigned len)
{
    int i;

    for (i=0;i<len;i++)
    {
        uartWaitXmt(baseAddr);
        PUT_UINT8(baseAddr+UART_TXDATA_OFFSET, str[i]);
    }
}

void uart_cfg(unsigned baseAddr, unsigned baudRate)
{
    PUT_UINT8(baseAddr+UART_CONTROL1_OFFSET, baudRate);
    PUT_UINT8(baseAddr+UART_CONTROL2_OFFSET, UART_CONTROL2_8BITS | UART_CONTROL2_NOPARITY);
}

unsigned uart_getchar(unsigned baseAddr)
{
    unsigned uartRxData;

    GET_UINT32(baseAddr+UART_RXDATA_OFFSET, &uartRxData);
    return (uartRxData & 0xFF);
}
