/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMask_DEFINED
#define SkMask_DEFINED

#include "include/core/SkColorPriv.h"
#include "include/core/SkRect.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"

#include <cstddef>
#include <cstdint>
#include <memory>

/** \class SkMask
    SkMask is used to describe alpha bitmaps, either 1bit, 8bit, or
    the 3-channel 3D format. These are passed to SkMaskFilter objects.
*/
struct SkMask {
    enum Format : uint8_t {
        kBW_Format, //!< 1bit per pixel mask (e.g. monochrome)
        kA8_Format, //!< 8bits per pixel mask (e.g. antialiasing)
        k3D_Format, //!< 3 8bit per pixl planes: alpha, mul, add
        kARGB32_Format,         //!< SkPMColor
        kLCD16_Format,          //!< 565 alpha for r/g/b
        kSDF_Format,            //!< 8bits representing signed distance field
    };

    enum {
        kCountMaskFormats = kSDF_Format + 1
    };

    SkMask(const uint8_t* img, const SkIRect& bounds, uint32_t rowBytes, Format format)
        : fImage(img), fBounds(bounds), fRowBytes(rowBytes), fFormat(format) {}
    uint8_t const * const fImage;
    const SkIRect fBounds;
    const uint32_t fRowBytes;
    const Format fFormat;

    static bool IsValidFormat(uint8_t format) { return format < kCountMaskFormats; }

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
    const uint8_t* getAddr1(int x, int y) const {
        SkASSERT(kBW_Format == fFormat);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fImage != nullptr);
        return fImage + ((x - fBounds.fLeft) >> 3) + (y - fBounds.fTop) * fRowBytes;
    }

    /** Returns the address of the specified byte.
        Asserts that the mask is kA8_Format, and that x,y are in range.
        x,y are in the same coordiate space as fBounds.
    */
    const uint8_t* getAddr8(int x, int y) const {
        SkASSERT(kA8_Format == fFormat || kSDF_Format == fFormat);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fImage != nullptr);
        return fImage + x - fBounds.fLeft + (y - fBounds.fTop) * fRowBytes;
    }

    /**
     *  Return the address of the specified 16bit mask. In the debug build,
     *  this asserts that the mask's format is kLCD16_Format, and that (x,y)
     *  are contained in the mask's fBounds.
     */
    const uint16_t* getAddrLCD16(int x, int y) const {
        SkASSERT(kLCD16_Format == fFormat);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fImage != nullptr);
        const uint16_t* row = (const uint16_t*)(fImage + (y - fBounds.fTop) * fRowBytes);
        return row + (x - fBounds.fLeft);
    }

    /**
     *  Return the address of the specified 32bit mask. In the debug build,
     *  this asserts that the mask's format is 32bits, and that (x,y)
     *  are contained in the mask's fBounds.
     */
    const uint32_t* getAddr32(int x, int y) const {
        SkASSERT(kARGB32_Format == fFormat);
        SkASSERT(fBounds.contains(x, y));
        SkASSERT(fImage != nullptr);
        const uint32_t* row = (const uint32_t*)(fImage + (y - fBounds.fTop) * fRowBytes);
        return row + (x - fBounds.fLeft);
    }

    /**
     *  Returns the address of the specified pixel, computing the pixel-size
     *  at runtime based on the mask format. This will be slightly slower than
     *  using one of the routines where the format is implied by the name
     *  e.g. getAddr8 or getAddr32.
     *
     *  x,y must be contained by the mask's bounds (this is asserted in the
     *  debug build, but not checked in the release build.)
     *
     *  This should not be called with kBW_Format, as it will give unspecified
     *  results (and assert in the debug build).
     */
    const void* getAddr(int x, int y) const;

    /** Iterates over the coverage values along a scanline in a given SkMask::Format. Provides
     *  constructor, copy constructor for creating
     *  operator++, operator-- for iterating over the coverage values on a scanline
     *  operator>>= to add row bytes
     *  operator* to get the coverage value at the current location
     *  operator< to compare two iterators
     */
    template <Format F> struct AlphaIter;
};

template <> struct SkMask::AlphaIter<SkMask::kBW_Format> {
    AlphaIter(const uint8_t* ptr, int offset) : fPtr(ptr), fOffset(7 - offset) {}
    AlphaIter(const AlphaIter& that) : fPtr(that.fPtr), fOffset(that.fOffset) {}
    AlphaIter& operator++() {
        if (0 < fOffset ) {
            --fOffset;
        } else {
            ++fPtr;
            fOffset = 7;
        }
        return *this;
    }
    AlphaIter& operator--() {
        if (fOffset < 7) {
            ++fOffset;
        } else {
            --fPtr;
            fOffset = 0;
        }
        return *this;
    }
    AlphaIter& operator>>=(uint32_t rb) {
        fPtr = SkTAddOffset<const uint8_t>(fPtr, rb);
        return *this;
    }
    uint8_t operator*() const { return ((*fPtr) >> fOffset) & 1 ? 0xFF : 0; }
    bool operator<(const AlphaIter& that) const {
        return fPtr < that.fPtr || (fPtr == that.fPtr && fOffset > that.fOffset);
    }
    const uint8_t* fPtr;
    int fOffset;
};

template <> struct SkMask::AlphaIter<SkMask::kA8_Format> {
    AlphaIter(const uint8_t* ptr) : fPtr(ptr) {}
    AlphaIter(const AlphaIter& that) : fPtr(that.fPtr) {}
    AlphaIter& operator++() { ++fPtr; return *this; }
    AlphaIter& operator--() { --fPtr; return *this; }
    AlphaIter& operator>>=(uint32_t rb) {
        fPtr = SkTAddOffset<const uint8_t>(fPtr, rb);
        return *this;
    }
    uint8_t operator*() const { return *fPtr; }
    bool operator<(const AlphaIter& that) const { return fPtr < that.fPtr; }
    const uint8_t* fPtr;
};

template <> struct SkMask::AlphaIter<SkMask::kARGB32_Format> {
    AlphaIter(const uint32_t* ptr) : fPtr(ptr) {}
    AlphaIter(const AlphaIter& that) : fPtr(that.fPtr) {}
    AlphaIter& operator++() { ++fPtr; return *this; }
    AlphaIter& operator--() { --fPtr; return *this; }
    AlphaIter& operator>>=(uint32_t rb) {
        fPtr = SkTAddOffset<const uint32_t>(fPtr, rb);
        return *this;
    }
    uint8_t operator*() const { return SkGetPackedA32(*fPtr); }
    bool operator<(const AlphaIter& that) const { return fPtr < that.fPtr; }
    const uint32_t* fPtr;
};

template <> struct SkMask::AlphaIter<SkMask::kLCD16_Format> {
    AlphaIter(const uint16_t* ptr) : fPtr(ptr) {}
    AlphaIter(const AlphaIter& that) : fPtr(that.fPtr) {}
    AlphaIter& operator++() { ++fPtr; return *this; }
    AlphaIter& operator--() { --fPtr; return *this; }
    AlphaIter& operator>>=(uint32_t rb) {
        fPtr = SkTAddOffset<const uint16_t>(fPtr, rb);
        return *this;
    }
    uint8_t operator*() const {
        unsigned packed = *fPtr;
        unsigned r = SkPacked16ToR32(packed);
        unsigned g = SkPacked16ToG32(packed);
        unsigned b = SkPacked16ToB32(packed);
        return (r + g + b) / 3;
    }
    bool operator<(const AlphaIter& that) const { return fPtr < that.fPtr; }
    const uint16_t* fPtr;
};

///////////////////////////////////////////////////////////////////////////////

struct SkMaskBuilder : public SkMask {
    SkMaskBuilder() : SkMask(nullptr, {0}, 0, SkMask::Format::kBW_Format) {}
    SkMaskBuilder(const SkMaskBuilder&) = delete;
    SkMaskBuilder(SkMaskBuilder&&) = default;
    SkMaskBuilder& operator=(const SkMaskBuilder&) = delete;
    SkMaskBuilder& operator=(SkMaskBuilder&& that) {
        this->image() = that.image();
        this->bounds() = that.bounds();
        this->rowBytes() = that.rowBytes();
        this->format() = that.format();
        that.image() = nullptr;
        return *this;
    }

    SkMaskBuilder(uint8_t* img, const SkIRect& bounds, uint32_t rowBytes, Format format)
        : SkMask(img, bounds, rowBytes, format) {}

    uint8_t*& image() { return *const_cast<uint8_t**>(&fImage); }
    SkIRect& bounds() { return *const_cast<SkIRect*>(&fBounds); }
    uint32_t& rowBytes() { return *const_cast<uint32_t*>(&fRowBytes); }
    Format& format() { return *const_cast<Format*>(&fFormat); }

    /** Returns the address of the byte that holds the specified bit.
        Asserts that the mask is kBW_Format, and that x,y are in range.
        x,y are in the same coordiate space as fBounds.
    */
    uint8_t* getAddr1(int x, int y) {
        return const_cast<uint8_t*>(this->SkMask::getAddr1(x, y));
    }

    /** Returns the address of the specified byte.
        Asserts that the mask is kA8_Format, and that x,y are in range.
        x,y are in the same coordiate space as fBounds.
    */
    uint8_t* getAddr8(int x, int y) {
        return const_cast<uint8_t*>(this->SkMask::getAddr8(x, y));
    }

    /**
     *  Return the address of the specified 16bit mask. In the debug build,
     *  this asserts that the mask's format is kLCD16_Format, and that (x,y)
     *  are contained in the mask's fBounds.
     */
    uint16_t* getAddrLCD16(int x, int y) {
        return const_cast<uint16_t*>(this->SkMask::getAddrLCD16(x, y));
    }

    /**
     *  Return the address of the specified 32bit mask. In the debug build,
     *  this asserts that the mask's format is 32bits, and that (x,y)
     *  are contained in the mask's fBounds.
     */
    uint32_t* getAddr32(int x, int y) {
        return const_cast<uint32_t*>(this->SkMask::getAddr32(x, y));
    }

    /**
     *  Returns the address of the specified pixel, computing the pixel-size
     *  at runtime based on the mask format. This will be slightly slower than
     *  using one of the routines where the format is implied by the name
     *  e.g. getAddr8 or getAddr32.
     *
     *  x,y must be contained by the mask's bounds (this is asserted in the
     *  debug build, but not checked in the release build.)
     *
     *  This should not be called with kBW_Format, as it will give unspecified
     *  results (and assert in the debug build).
     */
    void* getAddr(int x, int y) {
        return const_cast<void*>(this->SkMask::getAddr(x, y));
    }

    enum AllocType {
        kUninit_Alloc,
        kZeroInit_Alloc,
    };
    static uint8_t* AllocImage(size_t bytes, AllocType = kUninit_Alloc);
    static void FreeImage(void* image);

    enum CreateMode {
        kJustComputeBounds_CreateMode,      //!< compute bounds and return
        kJustRenderImage_CreateMode,        //!< render into preallocate mask
        kComputeBoundsAndRenderImage_CreateMode  //!< compute bounds, alloc image and render into it
    };

    /**
     *  Returns initial destination mask data padded by radiusX and radiusY
     */
    static SkMaskBuilder PrepareDestination(int radiusX, int radiusY, const SkMask& src);
};

/**
 *  \using SkAutoMaskImage
 *
 *  Stack class used to manage the fImage buffer in a SkMask.
 *  When this object loses scope, the buffer is freed with SkMask::FreeImage().
 */
using SkAutoMaskFreeImage = std::unique_ptr<uint8_t, SkFunctionObject<SkMaskBuilder::FreeImage>>;

#endif
