//*****************************************************************************
//
// usbenum.c - Enumeration code to handle all endpoint zero traffic.
//
// Copyright (c) 2007-2008 Luminary Micro, Inc.  All rights reserved.
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

#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "usb.h"
#include "interrupt.h"
#include "debug.h"
#include "usblib.h"
#include "usbdevice.h"

//*****************************************************************************
//
// Local functions prototypes.
//
//*****************************************************************************
static void USBDGetStatus(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDClearFeature(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDSetFeature(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDSetAddress(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDGetDescriptor(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDSetDescriptor(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDGetConfiguration(unsigned long ulIndex,
                                 tUSBRequest *pUSBRequest);
static void USBDSetConfiguration(unsigned long ulIndex,
                                 tUSBRequest *pUSBRequest);
static void USBDGetInterface(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDSetInterface(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDSyncFrame(unsigned long ulIndex, tUSBRequest *pUSBRequest);
static void USBDEP0StateTx(unsigned long ulIndex);

//*****************************************************************************
//
//! \addtogroup device_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// This structure holds the full state for the device enumeration.
//
//*****************************************************************************
typedef struct
{
    //
    // The devices current address, this also has a change pending bit in the
    // MSB of this value specified by DEV_ADDR_PENDING.
    //
    volatile unsigned long ulDevAddress;

    //
    // This holds the current active configuration for this device.
    //
    unsigned long ulConfiguration;

    //
    // This holds the configuration id that will take effect after a reset.
    //
    unsigned long ulDefaultConfiguration;

    //
    // This holds the current alternate interface for this device.
    //
    unsigned long ulAltSetting;

    //
    // This is the pointer to the current data being sent out or received
    // on endpoint zero.
    //
    unsigned char *pEP0Data;

    //
    // This is the number of bytes that remain to be sent from or received
    // into the g_sUSBDeviceState.pEP0Data data buffer.
    //
    volatile unsigned long ulEP0DataRemain;

    //
    // The amount of data being sent/received due to a custom request.
    //
    unsigned long ulOUTDataSize;

    //
    // Holds the current device status.
    //
    unsigned char ucStatus;

    //
    // Holds the endpoint status for the HALT condition.
    //
    unsigned char ucHalt[3];
}
tDeviceState;

//*****************************************************************************
//
// The states for endpoint zero during enumeration.
//
//*****************************************************************************
typedef enum
{
    //
    // The USB device is waiting on a request from the host controller on
    // endpoint zero.
    //
    USB_STATE_IDLE,

    //
    // The USB device is sending data back to the host due to an IN request.
    //
    USB_STATE_TX,

    //
    // The USB device is receiving data from the host due to an OUT
    // request from the host.
    //
    USB_STATE_RX,

    //
    // The USB device has completed the IN or OUT request and is now waiting
    // for the host to acknowledge the end of the IN/OUT transaction.  This
    // is the status phase for a USB control transaction.
    //
    USB_STATE_STATUS,

    //
    // This endpoint has signaled a stall condition and is waiting for the
    // stall to be acknowledged by the host controller.
    //
    USB_STATE_STALL
}
tEP0State;

//*****************************************************************************
//
// Define the max packet size for endpoint zero.
//
//*****************************************************************************
#define EP0_MAX_PACKET_SIZE     64

//*****************************************************************************
//
// This is a flag used with g_sUSBDeviceState.ulDevAddress to indicate that a
// device address change is pending.
//
//*****************************************************************************
#define DEV_ADDR_PENDING        0x80000000

//*****************************************************************************
//
// This label defines the default configuration number to use after a bus
// reset.  This may be overridden by calling USBDCDSetDefaultConfiguration()
// during processing of the device reset handler if required.
//
//*****************************************************************************
#define DEFAULT_CONFIG_ID       1

//*****************************************************************************
//
// The buffer for reading data coming into EP0
//
//*****************************************************************************
static unsigned char g_pucDataBufferIn[EP0_MAX_PACKET_SIZE];

//*****************************************************************************
//
// The pointer to the device's information structure as passed on a call to
// USBDCDInit().
//
//*****************************************************************************
tDeviceInfo *g_psUSBDeviceInfo;

//*****************************************************************************
//
// This global holds the current state information for the USB device.
//
//*****************************************************************************
static volatile tDeviceState g_sUSBDeviceState =
{
    0, DEFAULT_CONFIG_ID, DEFAULT_CONFIG_ID, 0, 0, 0, 0
};

//*****************************************************************************
//
// This global holds the current state of endpoint zero.
//
//*****************************************************************************
static volatile tEP0State g_eUSBDEP0State = USB_STATE_IDLE;

//*****************************************************************************
//
// Function table to handle standard requests.
//
//*****************************************************************************
static const tStdRequest g_psUSBDStdRequests[] =
{
    USBDGetStatus,
    USBDClearFeature,
    0,
    USBDSetFeature,
    0,
    USBDSetAddress,
    USBDGetDescriptor,
    USBDSetDescriptor,
    USBDGetConfiguration,
    USBDSetConfiguration,
    USBDGetInterface,
    USBDSetInterface,
    USBDSyncFrame
};

//*****************************************************************************
//
// Functions accessible by USBLIB clients.
//
//*****************************************************************************

//*****************************************************************************
//
//! Initialize the USB library device control driver for a given hardware
//! controller.
//!
//! \param ulIndex is the index of the USB controller which is to be
//! initialized.
//! \param psDevice is a pointer to a structure containing information that
//! the USB library requires to support operation of this application's
//! device.  The structure contains event handler callbacks and pointers to the
//! various standard descriptors that the device wishes to publish to the
//! host.
//!
//! This function must be called by any application which wishes to operate
//! as a USB device.  It initializes the USB device control driver for the
//! given controller and saves the device information for future use.  Prior to
//! returning from this function, the device is connected to the USB bus.
//! Following return, the caller can expect to receive a callback to the
//! supplied <tt>pfnResetHandler</tt> function when a host connects to the
//! device.
//!
//! The device information structure passed in \e psDevice must remain
//! unchanged between this call and any matching call to USBDCDTerm() since
//! it is not copied by the USB library.
//!
//! \return None.
//
//*****************************************************************************
void
USBDCDInit(unsigned long ulIndex, tDeviceInfo *psDevice)
{
    //
    // Check the arguments.
    //
    ASSERT(ulIndex == 0);
    ASSERT(psDevice != 0);

    //
    // Make sure someone else has not already claimed this controller.
    //
    ASSERT(g_psUSBDeviceInfo == 0);

    //
    // Remember the device information pointer.
    //
    g_psUSBDeviceInfo = psDevice;

    //
    // Ask for the interrupt status.  As a side effect, this clears all pending
    // USB interrupts.
    //
    USBIntStatus(USB0_BASE);

    //
    // Enable USB Interrupts.
    //
    USBIntEnable(USB0_BASE, USB_INT_RESET | USB_INT_DISCONNECT |
                 USB_INT_RESUME | USB_INT_SUSPEND | USB_INT_DEV_OUT |
                 USB_INT_DEV_IN);

    //
    // Attach the device using the soft connect.
    //
    USBDevConnect(USB0_BASE);

    //
    // Enable the USB interrupt.
    //
    IntEnable(INT_USB0);
}

//*****************************************************************************
//
//! Free the USB library device control driver for a given hardware controller.
//!
//! \param ulIndex is the index of the USB controller which is to be
//! freed.
//!
//! This function should be called by an application if it no longer requires
//! the use of a given USB controller to support its operation as a USB device.
//! It frees the controller for use by another client.
//!
//! It is the caller's responsibility to remove its device from the USB bus
//! prior to calling this function.
//!
//! \return None.
//
//*****************************************************************************
void
USBDCDTerm(unsigned long ulIndex)
{
    //
    // Check the arguments.
    //
    ASSERT(ulIndex == 0);

    g_psUSBDeviceInfo = (tDeviceInfo *)0;

    //
    // Disable the USB interrupts.
    //
    IntDisable(INT_USB0);
    USBIntDisable(USB0_BASE, USB_INT_RESET | USB_INT_DISCONNECT |
                  USB_INT_RESUME | USB_INT_SUSPEND | USB_INT_DEV_OUT |
                  USB_INT_DEV_IN);

    //
    // Clear any pending interrupts.
    //
    USBIntStatus(USB0_BASE);
}

//*****************************************************************************
//
//! This function starts the request for data from the host on endpoint zero.
//!
//! \param ulIndex is the index of the USB controller from which the data
//! is being requested.
//! \param pucData is a pointer to the buffer to fill with data from the USB
//! host.
//! \param ulSize is the size of the buffer or data to return from the USB
//! host.
//!
//! This function handles retrieving data from the host when a custom command
//! has been issued on endpoint zero.  If the application needs notification
//! when the data has been received,
//! <tt>tDeviceInfo.sCallbacks.pfnDataReceived</tt> should contain valid
//! function pointer.  In nearly all cases this is necessary because the caller
//! of this function would likely need to know that the data requested was
//! received.
//!
//! \return None.
//
//*****************************************************************************
void
USBDCDRequestDataEP0(unsigned long ulIndex, unsigned char *pucData,
                     unsigned long ulSize)
{
    ASSERT(ulIndex == 0);

    //
    // Enter the RX state on end point 0.
    //
    g_eUSBDEP0State = USB_STATE_RX;

    //
    // Save the pointer to the data.
    //
    g_sUSBDeviceState.pEP0Data = pucData;

    //
    // Location to save the current number of bytes received.
    //
    g_sUSBDeviceState.ulOUTDataSize = ulSize;

    //
    // Bytes remaining to be received.
    //
    g_sUSBDeviceState.ulEP0DataRemain = ulSize;
}

//*****************************************************************************
//
//! This function requests transfer of data to the host on endpoint zero.
//!
//! \param ulIndex is the index of the USB controller which is to be used to
//! send the data.
//! \param pucData is a pointer to the buffer to send via endpoint zero.
//! \param ulSize is the amount of data to send in bytes.
//!
//! This function handles sending data to the host when a custom command is
//! issued or non-standard descriptor has been requested on endpoint zero.  If
//! the application needs notification when this is complete,
//! <tt>tDeviceInfo.sCallbacks.pfnDataSent</tt> should contain a valid function
//! pointer.  This callback could be used to free up the buffer passed into
//! this function in the \e pucData parameter.  The contents of the \e pucData
//! buffer must remain unchanged until the <tt>pfnDataSent</tt> callback is
//! received.
//!
//! \return None.
//
//*****************************************************************************
void
USBDCDSendDataEP0(unsigned long ulIndex, unsigned char *pucData,
                  unsigned long ulSize)
{
    ASSERT(ulIndex == 0);

    //
    // Return the externally provided device descriptor.
    //
    g_sUSBDeviceState.pEP0Data = pucData;

    //
    // The size of the device descriptor is in the first byte.
    //
    g_sUSBDeviceState.ulEP0DataRemain = ulSize;

    //
    // Save the total size of the data sent.
    //
    g_sUSBDeviceState.ulOUTDataSize = ulSize;

    //
    // Now in the transmit data state.
    //
    USBDEP0StateTx(0);
}

//*****************************************************************************
//
//! This function sets the default configuration for the device.
//!
//! \param ulIndex is the index of the USB controller whose default
//! configuration is to be set.
//! \param ulDefaultConfig is the configuration identifier (byte 6 of the
//! standard configuration descriptor) which is to be presented to the host
//! as the default configuration in cases where the config descriptor is
//! queried prior to any specific configuration being set.
//!
//! This function allows a device to override the default configuration
//! descriptor that will be returned to a host whenever it is queried prior
//! to a specific configuration having been set.  The parameter passed must
//! equal one of the configuration identifiers found in the
//! <tt>ppConfigDescriptors</tt> array for the device.
//!
//! If this function is not called, the USB library will return the first
//! configuration in the <tt>ppConfigDescriptors</tt> array as the default
//! configuration.
//!
//! \note The USB device stack assumes that the configuration IDs (byte 6 of
//! the config descriptor, <tt>bConfigurationValue</tt>) stored within the
//! configuration descriptor array, <tt>ppConfigDescriptors</tt>,
//! are equal to the array index + 1.  In other words, the first entry in the
//! array must contain a descriptor with <tt>bConfigurationValue</tt> 1, the
//! second must have <tt>bConfigurationValue</tt> 2 and so on.
//!
//! \return None.
//
//*****************************************************************************
void
USBDCDSetDefaultConfiguration(unsigned long ulIndex,
                              unsigned long ulDefaultConfig)
{
    ASSERT(ulIndex == 0);

    g_sUSBDeviceState.ulDefaultConfiguration = ulDefaultConfig;
}

//*****************************************************************************
//
//! This function generates a stall condition on endpoint zero.
//!
//! \param ulIndex is the index of the USB controller whose endpoint zero is to
//! be stalled.
//!
//! This function is typically called to signal an error condition to the host
//! when an unsupported request is received by the device.  It should be
//! called from within the callback itself (in interrupt context) and not
//! deferred until later since it affects the operation of the endpoint zero
//! state machine in the USB library.
//!
//! \return None.
//
//*****************************************************************************
void
USBDCDStallEP0(unsigned long ulIndex)
{
    ASSERT(ulIndex == 0);

    //
    // Stall the endpoint in question.
    //
    USBDevEndpointStall(USB0_BASE, USB_EP_0, USB_EP_DEV_OUT);

    //
    // Enter the stalled state.
    //
    g_eUSBDEP0State = USB_STATE_STALL;
}

//*****************************************************************************
//
// Internal Functions, not to be called by applications
//
//*****************************************************************************

//*****************************************************************************
//
// This internal function reads a request data packet and dispatches it to
// either a standard request handler or the registered device request
// callback depending upon the request type.
//
// \return None.
//
//*****************************************************************************
static void
USBDReadAndDispatchRequest(void)
{
    unsigned long ulSize;
    tUSBRequest *pRequest;

    //
    // Cast the buffer to a request structure.
    //
    pRequest = (tUSBRequest *)g_pucDataBufferIn;

    //
    // Set the buffer size.
    //
    ulSize = EP0_MAX_PACKET_SIZE;

    //
    // Get the data from the USB controller end point 0.
    //
    USBEndpointDataGet(USB0_BASE,
                       USB_EP_0,
                       g_pucDataBufferIn,
                       &ulSize);
    //
    // See if this is a standard request or not.
    //
    if((pRequest->bmRequestType & USB_RTYPE_TYPE_M) != USB_RTYPE_STANDARD)
    {
        //
        // Since this is not a standard request, see if there is
        // an external handler present.
        //
        if(g_psUSBDeviceInfo->sCallbacks.pfnRequestHandler)
        {
            g_psUSBDeviceInfo->sCallbacks.pfnRequestHandler(0, pRequest);
        }
        else
        {
            //
            // If there is no handler then stall this request.
            //
            USBDCDStallEP0(0);
        }
    }
    else
    {
        //
        // Assure that the jump table is not out of bounds.
        //
        if ((pRequest->bRequest <
            (sizeof(g_psUSBDStdRequests) / sizeof(tStdRequest))) &&
            (g_psUSBDStdRequests[pRequest->bRequest] != 0))
        {
            //
            // Jump table to the appropriate handler.
            //
            g_psUSBDStdRequests[pRequest->bRequest](0, pRequest);
        }
        else
        {
            //
            // If there is no handler then stall this request.
            //
            USBDCDStallEP0(0);
        }
    }
}

//*****************************************************************************
//
// This is interrupt handler for endpoint zero.
//
// This function handles all interrupts on endpoint zero in order to maintain
// the state needed for the control endpoint on endpoint zero.  In order to
// successfully enumerate and handle all USB standard requests, all requests
// on endpoint zero must pass through this function.  The endpoint has the
// following states: \b USB_STATE_IDLE, \b USB_STATE_TX, \b USB_STATE_RX,
// \b USB_STATE_STALL, and \b USB_STATE_STATUS.  In the \b USB_STATE_IDLE
// state the USB controller has not received the start of a request, and once
// it does receive the data for the request it will either enter the
// \b USB_STATE_TX, \b USB_STATE_RX, or \b USB_STATE_STALL depending on the
// command.  If the controller enters the \b USB_STATE_TX or \b USB_STATE_RX
// then once all data has been sent or received, it must pass through the
// \b USB_STATE_STATUS state to allow the host to acknowledge completion of
// the request.  The \b USB_STATE_STALL is entered from \b USB_STATE_IDLE in
// the event that the USB request was not valid.  Both the \b USB_STATE_STALL
// and \b USB_STATE_STATUS are transitional states that return to the
// \b USB_STATE_IDLE state.
//
// \return None.
//
// USB_STATE_IDLE -*--> USB_STATE_TX -*-> USB_STATE_STATUS -*->USB_STATE_IDLE
//                 |                  |                     |
//                 |--> USB_STATE_RX -                      |
//                 |                                        |
//                 |--> USB_STATE_STALL ---------->---------
//
//  ----------------------------------------------------------------
// | Current State       | State 0           | State 1              |
// | --------------------|-------------------|----------------------
// | USB_STATE_IDLE      | USB_STATE_TX/RX   | USB_STATE_STALL      |
// | USB_STATE_TX        | USB_STATE_STATUS  |                      |
// | USB_STATE_RX        | USB_STATE_STATUS  |                      |
// | USB_STATE_STATUS    | USB_STATE_IDLE    |                      |
// | USB_STATE_STALL     | USB_STATE_IDLE    |                      |
//  ----------------------------------------------------------------
//
//*****************************************************************************
void
USBDeviceEnumHandler(void)
{
    unsigned long ulEPStatus;

    //
    // Get the end point 0 status.
    //
    ulEPStatus = USBEndpointStatus(USB0_BASE, USB_EP_0);

    switch(g_eUSBDEP0State)
    {
        //
        // Handle the status state, this is a transitory state from
        // USB_STATE_TX or USB_STATE_RX back to USB_STATE_IDLE.
        //
        case USB_STATE_STATUS:
        {
            //
            // Just go back to the idle state.
            //
            g_eUSBDEP0State = USB_STATE_IDLE;

            //
            // If there is a pending address change then set the address.
            //
            if(g_sUSBDeviceState.ulDevAddress & DEV_ADDR_PENDING)
            {
                //
                // Clear the pending address change and set the address.
                //
                g_sUSBDeviceState.ulDevAddress &= ~DEV_ADDR_PENDING;
                USBDevAddrSet(USB0_BASE, g_sUSBDeviceState.ulDevAddress);
            }

            //
            // If a new packet is already pending, we need to read it
            // and handle whatever request it contains.
            //
            if(ulEPStatus & USB_DEV_EP0_OUT_PKTRDY)
            {
                //
                // Process the newly arrived packet.
                //
                USBDReadAndDispatchRequest();
            }
            break;
        }

        //
        // In the IDLE state the code is waiting to receive data from the host.
        //
        case USB_STATE_IDLE:
        {
            //
            // Is there a packet waiting for us?
            //
            if(ulEPStatus & USB_DEV_EP0_OUT_PKTRDY)
            {
                //
                // Yes - process it.
                //
                USBDReadAndDispatchRequest();
            }
            break;
        }

        //
        // Data is still being sent to the host so handle this in the
        // EP0StateTx() function.
        //
        case USB_STATE_TX:
        {
            USBDEP0StateTx(0);
            break;
        }

        //
        // Handle the receive state for commands that are receiving data on
        // endpoint zero.
        //
        case USB_STATE_RX:
        {
            unsigned long ulDataSize;

            //
            // Set the number of bytes to get out of this next packet.
            //
            if(g_sUSBDeviceState.ulEP0DataRemain > EP0_MAX_PACKET_SIZE)
            {
                //
                // Don't send more than EP0_MAX_PACKET_SIZE bytes.
                //
                ulDataSize = EP0_MAX_PACKET_SIZE;
            }
            else
            {
                //
                // There was space so send the remaining bytes.
                //
                ulDataSize = g_sUSBDeviceState.ulEP0DataRemain;
            }

            //
            // Get the data from the USB controller end point 0.
            //
            USBEndpointDataGet(USB0_BASE, USB_EP_0, g_sUSBDeviceState.pEP0Data,
                               &ulDataSize);

            //
            // If there we not more that EP0_MAX_PACKET_SIZE or more bytes
            // remaining then this transfer is complete.  If there were exactly
            // EP0_MAX_PACKET_SIZE remaining then there still needs to be
            // null packet sent before this is complete.
            //
            if(g_sUSBDeviceState.ulEP0DataRemain < EP0_MAX_PACKET_SIZE)
            {
                //
                // Need to ack the data on end point 0 in this case
                // without setting data end.
                //
                USBDevEndpointDataAck(USB0_BASE, USB_EP_0, true);

                //
                // Return to the idle state.
                //
                g_eUSBDEP0State =  USB_STATE_IDLE;

                //
                // If there is a receive callback then call it.
                //
                if((g_psUSBDeviceInfo->sCallbacks.pfnDataReceived) &&
                   (g_sUSBDeviceState.ulOUTDataSize != 0))
                {
                    //
                    // Call the custom receive handler to handle the data
                    // that was received.
                    //
                    g_psUSBDeviceInfo->sCallbacks.pfnDataReceived(0,
                        g_sUSBDeviceState.ulOUTDataSize);

                    //
                    // Indicate that there is no longer any data being waited
                    // on.
                    //
                    g_sUSBDeviceState.ulOUTDataSize = 0;
                }
            }
            else
            {
                //
                // Need to ack the data on end point 0 in this case
                // without setting data end.
                //
                USBDevEndpointDataAck(USB0_BASE, USB_EP_0, false);
            }

            //
            // Advance the pointer.
            //
            g_sUSBDeviceState.pEP0Data += ulDataSize;

            //
            // Decrement the number of bytes that are being waited on.
            //
            g_sUSBDeviceState.ulEP0DataRemain -= ulDataSize;

            break;
        }
        //
        // The device stalled endpoint zero so check if the stall needs to be
        // cleared once it has been successfully sent.
        //
        case USB_STATE_STALL:
        {
            //
            // If we sent a stall then acknowledge this interrupt.
            //
            if(ulEPStatus & USB_DEV_EP0_SENT_STALL)
            {
                //
                // Clear the Setup End condition.
                //
                USBDevEndpointStatusClear(USB0_BASE, USB_EP_0,
                                          USB_DEV_EP0_SENT_STALL);

                //
                // Reset the global end point 0 state to IDLE.
                //
                g_eUSBDEP0State = USB_STATE_IDLE;

            }
            break;
        }
        //
        // Halt on an unknown state, but only in DEBUG mode builds.
        //
        default:
        {
            ASSERT(0);
        }
    }
}

//*****************************************************************************
//
// This function handles bus reset notifications.
//
// This function is called from the low level USB interrupt handler whenever
// a bus reset is detected.  It performs tidy-up as required and resets the
// configuration back to defaults in preparation for descriptor queries from
// the host.
//
// \return None.
//
//*****************************************************************************
void
USBDeviceEnumResetHandler(void)
{
    //
    // Call the device dependent code to indicate a bus reset has occured.
    //
    if(g_psUSBDeviceInfo->sCallbacks.pfnResetHandler)
    {
        g_psUSBDeviceInfo->sCallbacks.pfnResetHandler(0);
    }

    //
    // Reset the default configuration identifier and alternate function
    // selections.
    //
    g_sUSBDeviceState.ulConfiguration =
        g_sUSBDeviceState.ulDefaultConfiguration;
    g_sUSBDeviceState.ulAltSetting = 0;
}

//*****************************************************************************
//
// This function handles the GET_STATUS standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the request type and endpoint number if endpoint
// status is requested.
//
// This function handles responses to a Get Status request from the host
// controller.  A status request can be for the device, an interface or an
// endpoint.  If any other type of request is made this function will cause
// a stall condition to indicate that the command is not supported.  The
// \e pUSBRequest structure holds the type of the request in the
// bmRequestType field.  If the type indicates that this is a request for an
// endpoint's status, then the wIndex field holds the endpoint number.
//
// \return None.
//
//*****************************************************************************
static void
USBDGetStatus(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    unsigned long ulData;

    ASSERT(ulIndex == 0);

    //
    // Determine what type of status was requested.
    //
    switch(pUSBRequest->bmRequestType & USB_RTYPE_RECIPIENT_M)
    {
        //
        // This was a Device Status request.
        //
        case USB_RTYPE_DEVICE:
        {
            //
            // Return the current status for the device.
            //
            ulData = g_sUSBDeviceState.ucStatus;

            break;
        }

        //
        // This was a Interface status request.
        //
        case USB_RTYPE_INTERFACE:
        {
            //
            // Interface status always returns 0.
            //
            ulData = 0;

            break;
        }

        //
        // This was an endpoint status request.
        //
        case USB_RTYPE_ENDPOINT:
        {
            //
            // Check if this was a valid endpoint request.
            //
            if((pUSBRequest->wIndex == 0) || (pUSBRequest->wIndex > 3))
            {
                USBDCDStallEP0(0);
                return;
            }
            else
            {
                //
                // Get the current halt status for this endpoint.
                //
                ulData = g_sUSBDeviceState.ucHalt[pUSBRequest->wIndex];
            }
            break;
        }

        //
        // This was an unknown request.
        //
        default:
        {
            //
            // Anything else causes a stall condition to indicate that the
            // command was not supported.
            //
            USBDCDStallEP0(0);
            return;
        }
    }

    //
    // Return the current device status.
    //
    USBDCDSendDataEP0(0, (unsigned char *)&ulData, 2);
}

//*****************************************************************************
//
// This function handles the CLEAR_FEATURE standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the options for the Clear Feature USB request.
//
// This function handles device or endpoint clear feature requests.  The
// \e pUSBRequest structure holds the type of the request in the bmRequestType
// field and the feature is held in the wValue field.  For device, the only
// clearable feature is the Remote Wake feature.  This device request
// should only be made if the descriptor indicates that Remote Wake is
// implemented by the device.  For endpoint requests the only clearable
// feature is the ability to clear a halt on a given endpoint.  If any other
// requests are made, then the device will stall the request to indicate to
// the host that the command was not supported.
//
// \return None.
//
//*****************************************************************************
static void
USBDClearFeature(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    ASSERT(ulIndex == 0);

    //
    // Determine what type of status was requested.
    //
    switch(pUSBRequest->bmRequestType & USB_RTYPE_RECIPIENT_M)
    {
        //
        // This is a clear feature request at the device level.
        //
        case USB_RTYPE_DEVICE:
        {
            //
            // Only remote wake is clearable by this function.
            //
            if(USB_FEATURE_REMOTE_WAKE & pUSBRequest->wValue)
            {
                //
                // Clear the remote wake up state.
                //
                g_sUSBDeviceState.ucStatus &= ~USB_STATUS_REMOTE_WAKE;
            }
            else
            {
                USBDCDStallEP0(0);
            }
            break;
        }

        //
        // This is a clear feature request at the endpoint level.
        //
        case USB_RTYPE_ENDPOINT:
        {
            //
            // Not a valid endpoint.
            //
            if((pUSBRequest->wIndex == 0) || (pUSBRequest->wIndex > 3))
            {
                USBDCDStallEP0(0);
            }
            else if(USB_FEATURE_EP_HALT & pUSBRequest->wValue)
            {
                //
                // Clear the halt condition on this endpoint.
                //
                g_sUSBDeviceState.ucHalt[pUSBRequest->wIndex] = 0;
            }
            else
            {
                USBDCDStallEP0(0);
            }
            break;
        }

        //
        // This is an unknown request.
        //
        default:
        {
            USBDCDStallEP0(0);
            return;
        }
    }
}

//*****************************************************************************
//
// This function handles the SET_FEATURE standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the feature in the wValue field of the USB
// request.
//
// This function handles device or endpoint set feature requests.  The
// \e pUSBRequest structure holds the type of the request in the bmRequestType
// field and the feature is held in the wValue field.  For device, the only
// settable feature is the Remote Wake feature.  This device request
// should only be made if the descriptor indicates that Remote Wake is
// implemented by the device.  For endpoint requests the only settable feature
// is the ability to issue a halt on a given endpoint.  If any other requests
// are made, then the device will stall the request to indicate to the host
// that the command was not supported.
//
// \return None.
//
//*****************************************************************************
static void
USBDSetFeature(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    ASSERT(ulIndex == 0);

    //
    // Determine what type of status was requested.
    //
    switch(pUSBRequest->bmRequestType & USB_RTYPE_RECIPIENT_M)
    {
        //
        // This is a set feature request at the device level.
        //
        case USB_RTYPE_DEVICE:
        {
            //
            // Only remote wake is setable by this function.
            //
            if(USB_FEATURE_REMOTE_WAKE & pUSBRequest->wValue)
            {
                //
                // Clear the remote wakeup state.
                //
                g_sUSBDeviceState.ucStatus |= USB_STATUS_REMOTE_WAKE;
            }
            else
            {
                USBDCDStallEP0(0);
            }
            break;
        }

        //
        // This is a set feature request at the endpoint level.
        //
        case USB_RTYPE_ENDPOINT:
        {
            //
            // Not a valid endpoint.
            //
            if((pUSBRequest->wIndex == 0) || (pUSBRequest->wIndex > 3))
            {
                USBDCDStallEP0(0);
            }
            //
            // Only the Halt feature is settable.
            //
            else if(USB_FEATURE_EP_HALT & pUSBRequest->wValue)
            {
                //
                // Clear the halt condition on this endpoint.
                //
                g_sUSBDeviceState.ucHalt[pUSBRequest->wIndex] = 1;
            }
            else
            {
                //
                // All other requests are not supported.
                //
                USBDCDStallEP0(0);
            }
            break;
        }

        //
        // This is an unknown request.
        //
        default:
        {
            USBDCDStallEP0(0);
            return;
        }
    }
}

//*****************************************************************************
//
// This function handles the SET_ADDRESS standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the new address to use in the wValue field of the
// USB request.
//
// This function is called to handle the change of address request from the
// host controller.  This can only start the sequence as the host must
// acknowledge that the device has changed address.  Thus this function sets
// the address change as pending until the status phase of the request has
// been completed successfully.  This prevents the devices address from
// changing and not properly responding to the status phase.
//
// \return None.
//
//*****************************************************************************
static void
USBDSetAddress(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    ASSERT(ulIndex == 0);

    //
    // The data needs to be acknowledged on end point 0 without setting data
    // end because there is no data coming.
    //
    USBDevEndpointDataAck(USB0_BASE, USB_EP_0, true);

    //
    // Save the device address as we cannot change address until the status
    // phase is complete.
    //
    g_sUSBDeviceState.ulDevAddress = pUSBRequest->wValue | DEV_ADDR_PENDING;

    //
    // Transition directly to the status state since there is no data phase
    // for this request.
    //
    g_eUSBDEP0State = USB_STATE_STATUS;
}

//*****************************************************************************
//
// This function handles the GET_DESCRIPTOR standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the data for this request.
//
// This function will return most of the descriptors requested by the host
// controller.  The descriptor specified by \e
// g_psUSBDeviceInfo->pDeviceDescriptor will be returned when the device
// descriptor is requested.  If a request for a specific configuration
// descriptor is made, then the appropriate descriptor from the \e
// g_pConfigDescriptors will be returned.  When a request for a string
// descriptor is made, the appropriate string from the
// \e g_psUSBDeviceInfo->pStringDescriptors will be returned.  If the \e
// g_psUSBDeviceInfo->sCallbacks.GetDescriptor is specified it will be called
// to handle the request.  In this case it must call the USBDCDSendDataEP0()
// function to send the data to the host controller.  If the callback is not
// specified, and the descriptor request is not for a device, configuration,
// or string descriptor then this function will stall the request to indicate
// that the request was not supported by the device.
//
// \return None.
//
//*****************************************************************************
static void
USBDGetDescriptor(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    ASSERT(ulIndex == 0);

    //
    // Need to ack the data on end point 0 in this case without
    // setting data end.
    //
    USBDevEndpointDataAck(USB0_BASE, USB_EP_0, false);

    switch(pUSBRequest->wValue >> 8)
    {
        //
        // This request was for a device descriptor.
        //
        case USB_DTYPE_DEVICE:
        {
            //
            // Return the externally provided device descriptor.
            //
            g_sUSBDeviceState.pEP0Data =
                (unsigned char *)g_psUSBDeviceInfo->pDeviceDescriptor;

            //
            // The size of the device descriptor is in the first byte.
            //
            g_sUSBDeviceState.ulEP0DataRemain =
                g_psUSBDeviceInfo->pDeviceDescriptor[0];

            break;
        }

        //
        // This request was for a configuration descriptor.
        //
        case USB_DTYPE_CONFIGURATION:
        {
            //
            // Return the extrenally specified configuration descriptor.
            //
            g_sUSBDeviceState.pEP0Data =
                (unsigned char *)g_psUSBDeviceInfo->ppConfigDescriptors[
                                        g_sUSBDeviceState.ulConfiguration - 1];

            //
            // The total size of the configuration descriptor is in bytes
            // 3 and 4 so access them as a 16 bit value.
            //
            g_sUSBDeviceState.ulEP0DataRemain =
                ((unsigned short *)g_sUSBDeviceState.pEP0Data)[1];

            break;
        }

        //
        // This request was for a string descriptor.
        //
        case USB_DTYPE_STRING:
        {
            unsigned long ulIndex;

            //
            // Extract the string index from the USB request.
            //
            ulIndex = pUSBRequest->wValue & 0xff;

            //
            // If the string index was too large then stall the request to
            // indicate that the request was not valid.
            //
            if(ulIndex > g_psUSBDeviceInfo->ulNumStringDescriptors)
            {
                USBDCDStallEP0(0);
                break;
            }

            //
            // Return the extrenally specified configuration descriptor.
            //
            g_sUSBDeviceState.pEP0Data =
              (unsigned char *)g_psUSBDeviceInfo->ppStringDescriptors[ulIndex];

            //
            // The total size of a string descriptor is in byte 0.
            //
            g_sUSBDeviceState.ulEP0DataRemain =
                g_psUSBDeviceInfo->ppStringDescriptors[ulIndex][0];

            break;
        }

        //
        // Any other request is not handled by the default enumeration handler
        // so see if it needs to be passed on to another handler.
        //
        default:
        {
            //
            // If there is a handler for requests that are not handled then
            // call it.
            //
            if(g_psUSBDeviceInfo->sCallbacks.pfnGetDescriptor)
            {
                g_psUSBDeviceInfo->sCallbacks.pfnGetDescriptor(0, pUSBRequest);
            }
            else
            {
                //
                // Whatever this was this handler does not understand it so
                // just stall the request.
                //
                USBDCDStallEP0(0);
            }

            //
            // Reset the data pointer.
            //
            g_sUSBDeviceState.pEP0Data = 0;

            break;
        }
    }

    //
    // If this request has data to send, then send it.
    //
    if(g_sUSBDeviceState.pEP0Data)
    {
        //
        // If there is more data to send than is requested then just
        // send the requested amount of data.
        //
        if(g_sUSBDeviceState.ulEP0DataRemain > pUSBRequest->wLength)
        {
            g_sUSBDeviceState.ulEP0DataRemain = pUSBRequest->wLength;
        }

        //
        // Now in the transmit data state.
        //
        USBDEP0StateTx(0);
    }
}

//*****************************************************************************
//
// This function handles the SET_DESCRIPTOR standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the data for this request.
//
// This function currently is not supported and will respond with a Stall
// to indicate that this command is not supported by the device.
//
// \return None.
//
//*****************************************************************************
static void
USBDSetDescriptor(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    ASSERT(ulIndex == 0);

    //
    // This function is not handled by default.
    //
    USBDCDStallEP0(0);
}

//*****************************************************************************
//
// This function handles the GET_CONFIGURATION standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the data for this request.
//
// This function responds to a host request to return the current
// configuration of the USB device.  The function will send the configuration
// response to the host and return.  This value will either be 0 or the last
// value received from a call to SetConfiguration().
//
// \return None.
//
//*****************************************************************************
static void
USBDGetConfiguration(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    unsigned long ulValue;

    ASSERT(ulIndex == 0);

    //
    // If we still have an address pending then the device is still not
    // configured.
    //
    if(g_sUSBDeviceState.ulDevAddress & DEV_ADDR_PENDING)
    {
        ulValue = 0;
    }
    else
    {
        ulValue = g_sUSBDeviceState.ulConfiguration;
    }

    //
    // Send the single byte response.
    //
    USBDCDSendDataEP0(0, (unsigned char *)&ulValue, 1);
}

//*****************************************************************************
//
// This function handles the SET_CONFIGURATION standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the data for this request.
//
// This function responds to a host request to change the current
// configuration of the USB device.  The actual configuration number is taken
// from the structure passed in via \e pUSBRequest.  This number should be one
// of the configurations that was specified in the descriptors.  If the
// \e ConfigChange callback is specified in \e g_psUSBDeviceInfo->sCallbacks,
// it will be called so that the application can respond to a change in
// configuration.
//
// \return None.
//
//*****************************************************************************
static void
USBDSetConfiguration(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    ASSERT(ulIndex == 0);

    //
    // Cannot set the configuration to one that does not exist so check the
    // enumeration structure to see how many valid configurations are present.
    //
    if(pUSBRequest->wValue > g_psUSBDeviceInfo->pDeviceDescriptor[17])
    {
        //
        // The passed configuration number is not valid.  Stall the endpoint to
        // signal the error to the host.
        //
        USBDCDStallEP0(0);
    }
    else
    {
        //
        // Need to ack the data on end point 0 in this case without
        // setting data end.
        //
        USBDevEndpointDataAck(USB0_BASE, USB_EP_0, false);

        //
        // Save the configuration.
        //
        g_sUSBDeviceState.ulConfiguration = pUSBRequest->wValue;

        //
        // If there is a configuration change callback then call it.
        //
        if(g_psUSBDeviceInfo->sCallbacks.pfnConfigChange)
        {
            g_psUSBDeviceInfo->sCallbacks.pfnConfigChange(0,
                                            g_sUSBDeviceState.ulConfiguration);
        }
    }
}

//*****************************************************************************
//
// This function handles the GET_INTERFACE standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the data for this request.
//
// This function is called when the host controller request the current
// interface that is in use by the device.  This simply returns the value set
// by the last call to SetInterface().
//
// \return None.
//
//*****************************************************************************
static void
USBDGetInterface(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    unsigned long ulValue;

    ASSERT(ulIndex == 0);

    //
    // If we still have an address pending then the device is still not
    // configured.
    //
    if(g_sUSBDeviceState.ulDevAddress & DEV_ADDR_PENDING)
    {
        ulValue = 0;
    }
    else
    {
        ulValue = g_sUSBDeviceState.ulAltSetting;
    }

    //
    // Send the single byte response.
    //
    USBDCDSendDataEP0(0, (unsigned char *)&ulValue, 1);
}

//*****************************************************************************
//
// This function handles the SET_INTERFACE standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the data for this request.
//
// This function is called when a standard request for changing the interface
// is received from the host controller.  If this is a valid request the
// function will call the function specified by the InterfaceChange in the
// \e g_psUSBDeviceInfo->sCallbacks variable to notify the application that the
// interface has changed and will pass to it the new alternate interface.
//
// \return None.
//
//*****************************************************************************
static void
USBDSetInterface(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    const unsigned char *pConfigDescriptor;
    int iIndex;
    unsigned long ulAltIndex;

    ASSERT(ulIndex == 0);

    //
    // Use the current configuration.
    //
    pConfigDescriptor =
     g_psUSBDeviceInfo->ppConfigDescriptors[g_sUSBDeviceState.ulConfiguration];

    //
    // Reset the index for the search to past the Configuration descriptor.
    //
    iIndex = pConfigDescriptor[0];

    //
    // Which index have we found.
    //
    ulAltIndex = 0;

    //
    // While the current index is less than the total number of configurations
    // keep looking for the interface requested.
    //
    while(iIndex < (unsigned short)pConfigDescriptor[2])
    {
        //
        // If this is an interface descriptor, see if it is the one that is
        // being searched for.
        //
        if(pConfigDescriptor[iIndex + 1] == USB_DTYPE_INTERFACE)
        {
            //
            // If the alternate inteface was not found then keep looking.
            //
            if(pUSBRequest->wValue != ulAltIndex)
            {
                //
                // Now looking for next alternate interface.
                //
                ulAltIndex++;
            }
            else
            {
                //
                // The index was found so save the alternate interface index.
                //
                g_sUSBDeviceState.ulAltSetting = ulAltIndex;

                //
                // If there is a callback then notify the application of the
                // change to the alternate interface.
                //
                if(g_psUSBDeviceInfo->sCallbacks.pfnInterfaceChange)
                {
                    g_psUSBDeviceInfo->sCallbacks.pfnInterfaceChange(0,
                        g_sUSBDeviceState.ulAltSetting);
                }

                break;
            }
        }
        else
        {
            //
            // If this was not an interface descriptor, then move on to the
            // next descriptor.
            //
            iIndex += pConfigDescriptor[iIndex];
        }
    }

    //
    // If the alternate interface was not found then stall to indicate that
    // this was a bad request.
    //
    if(iIndex >= (unsigned short)pConfigDescriptor[2])
    {
        USBDCDStallEP0(0);
    }
}

//*****************************************************************************
//
// This function handles the SYNC_FRAME standard USB request.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
// \param pUSBRequest holds the data for this request.
//
// This is currently a stub function that will stall indicating that the
// command is not supported.
//
// \return None.
//
//*****************************************************************************
static void
USBDSyncFrame(unsigned long ulIndex, tUSBRequest *pUSBRequest)
{
    ASSERT(ulIndex == 0);

    //
    // Not handled yet so stall this request.
    //
    USBDCDStallEP0(0);
}

//*****************************************************************************
//
// This internal function handles sending data on endpoint zero.
//
// \param ulIndex is the index of the USB controller which is to be
// initialized.
//
// \return None.
//
//*****************************************************************************
static void
USBDEP0StateTx(unsigned long ulIndex)
{
    unsigned long ulNumBytes;
    unsigned char *pData;

    ASSERT(ulIndex == 0);

    //
    // In the TX state on endpoint zero.
    //
    g_eUSBDEP0State = USB_STATE_TX;

    //
    // Set the number of bytes to send this iteration.
    //
    ulNumBytes = g_sUSBDeviceState.ulEP0DataRemain;

    //
    // Limit individual transfers to 64 bytes.
    //
    if(ulNumBytes > 64)
    {
        ulNumBytes = 64;
    }

    //
    // Save the pointer so that it can be passed to the USBEndpointDataPut()
    // function.
    //
    pData = (unsigned char *)g_sUSBDeviceState.pEP0Data;

    //
    // Advance the data pointer and counter to the next data to be sent.
    //
    g_sUSBDeviceState.ulEP0DataRemain -= ulNumBytes;
    g_sUSBDeviceState.pEP0Data += ulNumBytes;

    //
    // Put the data in the correct FIFO.
    //
    USBEndpointDataPut(USB0_BASE, USB_EP_0, pData, ulNumBytes);

    //
    // If this is exactly 64 then don't set the last packet yet.
    //
    if(ulNumBytes == 64)
    {
        //
        // There is more data to send or exactly 64 bytes were sent, this
        // means that there is either more data coming or a null packet needs
        // to be sent to complete the transaction.
        //
        USBEndpointDataSend(USB0_BASE, USB_EP_0, USB_TRANS_IN);
    }
    else
    {
        //
        // Send the last bit of data.
        //
        USBEndpointDataSend(USB0_BASE, USB_EP_0, USB_TRANS_IN_LAST);

        //
        // If there is a receive callback then call it.
        //
        if((g_psUSBDeviceInfo->sCallbacks.pfnDataSent) &&
           (g_sUSBDeviceState.ulOUTDataSize != 0))
        {
            //
            // Call the custom handler.
            //
            g_psUSBDeviceInfo->sCallbacks.pfnDataSent(0,
                                              g_sUSBDeviceState.ulOUTDataSize);

            //
            // There is no longer any data pending to be sent.
            //
            g_sUSBDeviceState.ulOUTDataSize = 0;
        }

        //
        // Now go to the status state and wait for the transmit to complete.
        //
        g_eUSBDEP0State = USB_STATE_STATUS;
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
