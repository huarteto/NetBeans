//**********************************************************************
// This code was created by SoC Solutions and is free for use with     *
// Actel products.  For support e-mail support@socsolutions.com.       *
//**********************************************************************

#include "socDefs.h"
#include "APB_Interrupt.h"
#include "platform.h"

void interrupt_irq_enable(unsigned baseAddr, unsigned irqMask)
{
    PUT_UINT32(baseAddr+INTERRUPT_IRQENABLE_OFFSET, irqMask);
}

void interrupt_irq_disable(unsigned baseAddr, unsigned irqMask)
{
    PUT_UINT32(baseAddr+INTERRUPT_IRQENABLECLR_OFFSET, irqMask);
}

unsigned interrupt_irq_status(unsigned baseAddr)
{
    unsigned status;
    GET_UINT32(baseAddr+INTERRUPT_IRQSTATUS_OFFSET, &status);

    return status;
}

void cm1_irq_enable(unsigned irqMask)
{
    PUT_UINT32(CM1_NVIC_ENABLE_REG, irqMask);
}

void cm1_irq_disable(unsigned irqMask)
{
    PUT_UINT32(CM1_NVIC_ENABLECLR_REG, irqMask);
}
