//**********************************************************************
// This code was created by SoC Solutions and is free for use with     *
// Actel products.  For support e-mail support@socsolutions.com.       *
//**********************************************************************

#include "socDefs.h"
#include "APB_GPIO.h"

void gpio_out(unsigned baseAddr, unsigned dataOut)
{
    PUT_UINT32(baseAddr+GPIO_DATA_OFFSET, dataOut);
}

unsigned gpio_in(unsigned baseAddr)
{
    unsigned dataIn;
    GET_UINT32(baseAddr+GPIO_DATA_OFFSET, &dataIn);

    return dataIn;
}
