/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageInfo_DEFINED
#define SkImageInfo_DEFINED

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
    kBGRA_8888_SkColorType,
    kIndex_8_SkColorType,
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
    static const uint8_t gSize[] = {
        0,  // Unknown
        1,  // Alpha_8
        2,  // RGB_565
        2,  // ARGB_4444
        4,  // RGBA_8888
        4,  // BGRA_8888
        1,  // kIndex_8
        1,  // kGray_8
        8,  // kRGBA_F16
    };
    static_assert(SK_ARRAY_COUNT(gSize) == (size_t)(kLastEnum_SkColorType + 1),
                  "size_mismatch_with_SkColorType_enum");

    SkASSERT((size_t)ct < SK_ARRAY_COUNT(gSize));
    return gSize[ct];
}

static inline size_t SkColorTypeMinRowBytes(SkColorType ct, int width) {
    return width * SkColorTypeBytesPerPixel(ct);
}

static inline bool SkColorTypeIsValid(unsigned value) {
    return value <= kLastEnum_SkColorType;
}

static inline size_t SkColorTypeComputeOffset(SkColorType ct, int x, int y, size_t rowBytes) {
    int shift = 0;
    switch (SkColorTypeBytesPerPixel(ct)) {
        case 8: shift = 3; break;
        case 4: shift = 2; break;
        case 2: shift = 1; break;
        case 1: shift = 0; break;
        default: return 0;
    }
    return y * rowBytes + (x << shift);
}

///////////////////////////////////////////////////////////////////////////////

/**
 *  Return true if alphaType is supported by colorType. If there is a canonical
 *  alphaType for this colorType, return it in canonical.
 */
bool SkColorTypeValidateAlphaType(SkColorType colorType, SkAlphaType alphaType,
                                  SkAlphaType* canonical = NULL);

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

enum SkColorProfileType {
    kLinear_SkColorProfileType,
    kSRGB_SkColorProfileType,

    kLastEnum_SkColorProfileType = kSRGB_SkColorProfileType
};

/**
 *  Describe an image's dimensions and pixel type.
 *  Used for both src images and render-targets (surfaces).
 */
struct SK_API SkImageInfo {
public:
    SkImageInfo()
        : fWidth(0)
        , fHeight(0)
        , fColorType(kUnknown_SkColorType)
        , fAlphaType(kUnknown_SkAlphaType)
        , fProfileType(kLinear_SkColorProfileType)
    {}

    static SkImageInfo Make(int width, int height, SkColorType ct, SkAlphaType at,
                            SkColorProfileType pt = kLinear_SkColorProfileType) {
        return SkImageInfo(width, height, ct, at, pt);
    }

    /**
     *  Sets colortype to the native ARGB32 type.
     */
    static SkImageInfo MakeN32(int width, int height, SkAlphaType at,
                               SkColorProfileType pt = kLinear_SkColorProfileType) {
        return SkImageInfo(width, height, kN32_SkColorType, at, pt);
    }

    /**
     *  Sets colortype to the native ARGB32 type, and the alphatype to premul.
     */
    static SkImageInfo MakeN32Premul(int width, int height,
                                     SkColorProfileType pt = kLinear_SkColorProfileType) {
        return SkImageInfo(width, height, kN32_SkColorType, kPremul_SkAlphaType, pt);
    }

    /**
     *  Sets colortype to the native ARGB32 type, and the alphatype to premul.
     */
    static SkImageInfo MakeN32Premul(const SkISize& size,
                                     SkColorProfileType pt = kLinear_SkColorProfileType) {
        return MakeN32Premul(size.width(), size.height(), pt);
    }

    static SkImageInfo MakeA8(int width, int height) {
        return SkImageInfo(width, height, kAlpha_8_SkColorType, kPremul_SkAlphaType,
                           kLinear_SkColorProfileType);
    }

    static SkImageInfo MakeUnknown(int width, int height) {
        return SkImageInfo(width, height, kUnknown_SkColorType, kUnknown_SkAlphaType,
                           kLinear_SkColorProfileType);
    }

    static SkImageInfo MakeUnknown() {
        return SkImageInfo();
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    SkColorType colorType() const { return fColorType; }
    SkAlphaType alphaType() const { return fAlphaType; }
    SkColorProfileType profileType() const { return fProfileType; }

    bool isEmpty() const { return fWidth <= 0 || fHeight <= 0; }

    bool isOpaque() const {
        return SkAlphaTypeIsOpaque(fAlphaType);
    }

    bool isLinear() const { return kLinear_SkColorProfileType == fProfileType; }
    bool isSRGB() const { return kSRGB_SkColorProfileType == fProfileType; }

    SkISize dimensions() const { return SkISize::Make(fWidth, fHeight); }
    SkIRect bounds() const { return SkIRect::MakeWH(fWidth, fHeight); }

    /**
     *  Return a new ImageInfo with the same colortype and alphatype as this info,
     *  but with the specified width and height.
     */
    SkImageInfo makeWH(int newWidth, int newHeight) const {
        return SkImageInfo::Make(newWidth, newHeight, fColorType, fAlphaType, fProfileType);
    }

    SkImageInfo makeAlphaType(SkAlphaType newAlphaType) const {
        return SkImageInfo::Make(fWidth, fHeight, fColorType, newAlphaType, fProfileType);
    }
    
    SkImageInfo makeColorType(SkColorType newColorType) const {
        return SkImageInfo::Make(fWidth, fHeight, newColorType, fAlphaType, fProfileType);
    }

    int bytesPerPixel() const {
        return SkColorTypeBytesPerPixel(fColorType);
    }

    uint64_t minRowBytes64() const {
        return sk_64_mul(fWidth, this->bytesPerPixel());
    }

    size_t minRowBytes() const {
        return (size_t)this->minRowBytes64();
    }

    size_t computeOffset(int x, int y, size_t rowBytes) const {
        SkASSERT((unsigned)x < (unsigned)fWidth);
        SkASSERT((unsigned)y < (unsigned)fHeight);
        return SkColorTypeComputeOffset(fColorType, x, y, rowBytes);
    }

    bool operator==(const SkImageInfo& other) const {
        return 0 == memcmp(this, &other, sizeof(other));
    }
    bool operator!=(const SkImageInfo& other) const {
        return 0 != memcmp(this, &other, sizeof(other));
    }

    void unflatten(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const;

    int64_t getSafeSize64(size_t rowBytes) const {
        if (0 == fHeight) {
            return 0;
        }
        return sk_64_mul(fHeight - 1, rowBytes) + fWidth * this->bytesPerPixel();
    }

    size_t getSafeSize(size_t rowBytes) const {
        int64_t size = this->getSafeSize64(rowBytes);
        if (!sk_64_isS32(size)) {
            return 0;
        }
        return sk_64_asS32(size);
    }

    bool validRowBytes(size_t rowBytes) const {
        uint64_t rb = sk_64_mul(fWidth, this->bytesPerPixel());
        return rowBytes >= rb;
    }

    SkDEBUGCODE(void validate() const;)

private:
    int                 fWidth;
    int                 fHeight;
    SkColorType         fColorType;
    SkAlphaType         fAlphaType;
    SkColorProfileType  fProfileType;

    SkImageInfo(int width, int height, SkColorType ct, SkAlphaType at, SkColorProfileType pt)
        : fWidth(width)
        , fHeight(height)
        , fColorType(ct)
        , fAlphaType(at)
        , fProfileType(pt)
    {}
};

///////////////////////////////////////////////////////////////////////////////

static inline bool SkColorAndProfileAreGammaCorrect(SkColorType ct, SkColorProfileType pt) {
    return kSRGB_SkColorProfileType == pt || kRGBA_F16_SkColorType == ct;
}

static inline bool SkImageInfoIsGammaCorrect(const SkImageInfo& info) {
    return SkColorAndProfileAreGammaCorrect(info.colorType(), info.profileType());
}

#endif
