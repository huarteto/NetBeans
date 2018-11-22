//*****************************************************************************
//
// usbmode.c - Functions related to dual mode USB device/host operation.
//
// Copyright (c) 2008 Luminary Micro, Inc.  All rights reserved.
// 
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2523 of the Stellaris USB Library.
//
//*****************************************************************************

#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "hw_usb.h"
#include "usb.h"
#include "sysctl.h"
#include "interrupt.h"
#include "debug.h"
#include "usblib.h"
#include "usbdevice.h"
#include "usbhost.h"
#include "usblibpriv.h"

//*****************************************************************************
//
//! \addtogroup general_usblib_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// The following label defines interrupts that we will always pass to the host
// interrupt handler even if we are in dual mode and not yet sure of which
// mode we are operating in.
//
//*****************************************************************************
#define USB_HOST_INTS           (USB_INT_VBUS_ERR)

//*****************************************************************************
//
// Global variable indicating which mode of operation the application has
// requested.
//
//*****************************************************************************
static tUSBMode g_eUSBMode = USB_MODE_NONE;

//*****************************************************************************
//
// Global variable holding a pointer to the callback function which will be
// called when the USB mode changes between device and host.
//
//*****************************************************************************
static tUSBModeCallback g_pfnUSBModeCallback;

//*****************************************************************************
//
//! Allows a dual-mode application to switch between USB device and host modes.
//!
//! \param ulIndex specifies the USB controller whose mode of operation is to
//! be set.  This parameter must be set to 0.
//! \param eUSBMode indicates the mode that the application wishes to operate
//! in.  Valid values are \b USB_MODE_DEVICE to operate as a USB device and
//! \b USB_MODE_HOST to operate as a USB host.
//! \param pfnCallback is a pointer to a function which the USB library will
//! call each time the mode is changed to indicate the new operating mode.  In
//! cases where \e eUSBMode is set to either \b USB_MODE_DEVICE or
//! \b USB_MODE_HOST, the callback will be made immediately to allow the
//! application to perform any host or device specific initialization.
//!
//! This function allows a USB application, which can operate in host
//! or device mode, to indicate to the USB stack the mode that it wishes to
//! use.  The caller is responsible for cleaning up the interface and removing
//! itself from the bus prior to making this call and reconfiguring afterwards.
//!
//! For successful dual mode mode operation, an application must register
//! USB0DualModeIntHandler() as the interrupt handler for the USB0 interrupt.
//! This handler is responsible for steering interrupts to the device or host
//! stack depending upon the chosen mode.  Devices which do not require dual
//! mode capability should register either \e USB0DeviceIntHandler() or
//! \e USB0HostIntHandler() instead.  Registering \e USB0DualModeIntHandler()
//! for a single mode application will result in an application binary larger
//! than required since library functions for both USB operating modes will be
//! included even though only one mode is required.
//!
//! Single mode applications (those offering exclusively USB device or USB
//! host functionality) need not call this function since no interrupt
//! steering is required if the appropriate single mode interrupt handler is
//! installed.
//!
//! \return None.
//
//*****************************************************************************
void
USBStackModeSet(unsigned long ulIndex, tUSBMode eUSBMode,
                tUSBModeCallback pfnCallback)
{
    //
    // Check the arguments.
    //
    ASSERT(ulIndex == 0);
    ASSERT((eUSBMode == USB_MODE_HOST) || (eUSBMode == USB_MODE_DEVICE));
    ASSERT(pfnCallback);

    //
    // Remember the mode so that we can steer the interrupts appropriately.
    //
    g_eUSBMode = eUSBMode;

    //
    // Remember the callback pointer.
    //
    g_pfnUSBModeCallback = pfnCallback;

    //
    // If we are being asked to be either a host or device, we will not be
    // trying to auto-detect the mode so make the callback immediately.
    //
    if((eUSBMode == USB_MODE_DEVICE) || (eUSBMode == USB_MODE_HOST))
    {
        g_pfnUSBModeCallback(0, eUSBMode);
    }
}

//*****************************************************************************
//
//! Steers USB interrupts from controller to the correct handler in the USB
//! stack.
//!
//! This interrupt handler is used in applications which require to operate
//! in both host and device mode.  It steers the USB hardware interrupt to the
//! correct handler in the USB stack depending upon the current operating mode
//! of the application, USB device or host.
//!
//! For successful dual mode operation, an application must register
//! USB0DualModeIntHandler() in the CPU vector table as the interrupt handler
//! for the USB0 interrupt.  This handler is responsible for steering
//! interrupts to the device or host stack depending upon the chosen mode.
//!
//! \note Devices which do not require dual mode capability should register
//! either USB0DeviceIntHandler() or USB0HostIntHandler() instead.  Registering
//! USB0DualModeIntHandler() for a single mode application will result in an
//! application binary larger than required since library functions for both
//! USB operating modes will be included even though only one mode is actually
//! required.
//!
//! \return None.
//
//*****************************************************************************
void
USB0DualModeIntHandler(void)
{
    unsigned long ulStatus;

    //
    // Read the USB interrupt status.
    //
    ulStatus = USBIntStatus(USB0_BASE);

    //
    // Pass through the subset of interrupts that we always want
    // the host stack to see regardless of whether or not we
    // are actually in host mode at this point.
    //
    if(ulStatus & USB_HOST_INTS)
    {
        USBHostIntHandlerInternal(ulStatus & USB_HOST_INTS);

        //
        // We have already processed these interrupts so clear them
        // from the status.
        //
        ulStatus &= ~USB_HOST_INTS;
    }

    //
    // Steer the interrupt to the appropriate handler within the stack
    // depending upon our current operating mode.  Note that we need to pass
    // the ulStatus parameter since the USB interrupt register is
    // clear-on-read.
    //
    switch(g_eUSBMode)
    {
        case USB_MODE_NONE:
        {
            //
            // No mode is set yet so we have no idea what to do.  Just ignore
            // the interrupt.
            //
        }
        break;

        //
        // Operating in pure host mode.
        //
        case USB_MODE_HOST:
        {
            //
            // Call the host interrupt handler if there is anything still to
            // process.
            //
            if(ulStatus)
            {
                USBHostIntHandlerInternal(ulStatus);
            }
        }
        break;

        //
        // Operating in pure device mode.
        //
        case USB_MODE_DEVICE:
        {
            //
            // Call the device interrupt handler.
            //
            USBDeviceIntHandlerInternal(ulStatus);
        }
        break;
    }
}

//*****************************************************************************
//
//! Initializes the USB controller for dual mode operation.
//!
//! \param ulIndex specifies the USB controller that is to be initialized for
//! dual mode operation.  This parameter must be set to 0.
//!
//! This function initializes the USB controller hardware into a state
//! suitable for dual mode operation.  Applications may use this function to
//! ensure that the controller is in a neutral state and able to receive
//! appropriate interrupts before host or device mode is chosen using a call
//! to USBStackModeSet().
//!
//! \return None.
//
//*****************************************************************************
void
USBDualModeInit(unsigned long ulIndex)
{
    //
    // Configure the End point 0.
    //
    USBHostEndpointConfig(USB0_BASE, USB_EP_0, 64, 0, 0,
                          (USB_EP_MODE_CTRL | USB_EP_SPEED_FULL |
                           USB_EP_HOST_OUT));

    //
    // Enable USB Interrupts.
    //
    USBIntEnable(USB0_BASE, (USB_INT_RESET | USB_INT_DISCONNECT |
                             USB_INT_SESSION_START |  USB_INT_BABBLE |
                             USB_INT_CONNECT | USB_INT_RESUME |
                             USB_INT_SUSPEND | USB_INT_HOST_OUT |
                             USB_INT_HOST_IN | USB_INT_EP0 |
                             USB_INT_VBUS_ERR));

    //
    // Enable the USB interrupt.
    //
    IntEnable(INT_USB0);

    //
    // Turn on session request to enable ID pin checking.
    //
    USBOTGSessionRequest(USB0_BASE, true);

    //
    // Attach the device using the soft connect.
    //
    USBDevConnect(USB0_BASE);

    //
    // USB power enable out is active high.
    //
    USBHostPwrFaultConfig(USB0_BASE,
                          USB_HOST_PWRFLT_EP_NONE | USB_HOST_PWREN_HIGH);
}

//*****************************************************************************
//
//! Returns the USB controller to the default mode when in dual mode operation.
//!
//! \param ulIndex specifies the USB controller whose dual mode operation is to
//! be ended.  This parameter must be set to 0.
//!
//! Applications using both host and device modes may call this function to
//! disable interrupts in preparation for shutdown or a change of operating
//! mode.
//!
//! \return None.
//
//*****************************************************************************
void
USBDualModeTerm(unsigned long ulIndex)
{
    //
    // Disable the USB interrupt.
    //
    IntDisable(INT_USB0);
    USBIntDisable(USB0_BASE, USB_INT_RESET | USB_INT_DISCONNECT |
                  USB_INT_SESSION_START |  USB_INT_BABBLE | USB_INT_CONNECT |
                  USB_INT_RESUME | USB_INT_SUSPEND | USB_INT_HOST_OUT |
                  USB_INT_HOST_IN | USB_INT_EP0 | USB_INT_VBUS_ERR);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
