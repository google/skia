/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_GpuTypesInternal_DEFINED
#define skgpu_GpuTypesInternal_DEFINED

#include "include/core/SkColorType.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"

/**
 * This file includes internal types that are used by all of our gpu backends.
 */

namespace skgpu {

struct IRect16 {
    int16_t fLeft, fTop, fRight, fBottom;

    static IRect16 SK_WARN_UNUSED_RESULT MakeEmpty() {
        IRect16 r;
        r.setEmpty();
        return r;
    }

    static IRect16 SK_WARN_UNUSED_RESULT MakeWH(int16_t w, int16_t h) {
        IRect16 r;
        r.set(0, 0, w, h);
        return r;
    }

    static IRect16 SK_WARN_UNUSED_RESULT MakeXYWH(int16_t x, int16_t y, int16_t w, int16_t h) {
        IRect16 r;
        r.set(x, y, x + w, y + h);
        return r;
    }

    static IRect16 SK_WARN_UNUSED_RESULT Make(const SkIRect& ir) {
        IRect16 r;
        r.set(ir);
        return r;
    }

    int width() const { return fRight - fLeft; }
    int height() const { return fBottom - fTop; }
    int area() const { return this->width() * this->height(); }
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    void setEmpty() { memset(this, 0, sizeof(*this)); }

    void set(int16_t left, int16_t top, int16_t right, int16_t bottom) {
        fLeft = left;
        fTop = top;
        fRight = right;
        fBottom = bottom;
    }

    void set(const SkIRect& r) {
        fLeft   = SkToS16(r.fLeft);
        fTop    = SkToS16(r.fTop);
        fRight  = SkToS16(r.fRight);
        fBottom = SkToS16(r.fBottom);
    }

    void offset(int16_t dx, int16_t dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }
};

/**
 *  Formats for masks, used by the font cache. Important that these are 0-based.
 */
enum class MaskFormat : int {
    kA8,    //!< 1-byte per pixel
    kA565,  //!< 2-bytes per pixel, RGB represent 3-channel LCD coverage
    kARGB,  //!< 4-bytes per pixel, color format

    kLast = kARGB
};
static const int kMaskFormatCount = static_cast<int>(MaskFormat::kLast) + 1;

/**
 *  Return the number of bytes-per-pixel for the specified mask format.
 */
inline constexpr int MaskFormatBytesPerPixel(MaskFormat format) {
    SkASSERT(static_cast<int>(format) < kMaskFormatCount);
    // kA8   (0) -> 1
    // kA565 (1) -> 2
    // kARGB (2) -> 4
    static_assert(static_cast<int>(MaskFormat::kA8) == 0, "enum_order_dependency");
    static_assert(static_cast<int>(MaskFormat::kA565) == 1, "enum_order_dependency");
    static_assert(static_cast<int>(MaskFormat::kARGB) == 2, "enum_order_dependency");

    return SkTo<int>(1u << static_cast<int>(format));
}

static constexpr SkColorType MaskFormatToColorType(MaskFormat format) {
    switch (format) {
        case MaskFormat::kA8:
            return kAlpha_8_SkColorType;
        case MaskFormat::kA565:
            return kRGB_565_SkColorType;
        case MaskFormat::kARGB:
            return kRGBA_8888_SkColorType;
    }
    SkUNREACHABLE;
}

} // namespace skgpu

#endif // skgpu_GpuTypesInternal_DEFINED
