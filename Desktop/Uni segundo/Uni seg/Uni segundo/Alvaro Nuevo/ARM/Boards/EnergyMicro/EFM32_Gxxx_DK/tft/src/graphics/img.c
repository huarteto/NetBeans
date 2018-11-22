/*************************************************************************//**
*           <b>(C) Copyright 2009 Energy Micro AS, www.energymicro.com</b>
******************************************************************************
*
* This source code is the property of Energy Micro AS. The source/compiled
* code may not be used, redistributed or modified without a written consent
* from Energy Micro AS.
*
* This copyright notice may not be removed from the source code nor changed.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
* obligation to support this Software. Energy Micro AS is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Energy Micro AS will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
******************************************************************************
* @file img.c
* @brief Bmp drawing for Display Control
* @author Energy Micro AS, I. Urdalen
*****************************************************************************/

/* Standard C header files */
#include <stdint.h>
#include <string.h>

/* EM types */
#include "graphics/em_types.h"

/* GLIB header files */
#include "graphics/glib/glib.h"
#include "graphics/glib/bmp.h"
#include "graphics/glib/glib_color.h"

/* File headers */
#include "rfs/rfs.h"

#include "img.h"

/* Buffer declarations */
static uint8_t rgbBuffer[ RGB_BUFFER_SIZE ];
static uint8_t palette[ BMP_PALETTE_8BIT_SIZE ];
RFS_FileHandle rfsHandle;

/* Local function declaration */
EMSTATUS readData(uint8_t buffer[], uint32_t bufLength, uint32_t bytesToRead);
static EMSTATUS setBmpSource(char *filename);
static EMSTATUS closeBmpSource();

/**************************************************************************//**
*  @brief
*  Initializes the bmp module
*
*  @return
*  Returns DISPCTRL_OK on success, or else error code
******************************************************************************/
EMSTATUS IMG_bmpInit(void)
{
  EMSTATUS status;

  status = BMP_init(palette, BMP_PALETTE_8BIT_SIZE, readData);
  if (status != BMP_OK) return status;

  return DISPCTRL_OK;
}

/**************************************************************************//**
*  @brief
*  Draws a BMP file from an external memory location
*
*  @param pContext
*  Pointer to the current drawing context
*  @param x
*  Upper left corner x-coordinate
*  @param y
*  Upper left corner y-coordinate
*  @param filename
*  Cstring that contains the filename. Use prefix "rfs/" if the file is located in
*  RFS, otherwise use full location on spi flash.
*
*  Example: "rfs/btnGreen.bmp" draws btnGreen.bmp located in RFS
*  Example: "\IMG\EM.bmp" draws EM.bmp located in IMG folder on SPI flash. It reads
*  from flash using emFile. Notice: emFile uses 8.3 FAT file format.
*
*  @return
*  Returns DISPCTRL_OK on success, or else error code
******************************************************************************/

EMSTATUS IMG_drawBmpEx(const GLIB_Context *pContext, uint16_t x,
                       uint16_t y, char* filename)
{
  EMSTATUS status;
  int32_t  xCursor;
  int32_t  yCursor;
  uint32_t pixelsRead;
  uint16_t xEnd;
  uint16_t yEnd;
  uint32_t nPixelsPerRow;
  uint32_t nRows;

  /* Check arguments */
  if (pContext == NULL) return DISPCTRL_ERROR_INVALID_ARGUMENT;

  /* Set bmp source to filename */
  status = setBmpSource(filename);
  if (status != DISPCTRL_OK) return status;

  /* Read in header and palette if necessary*/
  status = BMP_reset();

  /* Check against clipping region */
  xEnd = x + BMP_getWidth() - 1;
  yEnd = y + BMP_getHeight() - 1;

  if (GLIB_rectContainsPoint(&pContext->clippingRegion, xEnd, yEnd) == 0)
  {
    return GLIB_DID_NOT_DRAW;
  }

  /* Get important BMP data */
  nPixelsPerRow = BMP_getWidth();
  nRows         = BMP_getHeight();
  yCursor       = nRows - 1;
  xCursor       = 0;

  /* Set clipping region */
  DMD_setClippingArea(x, y, nPixelsPerRow, nRows);

  /* Read in and draw row for row */
  while (yCursor >= 0)
  {
    /* Read in row buffer */
    status = BMP_readRgbData(rgbBuffer, RGB_BUFFER_SIZE, &pixelsRead);
    if (status != BMP_OK || pixelsRead == 0)
    {
      break;
    }

    /* Draw row buffer. Remember, BMP is stored bottom-up */
    status = DMD_writeData(xCursor, yCursor, rgbBuffer, pixelsRead);
    if (status != DMD_OK)
    {
      return status;
    }
    /* Update cursor */
    xCursor += pixelsRead;
    if (xCursor >= nPixelsPerRow)
    {
      yCursor -= xCursor / nPixelsPerRow;
      xCursor  = xCursor % nPixelsPerRow;
    }
  }
  /* Reset clipping area in DMD driver */
  status = GLIB_resetDisplayClippingArea(pContext);
  if (status != 0) return status;

  closeBmpSource();

  return DISPCTRL_OK;
}


/**************************************************************************//**
*  @brief
*  Draws a BMP file from an external memory location, with all pixels of one color
*  transparent
*
*  @param pContext
*  Pointer to the current drawing context
*  @param x
*  Upper left corner x-coordinate
*  @param y
*  Upper left corner y-coordinate
*  @param filename
*  Cstring that contains the filename. Use prefix "rfs/" if the file is located in
*  RFS, otherwise use full location on spi flash.
*
*  Example: "rfs/btnGreen.bmp" draws btnGreen.bmp located in RFS
*  Example: "\IMG\EM.bmp" draws EM.bmp located in IMG folder on SPI flash. It reads
*  from flash using emFile. Notice: emFile uses 8.3 FAT file format.
*  @param transparentColor
*  The color that is considered transparent. All pixels of this color will not be
*  drawn, instead the color of the display at this pixel will be kept.
*
*  @return
*  Returns DISPCTRL_OK on success, or else error code
******************************************************************************/

EMSTATUS IMG_drawBmpTransparent(const GLIB_Context *pContext, uint16_t x, uint16_t y,
                                char* filename, uint32_t transparentColor)
{
  EMSTATUS status;
  int      i;
  uint32_t pixelColor;
  uint32_t opaquePixelsRead = 0;
  int32_t  xCursor;
  int32_t  yCursor;
  uint32_t pixelsRead;
  uint16_t xEnd;
  uint16_t yEnd;
  uint32_t nPixelsPerRow;
  uint32_t nRows;

  /* Check arguments */
  if (pContext == NULL) return DISPCTRL_ERROR_INVALID_ARGUMENT;

  /* Set bmp source to filename */
  status = setBmpSource(filename);
  if (status != DISPCTRL_OK) return status;

  /* Read in header and palette if necessary*/
  status = BMP_reset();

  /* Check against clipping region */
  xEnd = x + BMP_getWidth() - 1;
  yEnd = y + BMP_getHeight() - 1;

  if (GLIB_rectContainsPoint(&pContext->clippingRegion, xEnd, yEnd) == 0)
  {
    return GLIB_DID_NOT_DRAW;
  }

  /* Get important BMP data */
  nPixelsPerRow = BMP_getWidth();
  nRows         = BMP_getHeight();
  yCursor       = nRows - 1;
  xCursor       = 0;

  /* Set clipping region */
  DMD_setClippingArea(x, y, nPixelsPerRow, nRows);

  /* Read in and draw row for row */
  while (yCursor >= 0)
  {
    /* Read in row buffer */
    status = BMP_readRgbData(rgbBuffer, RGB_BUFFER_SIZE, &pixelsRead);
    if (status != BMP_OK || pixelsRead == 0)
    {
      break;
    }

    /* Loop through all pixels in buffer to see which ones to draw
     * and which ones not */
    for (i = 0; i < pixelsRead; i++)
    {
      pixelColor = GLIB_rgbColor(rgbBuffer[3 * i],
                                 rgbBuffer[3 * i + 1],
                                 rgbBuffer[3 * i + 2]);
      /* If a transparent pixel is found, draw all opaque pixels just
       * before this pixel, and skip this pixel */
      if (pixelColor == transparentColor)
      {
        if (opaquePixelsRead > 0)
        {
          /* If some opaque pixels were read before this transparent pixel,
           * draw those pixels */
          status = DMD_writeData(xCursor, yCursor,
                                 &rgbBuffer[3 * (i - opaquePixelsRead)],
                                 opaquePixelsRead);
          if (status != DMD_OK) return status;
          /* Update cursor */
          xCursor += opaquePixelsRead;
          if (xCursor >= nPixelsPerRow)
          {
            yCursor -= xCursor / nPixelsPerRow;
            xCursor  = xCursor % nPixelsPerRow;
          }
        }

        /* Update cursor */
        xCursor++;
        if (xCursor >= nPixelsPerRow)
        {
          yCursor -= xCursor / nPixelsPerRow;
          xCursor  = xCursor % nPixelsPerRow;
        }
        opaquePixelsRead = 0;
      }
      else
      {
        opaquePixelsRead++;
      }
    }
    if (opaquePixelsRead > 0)
    {
      /* Draw the rest of the opaque pixels, from the last transparent pixel
       * to the end of the buffer */
      status = DMD_writeData(xCursor, yCursor,
                             &rgbBuffer[3 * (i - opaquePixelsRead)],
                             opaquePixelsRead);
      if (status != DMD_OK) return status;
      /* Update cursor */
      xCursor += opaquePixelsRead;
      if (xCursor >= nPixelsPerRow)
      {
        yCursor -= xCursor / nPixelsPerRow;
        xCursor  = xCursor % nPixelsPerRow;
      }
      opaquePixelsRead = 0;
    }
  }
  /* Reset clipping area in DMD driver */
  status = GLIB_resetDisplayClippingArea(pContext);
  if (status != 0) return status;

  closeBmpSource();

  return DISPCTRL_OK;
}


/**************************************************************************//**
*  @brief
*  Draws a BMP file from an external memory location, with all the pixel color
*  components multiplied by the values given in redIntensity, greenIntensity and
*  blueIntensity. Used to fade the image by giving gradually increasing (or
*  decreasing values of the intensities)
*
*  @param pContext
*  Pointer to the current drawing context
*  @param x
*  Upper left corner x-coordinate
*  @param y
*  Upper left corner y-coordinate
*  @param filename
*  Cstring that contains the filename. Use prefix "rfs/" if the file is located in
*  RFS, otherwise use full location on spi flash.
*
*  Example: "rfs/btnGreen.bmp" draws btnGreen.bmp located in RFS
*  Example: "\IMG\EM.bmp" draws EM.bmp located in IMG folder on SPI flash. It reads
*  from flash using emFile. Notice: emFile uses 8.3 FAT file format.
*  @param redIntensity
*  An 8 bit integergiving the intensity of the red component of the pixels.
*  0x00 means that the red component won't be shown at all, 0xFF means that
*  the red component will be shown at its original intensity.
*  @param greenIntensity
*  An 8 bit integergiving the intensity of the green component of the pixels.
*  0x00 means that the green component won't be shown at all, 0xFF means that
*  the green component will be shown at its original intensity.
*  @param blueIntensity
*  An 8 bit integergiving the intensity of the blue component of the pixels.
*  0x00 means that the blue component won't be shown at all, 0xFF means that
*  the blue component will be shown at its original intensity.
*
*  @return
*  Returns DISPCTRL_OK on success, or else error code
******************************************************************************/

EMSTATUS IMG_drawBmpFade(const GLIB_Context *pContext, uint16_t x, uint16_t y,
                         char* filename, uint8_t redIntensity,
                         uint8_t greenIntensity, uint8_t blueIntensity)
{
  EMSTATUS status;
  int      i;
  int32_t  xCursor;
  int32_t  yCursor;
  uint32_t pixelsRead;
  uint16_t xEnd;
  uint16_t yEnd;
  uint32_t nPixelsPerRow;
  uint32_t nRows;

  /* Check arguments */
  if (pContext == NULL) return DISPCTRL_ERROR_INVALID_ARGUMENT;

  /* Set bmp source to filename */
  status = setBmpSource(filename);
  if (status != DISPCTRL_OK) return status;

  /* Read in header and palette if necessary*/
  status = BMP_reset();

  /* Check against clipping region */
  xEnd = x + BMP_getWidth() - 1;
  yEnd = y + BMP_getHeight() - 1;

  if (GLIB_rectContainsPoint(&pContext->clippingRegion, xEnd, yEnd) == 0)
  {
    return GLIB_DID_NOT_DRAW;
  }

  /* Get important BMP data */
  nPixelsPerRow = BMP_getWidth();
  nRows         = BMP_getHeight();
  yCursor       = nRows - 1;
  xCursor       = 0;

  /* Set clipping region */
  DMD_setClippingArea(x, y, nPixelsPerRow, nRows);

  /* Read in and draw row for row */
  while (yCursor >= 0)
  {
    /* Read in row buffer */
    status = BMP_readRgbData(rgbBuffer, RGB_BUFFER_SIZE, &pixelsRead);
    if (status != BMP_OK || pixelsRead == 0)
    {
      break;
    }

    for (i = 0; i < pixelsRead; i++)
    {
      /* Multiply each of the color components by the intensity value,
       * to create a darker color */
      rgbBuffer[3 * i] = (uint8_t)(((uint16_t) rgbBuffer[3 * i] *
                                    redIntensity) / 0xFF);
      rgbBuffer[3 * i + 1] = (uint8_t)(((uint16_t) rgbBuffer[3 * i + 1] *
                                        greenIntensity) / 0xFF);
      rgbBuffer[3 * i + 2] = (uint8_t)(((uint16_t) rgbBuffer[3 * i + 2] *
                                        blueIntensity) / 0xFF);
    }

    /* Draw row buffer. Remember, BMP is stored bottom-up */
    status = DMD_writeData(xCursor, yCursor, rgbBuffer, pixelsRead);
    if (status != DMD_OK)
    {
      return status;
    }
    /* Update cursor */
    xCursor += pixelsRead;
    if (xCursor >= nPixelsPerRow)
    {
      yCursor -= xCursor / nPixelsPerRow;
      xCursor  = xCursor % nPixelsPerRow;
    }
  }

  /* Reset clipping area in DMD driver */
  status = GLIB_resetDisplayClippingArea(pContext);
  if (status != 0) return status;

  closeBmpSource();

  return DISPCTRL_OK;
}

/**************************************************************************//**
*  @brief
*  Draws a BMP file from an external memory location, with all grayscale pixels
*  changed into a given color. All grayscale pixels in the image (pixels where the
*  red, green and blue components are equal) will be drawn as this color, with the
*  intensity of the color equal to the intensity of the grayscale pixels.
*  All pixels which are not grayscale will be drawn normally.
*
*  @param pContext
*  Pointer to the current drawing context
*  @param x
*  Upper left corner x-coordinate
*  @param y
*  Upper left corner y-coordinate
*  @param filename
*  Cstring that contains the filename. Use prefix "rfs/" if the file is located in
*  RFS, otherwise use full location on spi flash.
*
*  Example: "rfs/btnGreen.bmp" draws btnGreen.bmp located in RFS
*  Example: "\IMG\EM.bmp" draws EM.bmp located in IMG folder on SPI flash. It reads
*  from flash using emFile. Notice: emFile uses 8.3 FAT file format.
*  @param color
*  The color that all grayscale pixels will be changed into. Should be a color
*  where one or more of the color components are maximum (0xFF), and the others
*  are 0. Of the colors defined in glib_color.h, the only ones that should be used
*  are: Blue, Cyan, Lime, Magenta, Red, Yellow, White (keeps all grayscale pixels
*  as they are) and Black (deletes all grayscale pixels)
*
*  @return
*  Returns DISPCTRL_OK on success, or else error code
******************************************************************************/

EMSTATUS IMG_drawBmpGrayscaleToColor(const GLIB_Context *pContext, uint16_t x,
                                     uint16_t y, char* filename, uint32_t color)
{
  EMSTATUS status;
  int32_t  xCursor;
  int32_t  yCursor;
  uint32_t pixelsRead;
  uint16_t xEnd;
  uint16_t yEnd;
  uint32_t nPixelsPerRow;
  uint32_t nRows;
  int      i;

  /* Check arguments */
  if (pContext == NULL) return DISPCTRL_ERROR_INVALID_ARGUMENT;

  /* Set bmp source to filename */
  status = setBmpSource(filename);
  if (status != DISPCTRL_OK) return status;

  /* Read in header and palette if necessary*/
  status = BMP_reset();

  /* Check against clipping region */
  xEnd = x + BMP_getWidth() - 1;
  yEnd = y + BMP_getHeight() - 1;

  if (GLIB_rectContainsPoint(&pContext->clippingRegion, xEnd, yEnd) == 0)
  {
    return GLIB_DID_NOT_DRAW;
  }

  /* Get important BMP data */
  nPixelsPerRow = BMP_getWidth();
  nRows         = BMP_getHeight();
  yCursor       = nRows - 1;
  xCursor       = 0;

  /* Set clipping region */
  DMD_setClippingArea(x, y, nPixelsPerRow, nRows);

  /* Read in and draw row for row */
  while (yCursor >= 0)
  {
    /* Read in row buffer */
    status = BMP_readRgbData(rgbBuffer, RGB_BUFFER_SIZE, &pixelsRead);
    if (status != BMP_OK || pixelsRead == 0)
    {
      break;
    }

    for (i = 0; i < pixelsRead; i++)
    {
      /* Change grayscale pixels into the color given as an argument, with the
       * intensity of the color equal to the intensity of the grayscale pixel */
      if (rgbBuffer[3 * i] == rgbBuffer[3 * i + 1] &&
          rgbBuffer[3 * i + 1] == rgbBuffer[3 * i + 2])
      {
        rgbBuffer[3 * i] &= (uint8_t)((color & RedMask) >>
                                      RedShift);
        rgbBuffer[3 * i + 1] &= (uint8_t)((color & GreenMask) >>
                                          GreenShift);
        rgbBuffer[3 * i + 2] &= (uint8_t)((color & BlueMask) >>
                                          BlueShift);
      }
    }

    /* Draw row buffer. Remember, BMP is stored bottom-up */
    status = DMD_writeData(xCursor, yCursor, rgbBuffer, pixelsRead);
    if (status != DMD_OK)
    {
      return status;
    }
    /* Update cursor */
    xCursor += pixelsRead;
    if (xCursor >= nPixelsPerRow)
    {
      yCursor -= xCursor / nPixelsPerRow;
      xCursor  = xCursor % nPixelsPerRow;
    }
  }
  /* Reset clipping area in DMD driver */
  status = GLIB_resetDisplayClippingArea(pContext);
  if (status != 0) return status;

  closeBmpSource();

  return DISPCTRL_OK;
}

/**************************************************************************//**
*  @brief
*  Get the dimensions of a bmp image
*
*  @param filename
*  Cstring that contains the filename. Use prefix "rfs/" if the file is located in
*  RFS, otherwise use full location on spi flash.
*
*  Example: "rfs/btnGreen.bmp" draws btnGreen.bmp located in RFS
*  Example: "\IMG\EM.bmp" draw EM.bmp located in IMG folder on SPI flash. It reads
*  from flash using emFile. Notice: emFile uses 8.3 FAT file format.
*  @param width
*  Pointer to integer where the width of the image will be stored
*  @param height
*  Pointer to integer where the height of the image will be stored
*
*  @return
*  Returns DISPCTRL_OK on success
******************************************************************************/
EMSTATUS IMG_getImageDimensions(char* filename, uint16_t *width,
                                uint16_t *height)
{
  EMSTATUS status;

  /* Set bmp source to filename */
  status = setBmpSource(filename);
  if (status != DISPCTRL_OK) return status;

  status = BMP_reset();

  /* Read the width and height of the image */
  *width  = BMP_getWidth( );
  *height = BMP_getHeight( );

  closeBmpSource();

  return DISPCTRL_OK;
}

/**************************************************************************//**
*  @brief
*  Help function to set the source where the BMP file is read from
*
*  @param filename
*  Cstring that contains the filename. Use prefix "rfs/" if the file is located in
*  RFS, otherwise use full location on spi flash.
*
*  Example: "rfs/btnGreen.bmp" draws btnGreen.bmp located in RFS
*  Example: "\IMG\EM.bmp" draw EM.bmp located in IMG folder on SPI flash. It reads
*  from flash using emFile. Notice: emFile uses 8.3 FAT file format.
*
*  @return
*  Returns DISPCTRL_OK on success
******************************************************************************/
static EMSTATUS setBmpSource(char *filename)
{
  uint32_t strLength = strlen(filename);
  int32_t  rfsStatus;

  if (filename == NULL) return DISPCTRL_ERROR_INVALID_ARGUMENT;

  /* Open file using rfs */
  rfsStatus = RFS_fileOpen(&rfsHandle, (uint8_t *) filename);
  if (rfsStatus != 1)
  {
    return DISPCTRL_ERROR_IO;
  }
  return DISPCTRL_OK;
}

/**************************************************************************//**
*  @brief
*  Help function to close the bmp source
*
*  @return
*  Returns DISPCTRL_OK on success
******************************************************************************/
static EMSTATUS closeBmpSource()
{
  return DISPCTRL_OK;
}

/**************************************************************************//**
*  @brief
*  Function that reads bytesToRead into buffer. Required by BMP module
*
*  @param buffer
*  Buffer to be filled with bytes from bmp source
*  @param bufLength
*  Length of buffer
*  @param bytesToRead
*  Number of bytes to read from source set by setBmpSource
*
*  @return
*  Returns BMP_OK on success
******************************************************************************/
EMSTATUS readData(uint8_t buffer[], uint32_t bufLength, uint32_t bytesToRead)
{
  /* Use RFS to read from file */
  if (RFS_fileRead(buffer, 1, bytesToRead, &rfsHandle) < 0)
    return BMP_ERROR_IO;

  return BMP_OK;
}
