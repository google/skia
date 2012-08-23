
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMask_DEFINED
#define SkMask_DEFINED

#include "SkRect.h"

/** \class SkMask
    SkMask is used to describe alpha bitmaps, either 1bit, 8bit, or
    the 3-channel 3D format. These are passed to SkMaskFilter objects.
*/
struct SkMask {
    enum Format {
        kBW_Format, //!< 1bit per pixel mask (e.g. monochrome)
        kA8_Format, //!< 8bits per pixel mask (e.g. antialiasing)
        k3D_Format, //!< 3 8bit per pixl planes: alpha, mul, add
        kARGB32_Format,         //!< SkPMColor
        kLCD16_Format,          //!< 565 alpha for r/g/b
        kLCD32_Format           //!< 888 alpha for r/g/b
    };

    enum {
        kCountMaskFormats = kLCD32_Format + 1
    };

    uint8_t*    fImage;
    SkIRect     fBounds;
    uint32_t    fRowBytes;
    Format      fFormat;

    /** Returns true if the mask is empty: i.e. it has an empty bounds.
     */
    bool isEmpty() const { return fBounds.isEmpty(); }

    /** Return the byte size of the mask, assuming only 1 plane.
        Does not account for k3D_Format. For that, use computeTotalImageSize().
        If there is an overflow of 32bits, then returns 0.
    */
    size_t computeImageSize() const;

    /** Return the byte size of the mask, taking into account
        any extra planes (e.g. k3D_Format).
        If there is an overflow of 32bits, then returns 0.
    */
    size_t computeTotalImageSize() const;

    /** Returns the address of the byte that holds the specified bit.
        Asserts that the mask is kBW_Format, and that x,y are in range.
        x,y are in the same coordiate space as fBounds.
    */
    uint8_t* getAddr1(int x, int y) const {
        SkASSERT(kBW_Format == fFormat);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fImage != NULL);
        return fImage + ((x - fBounds.fLeft) >> 3) + (y - fBounds.fTop) * fRowBytes;
    }

    /** Returns the address of the specified byte.
        Asserts that the mask is kA8_Format, and that x,y are in range.
        x,y are in the same coordiate space as fBounds.
    */
    uint8_t* getAddr8(int x, int y) const {
        SkASSERT(kA8_Format == fFormat);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fImage != NULL);
        return fImage + x - fBounds.fLeft + (y - fBounds.fTop) * fRowBytes;
    }

    /**
     *  Return the address of the specified 16bit mask. In the debug build,
     *  this asserts that the mask's format is kLCD16_Format, and that (x,y)
     *  are contained in the mask's fBounds.
     */
    uint16_t* getAddrLCD16(int x, int y) const {
        SkASSERT(kLCD16_Format == fFormat);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fImage != NULL);
        uint16_t* row = (uint16_t*)(fImage + (y - fBounds.fTop) * fRowBytes);
        return row + (x - fBounds.fLeft);
    }

    /**
     *  Return the address of the specified 32bit mask. In the debug build,
     *  this asserts that the mask's format is kLCD32_Format, and that (x,y)
     *  are contained in the mask's fBounds.
     */
    uint32_t* getAddrLCD32(int x, int y) const {
        SkASSERT(kLCD32_Format == fFormat);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fImage != NULL);
        uint32_t* row = (uint32_t*)(fImage + (y - fBounds.fTop) * fRowBytes);
        return row + (x - fBounds.fLeft);
    }

    /**
     *  Returns the address of the specified pixel, computing the pixel-size
     *  at runtime based on the mask format. This will be slightly slower than
     *  using one of the routines where the format is implied by the name
     *  e.g. getAddr8 or getAddrLCD32.
     *
     *  x,y must be contained by the mask's bounds (this is asserted in the
     *  debug build, but not checked in the release build.)
     *
     *  This should not be called with kBW_Format, as it will give unspecified
     *  results (and assert in the debug build).
     */
    void* getAddr(int x, int y) const;

    static uint8_t* AllocImage(size_t bytes);
    static void FreeImage(void* image);

    enum CreateMode {
        kJustComputeBounds_CreateMode,      //!< compute bounds and return
        kJustRenderImage_CreateMode,        //!< render into preallocate mask
        kComputeBoundsAndRenderImage_CreateMode  //!< compute bounds, alloc image and render into it
    };
};

///////////////////////////////////////////////////////////////////////////////

/**
 *  \class SkAutoMaskImage
 *
 *  Stack class used to manage the fImage buffer in a SkMask.
 *  When this object loses scope, the buffer is freed with SkMask::FreeImage().
 */
class SkAutoMaskFreeImage {
public:
    SkAutoMaskFreeImage(uint8_t* maskImage) {
        fImage = maskImage;
    }

    ~SkAutoMaskFreeImage() {
        SkMask::FreeImage(fImage);
    }

private:
    uint8_t* fImage;
};

#endif

