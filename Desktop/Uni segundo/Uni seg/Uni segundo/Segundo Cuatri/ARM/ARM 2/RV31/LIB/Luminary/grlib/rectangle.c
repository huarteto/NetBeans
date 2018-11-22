//*****************************************************************************
//
// rectangle.c - Routines for drawing and filling rectangles.
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
// This is part of revision 2523 of the Stellaris Graphics Library.
//
//*****************************************************************************

#include "debug.h"
#include "grlib.h"

//*****************************************************************************
//
//! \addtogroup primitives_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! Draws a rectangle.
//!
//! \param pContext is a pointer to the drawing context to use.
//! \param pRect is a pointer to the structure containing the extents of the
//! rectangle.
//!
//! This function draws a rectangle.  The rectangle will extend from \e lXMin
//! to \e lXMax and \e lYMin to \e lYMax, inclusive.
//!
//! \return None.
//
//*****************************************************************************
void
GrRectDraw(const tContext *pContext, const tRectangle *pRect)
{
    //
    // Check the arguments.
    //
    ASSERT(pContext);
    ASSERT(pRect);

    //
    // Draw a line across the top of the rectangle.
    //
    GrLineDrawH(pContext, pRect->sXMin, pRect->sXMax, pRect->sYMin);

    //
    // Return if the rectangle is one pixel tall.
    //
    if(pRect->sYMin == pRect->sYMax)
    {
        return;
    }

    //
    // Draw a line down the right side of the rectangle.
    //
    GrLineDrawV(pContext, pRect->sXMax, pRect->sYMin + 1, pRect->sYMax);

    //
    // Return if the rectangle is one pixel wide.
    //
    if(pRect->sXMin == pRect->sXMax)
    {
        return;
    }

    //
    // Draw a line across the bottom of the rectangle.
    //
    GrLineDrawH(pContext, pRect->sXMax - 1, pRect->sXMin, pRect->sYMax);

    //
    // Return if the rectangle is two pixels tall.
    //
    if((pRect->sYMin + 1) == pRect->sYMax)
    {
        return;
    }

    //
    // Draw a line up the left side of the rectangle.
    //
    GrLineDrawV(pContext, pRect->sXMin, pRect->sYMax - 1, pRect->sYMin + 1);
}

//*****************************************************************************
//
//! Draws a filled rectangle.
//!
//! \param pContext is a pointer to the drawing context to use.
//! \param pRect is a pointer to the structure containing the extents of the
//! rectangle.
//!
//! This function draws a filled rectangle.  The rectangle will extend from
//! \e lXMin to \e lXMax and \e lYMin to \e lYMax, inclusive.  The clipping of
//! the rectangle to the clipping rectangle is performed within this routine;
//! the display driver's rectangle fill routine is used to perform the actual
//! rectangle fill.
//!
//! \return None.
//
//*****************************************************************************
void
GrRectFill(const tContext *pContext, const tRectangle *pRect)
{
    tRectangle sTemp;

    //
    // Check the arguments.
    //
    ASSERT(pContext);
    ASSERT(pRect);

    //
    // Swap the X coordinates if sXMin is greater than sXMax.
    //
    if(pRect->sXMin > pRect->sXMax)
    {
        sTemp.sXMin = pRect->sXMax;
        sTemp.sXMax = pRect->sXMin;
    }
    else
    {
        sTemp.sXMin = pRect->sXMin;
        sTemp.sXMax = pRect->sXMax;
    }

    //
    // Swap the Y coordinates if sYMin is greater than sYMax.
    //
    if(pRect->sYMin > pRect->sYMax)
    {
        sTemp.sYMin = pRect->sYMax;
        sTemp.sYMax = pRect->sYMin;
    }
    else
    {
        sTemp.sYMin = pRect->sYMin;
        sTemp.sYMax = pRect->sYMax;
    }

    //
    // Now that the coordiantes are ordered, return without drawing anything if
    // the entire rectangle is out of the clipping region.
    //
    if((sTemp.sXMin > pContext->sClipRegion.sXMax) ||
       (sTemp.sXMax < pContext->sClipRegion.sXMin) ||
       (sTemp.sYMin > pContext->sClipRegion.sYMax) ||
       (sTemp.sYMax < pContext->sClipRegion.sYMin))
    {
        return;
    }

    //
    // Clip the X coordinates to the edges of the clipping region if necessary.
    //
    if(sTemp.sXMin < pContext->sClipRegion.sXMin)
    {
        sTemp.sXMin = pContext->sClipRegion.sXMin;
    }
    if(sTemp.sXMax > pContext->sClipRegion.sXMax)
    {
        sTemp.sXMax = pContext->sClipRegion.sXMax;
    }

    //
    // Clip the Y coordinates to the edges of the clipping region if necessary.
    //
    if(sTemp.sYMin < pContext->sClipRegion.sYMin)
    {
        sTemp.sYMin = pContext->sClipRegion.sYMin;
    }
    if(sTemp.sYMax > pContext->sClipRegion.sYMax)
    {
        sTemp.sYMax = pContext->sClipRegion.sYMax;
    }

    //
    // Call the low level rectangle fill routine.
    //
    DpyRectFill(pContext->pDisplay, &sTemp, pContext->ulForeground);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
