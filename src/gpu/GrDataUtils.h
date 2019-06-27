/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDataUtils_DEFINED
#define GrDataUtils_DEFINED

#include "include/core/SkColor.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSwizzle.h"

// TODO: consolidate all the backend-specific flavors of this method to this
size_t GrETC1CompressedDataSize(int w, int h);

// TODO: should this be grown into a replacement for GrPixelConfig?
enum class GrCompression {
    kNone,
    kETC1,
};

// Compute the size of the buffer required to hold all the mipLevels of the specified type
// of data when all rowBytes are tight.
// Note there may still be padding between the mipLevels to meet alignment requirements.
size_t GrComputeTightCombinedBufferSize(GrCompression, size_t bytesPerPixel,
                                        int baseWidth, int baseHeight,
                                        SkTArray<size_t>* individualMipOffsets,
                                        int mipLevelCount);

void GrFillInData(GrCompression, GrPixelConfig,
                  int baseWidth, int baseHeight,
                  const SkTArray<size_t>& individualMipOffsets,
                  char* dest, const SkColor4f& color);

// TODO: Replace with GrColorSpaceInfo once GrPixelConfig is excised from that type.
struct GrColorInfo {
    constexpr GrColorInfo() = default;

    constexpr GrColorInfo(GrColorType ct, SkAlphaType at, SkColorSpace* cs) : fColorSpace(cs), fColorType(ct), fAlphaType(at) {}

    constexpr bool isValid() const {
        return fColorType != GrColorType::kUnknown && fAlphaType != kUnknown_SkAlphaType;
    }

    SkColorSpace* fColorSpace = nullptr;
    GrColorType fColorType = GrColorType::kUnknown;
    SkAlphaType fAlphaType = kUnknown_SkAlphaType;

};

struct GrPixelInfo {
    constexpr GrPixelInfo() = default;
    explicit GrPixelInfo(const SkImageInfo& info)
            : fColorInfo(SkColorTypeToGrColorType(info.colorType()), info.alphaType(),
                         info.colorSpace())
            , fWidth(info.width())
            , fHeight(info.height()) {}

    constexpr GrPixelInfo(GrColorType ct, SkAlphaType at, SkColorSpace* cs, int w, int h)
            : fColorInfo(ct, at, cs), fWidth(w), fHeight(h) {}

    constexpr GrColorType colorType() const { return fColorInfo.fColorType; }

    constexpr SkAlphaType alphaType() const { return fColorInfo.fAlphaType; }

    constexpr SkColorSpace* colorSpace() const { return fColorInfo.fColorSpace; }

    constexpr int width() const { return fWidth; }

    constexpr int height() const { return fHeight; }

    constexpr size_t bpp() const { return GrColorTypeBytesPerPixel(this->colorType()); }

    constexpr size_t minRowBytes() const { return this->bpp() * this->width(); }

    // Place this pixel rect in a surface of surfaceWidth x surfaceHeight size at offset (x, y) and
    // clip it to the bounds of the surface. If the pixel rect does not intersect the rectangle or
    // is empty then return false and leave unmodified. If clipped the input x/y, the width/
    // height of this info, and the data pointer will be modified.
    template<typename T>
    bool clip(int surfaceWidth, int surfaceHeight, int* x, int* y, T** data, size_t rowBytes) {
        auto bounds = SkIRect::MakeWH(surfaceWidth, surfaceHeight);
        auto rect = SkIRect::MakeXYWH(*x, *y, fWidth, fHeight);
        if (!rect.intersect(bounds)) {
            return false;
        }
        using C = typename std::conditional<std::is_const<T>::value, const char, char>::type;
        *data = static_cast<T*>(static_cast<C*>(*data) +
                                (rect.fTop - *y) * rowBytes + (rect.fLeft - *x) * this->bpp());
        *x = rect.fLeft;
        *y = rect.fTop;
        fWidth = rect.width();
        fHeight = rect.height();
        return true;
    }

    GrColorInfo fColorInfo = {};
    int fWidth = 0;
    int fHeight = 0;

    constexpr bool isValid() const {
        return fColorInfo.isValid() && fWidth > 0 && fHeight > 0;
    }
};

// Swizzle param is applied after loading and before converting from srcInfo to dstInfo.
bool GrConvertPixels(const GrPixelInfo& dstInfo,       void* dst, size_t dstRB,
                     const GrPixelInfo& srcInfo, const void* src, size_t srcRB,
                     bool flipY = false, GrSwizzle swizzle = GrSwizzle{});

#endif
