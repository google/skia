/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageInfo_DEFINED
#define SkImageInfo_DEFINED

#include "SkColorSpace.h"
#include "SkMath.h"
#include "SkRect.h"
#include "SkSize.h"

class SkReadBuffer;
class SkWriteBuffer;

/**
 *  Describes how to interpret the alpha component of a pixel.
 */
enum SkAlphaType {
    kUnknown_SkAlphaType,

    /**
     *  All pixels are stored as opaque. This differs slightly from kIgnore in
     *  that kOpaque has correct "opaque" values stored in the pixels, while
     *  kIgnore may not, but in both cases the caller should treat the pixels
     *  as opaque.
     */
    kOpaque_SkAlphaType,

    /**
     *  All pixels have their alpha premultiplied in their color components.
     *  This is the natural format for the rendering target pixels.
     */
    kPremul_SkAlphaType,

    /**
     *  All pixels have their color components stored without any regard to the
     *  alpha. e.g. this is the default configuration for PNG images.
     *
     *  This alpha-type is ONLY supported for input images. Rendering cannot
     *  generate this on output.
     */
    kUnpremul_SkAlphaType,

    kLastEnum_SkAlphaType = kUnpremul_SkAlphaType
};

static inline bool SkAlphaTypeIsOpaque(SkAlphaType at) {
    return kOpaque_SkAlphaType == at;
}

static inline bool SkAlphaTypeIsValid(unsigned value) {
    return value <= kLastEnum_SkAlphaType;
}

///////////////////////////////////////////////////////////////////////////////

/** Temporary macro that allows us to add new color types without breaking Chrome compile. */
#define SK_EXTENDED_COLOR_TYPES

/**
 *  Describes how to interpret the components of a pixel.
 *
 *  kN32_SkColorType is an alias for whichever 32bit ARGB format is the "native"
 *  form for skia's blitters. Use this if you don't have a swizzle preference
 *  for 32bit pixels.
 */
enum SkColorType {
    kUnknown_SkColorType,
    kAlpha_8_SkColorType,
    kRGB_565_SkColorType,
    kARGB_4444_SkColorType,
    kRGBA_8888_SkColorType,
    kRGB_888x_SkColorType,
    kBGRA_8888_SkColorType,
    kRGBA_1010102_SkColorType,
    kRGB_101010x_SkColorType,
    kGray_8_SkColorType,
    kRGBA_F16_SkColorType,

    kLastEnum_SkColorType = kRGBA_F16_SkColorType,

#if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
    kN32_SkColorType = kBGRA_8888_SkColorType,
#elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
    kN32_SkColorType = kRGBA_8888_SkColorType,
#else
    #error "SK_*32_SHFIT values must correspond to BGRA or RGBA byte order"
#endif
};

static int SkColorTypeBytesPerPixel(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return 0;
        case kAlpha_8_SkColorType:      return 1;
        case kRGB_565_SkColorType:      return 2;
        case kARGB_4444_SkColorType:    return 2;
        case kRGBA_8888_SkColorType:    return 4;
        case kBGRA_8888_SkColorType:    return 4;
        case kRGB_888x_SkColorType:     return 4;
        case kRGBA_1010102_SkColorType: return 4;
        case kRGB_101010x_SkColorType:  return 4;
        case kGray_8_SkColorType:       return 1;
        case kRGBA_F16_SkColorType:     return 8;
    }
    return 0;
}

static int SkColorTypeShiftPerPixel(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return 0;
        case kAlpha_8_SkColorType:      return 0;
        case kRGB_565_SkColorType:      return 1;
        case kARGB_4444_SkColorType:    return 1;
        case kRGBA_8888_SkColorType:    return 2;
        case kRGB_888x_SkColorType:     return 2;
        case kBGRA_8888_SkColorType:    return 2;
        case kRGBA_1010102_SkColorType: return 2;
        case kRGB_101010x_SkColorType:  return 2;
        case kGray_8_SkColorType:       return 0;
        case kRGBA_F16_SkColorType:     return 3;
    }
    return 0;
}

static inline size_t SkColorTypeMinRowBytes(SkColorType ct, int width) {
    return width * SkColorTypeBytesPerPixel(ct);
}

static inline bool SkColorTypeIsValid(unsigned value) {
    return value <= kLastEnum_SkColorType;
}

static inline size_t SkColorTypeComputeOffset(SkColorType ct, int x, int y, size_t rowBytes) {
    if (kUnknown_SkColorType == ct) {
        return 0;
    }
    return y * rowBytes + (x << SkColorTypeShiftPerPixel(ct));
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return true if alphaType is supported by colorType. If there is a canonical
 *  alphaType for this colorType, return it in canonical.
 */
bool SkColorTypeValidateAlphaType(SkColorType colorType, SkAlphaType alphaType,
                                  SkAlphaType* canonical = nullptr);

///////////////////////////////////////////////////////////////////////////////

/**
 *  Describes the color space a YUV pixel.
 */
enum SkYUVColorSpace {
    /** Standard JPEG color space. */
    kJPEG_SkYUVColorSpace,
    /** SDTV standard Rec. 601 color space. Uses "studio swing" [16, 235] color
       range. See http://en.wikipedia.org/wiki/Rec._601 for details. */
    kRec601_SkYUVColorSpace,
    /** HDTV standard Rec. 709 color space. Uses "studio swing" [16, 235] color
       range. See http://en.wikipedia.org/wiki/Rec._709 for details. */
    kRec709_SkYUVColorSpace,

    kLastEnum_SkYUVColorSpace = kRec709_SkYUVColorSpace
};

///////////////////////////////////////////////////////////////////////////////

enum class SkDestinationSurfaceColorMode {
    kLegacy,
    kGammaAndColorSpaceAware,
};

/**
 *  Describe an image's dimensions and pixel type.
 *  Used for both src images and render-targets (surfaces).
 */
struct SK_API SkImageInfo {
public:
    SkImageInfo()
        : fColorSpace(nullptr)
        , fWidth(0)
        , fHeight(0)
        , fColorType(kUnknown_SkColorType)
        , fAlphaType(kUnknown_SkAlphaType)
    {}

    static SkImageInfo Make(int width, int height, SkColorType ct, SkAlphaType at,
                            sk_sp<SkColorSpace> cs = nullptr) {
        return SkImageInfo(width, height, ct, at, std::move(cs));
    }

    /**
     *  Sets colortype to the native ARGB32 type.
     */
    static SkImageInfo MakeN32(int width, int height, SkAlphaType at,
                               sk_sp<SkColorSpace> cs = nullptr) {
        return Make(width, height, kN32_SkColorType, at, cs);
    }

    /**
     *  Create an ImageInfo marked as SRGB with N32 swizzle.
     */
    static SkImageInfo MakeS32(int width, int height, SkAlphaType at);

    /**
     *  Sets colortype to the native ARGB32 type, and the alphatype to premul.
     */
    static SkImageInfo MakeN32Premul(int width, int height, sk_sp<SkColorSpace> cs = nullptr) {
        return Make(width, height, kN32_SkColorType, kPremul_SkAlphaType, cs);
    }

    static SkImageInfo MakeN32Premul(const SkISize& size) {
        return MakeN32Premul(size.width(), size.height());
    }

    static SkImageInfo MakeA8(int width, int height) {
        return Make(width, height, kAlpha_8_SkColorType, kPremul_SkAlphaType, nullptr);
    }

    static SkImageInfo MakeUnknown(int width, int height) {
        return Make(width, height, kUnknown_SkColorType, kUnknown_SkAlphaType, nullptr);
    }

    static SkImageInfo MakeUnknown() {
        return MakeUnknown(0, 0);
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    SkColorType colorType() const { return fColorType; }
    SkAlphaType alphaType() const { return fAlphaType; }
    SkColorSpace* colorSpace() const { return fColorSpace.get(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fColorSpace; }

    bool isEmpty() const { return fWidth <= 0 || fHeight <= 0; }

    bool isOpaque() const {
        return SkAlphaTypeIsOpaque(fAlphaType);
    }

    SkISize dimensions() const { return SkISize::Make(fWidth, fHeight); }
    SkIRect bounds() const { return SkIRect::MakeWH(fWidth, fHeight); }

    bool gammaCloseToSRGB() const {
        return fColorSpace && fColorSpace->gammaCloseToSRGB();
    }

    /**
     *  Return a new ImageInfo with the same colortype and alphatype as this info,
     *  but with the specified width and height.
     */
    SkImageInfo makeWH(int newWidth, int newHeight) const {
        return Make(newWidth, newHeight, fColorType, fAlphaType, fColorSpace);
    }

    SkImageInfo makeAlphaType(SkAlphaType newAlphaType) const {
        return Make(fWidth, fHeight, fColorType, newAlphaType, fColorSpace);
    }

    SkImageInfo makeColorType(SkColorType newColorType) const {
        return Make(fWidth, fHeight, newColorType, fAlphaType, fColorSpace);
    }

    SkImageInfo makeColorSpace(sk_sp<SkColorSpace> cs) const {
        return Make(fWidth, fHeight, fColorType, fAlphaType, std::move(cs));
    }

    int bytesPerPixel() const { return SkColorTypeBytesPerPixel(fColorType); }

    int shiftPerPixel() const { return SkColorTypeShiftPerPixel(fColorType); }

    uint64_t minRowBytes64() const {
        return sk_64_mul(fWidth, this->bytesPerPixel());
    }

    size_t minRowBytes() const {
        uint64_t minRowBytes = this->minRowBytes64();
        if (!sk_64_isS32(minRowBytes)) {
            return 0;
        }
        return sk_64_asS32(minRowBytes);
    }

    size_t computeOffset(int x, int y, size_t rowBytes) const {
        SkASSERT((unsigned)x < (unsigned)fWidth);
        SkASSERT((unsigned)y < (unsigned)fHeight);
        return SkColorTypeComputeOffset(fColorType, x, y, rowBytes);
    }

    bool operator==(const SkImageInfo& other) const {
        return fWidth == other.fWidth && fHeight == other.fHeight &&
               fColorType == other.fColorType && fAlphaType == other.fAlphaType &&
               SkColorSpace::Equals(fColorSpace.get(), other.fColorSpace.get());
    }
    bool operator!=(const SkImageInfo& other) const {
        return !(*this == other);
    }

    void unflatten(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const;

    /**
     *  Returns the size (in bytes) of the image buffer that this info needs, given the specified
     *  rowBytes. The rowBytes must be >= this->minRowBytes().
     *
     *  if (height == 0) {
     *      return 0;
     *  } else {
     *      return (height - 1) * rowBytes + width * bytes_per_pixel;
     *  }
     *
     *  If the calculation overflows this returns SK_MaxSizeT
     */
    size_t computeByteSize(size_t rowBytes) const;

    /**
     *  Returns the minimum size (in bytes) of the image buffer that this info needs.
     *  If the calculation overflows, or if the height is 0, this returns 0.
     */
    size_t computeMinByteSize() const {
        return this->computeByteSize(this->minRowBytes());
    }

    // Returns true if the result of computeByteSize (or computeMinByteSize) overflowed
    static bool ByteSizeOverflowed(size_t byteSize) {
        return SK_MaxSizeT == byteSize;
    }

    bool validRowBytes(size_t rowBytes) const {
        uint64_t minRB = sk_64_mul(fWidth, this->bytesPerPixel());
        return rowBytes >= minRB;
    }

    void reset() {
        fColorSpace = nullptr;
        fWidth = 0;
        fHeight = 0;
        fColorType = kUnknown_SkColorType;
        fAlphaType = kUnknown_SkAlphaType;
    }

    SkDEBUGCODE(void validate() const;)

private:
    sk_sp<SkColorSpace> fColorSpace;
    int                 fWidth;
    int                 fHeight;
    SkColorType         fColorType;
    SkAlphaType         fAlphaType;

    SkImageInfo(int width, int height, SkColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs)
        : fColorSpace(std::move(cs))
        , fWidth(width)
        , fHeight(height)
        , fColorType(ct)
        , fAlphaType(at)
    {}
};

#endif
