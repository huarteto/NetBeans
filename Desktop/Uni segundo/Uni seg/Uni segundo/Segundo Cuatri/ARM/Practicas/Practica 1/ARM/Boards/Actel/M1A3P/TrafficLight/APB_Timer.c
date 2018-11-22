//**********************************************************************
// This code was created by SoC Solutions and is free for use with     *
// Actel products.  For support e-mail support@socsolutions.com.       *
//**********************************************************************

#include "socDefs.h"
#include "APB_Timer.h"

void timer_cfg(unsigned baseAddr, unsigned config, unsigned prescale)
{
    PUT_UINT32(baseAddr+TIMER_PRESCALE_OFFSET, prescale);
    PUT_UINT32(baseAddr+TIMER_CONTROL_OFFSET, config);
}

void timer_load(unsigned baseAddr, unsigned loadVal)
{
    PUT_UINT32(baseAddr+TIMER_LOAD_OFFSET, loadVal);
}

void timer_irq_clear(unsigned baseAddr)
{
    PUT_UINT32(baseAddr+TIMER_INTCLR_OFFSET, DONT_CARE);
}

