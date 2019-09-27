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
#include "src/gpu/GrColorSpaceInfo.h"
#include "src/gpu/GrSwizzle.h"

size_t GrCompressedDataSize(SkImage::CompressionType, int w, int h);

// Returns a value that can be used to set rowBytes for a transfer function.
size_t GrCompressedRowBytes(SkImage::CompressionType, int w);

// Compute the size of the buffer required to hold all the mipLevels of the specified type
// of data when all rowBytes are tight.
// Note there may still be padding between the mipLevels to meet alignment requirements.
size_t GrComputeTightCombinedBufferSize(size_t bytesPerPixel, int baseWidth, int baseHeight,
                                        SkTArray<size_t>* individualMipOffsets, int mipLevelCount);

void GrFillInData(GrPixelConfig, int baseWidth, int baseHeight,
                  const SkTArray<size_t>& individualMipOffsets, char* dest, const SkColor4f& color);

void GrFillInCompressedData(SkImage::CompressionType, int width, int height, char* dest,
                            const SkColor4f& color);
class GrPixelInfo {
public:
    GrPixelInfo() = default;

    // not explicit
    GrPixelInfo(const SkImageInfo& info)
            : fColorInfo(SkColorTypeToGrColorType(info.colorType()), info.alphaType(),
                         info.refColorSpace())
            , fSize(info.dimensions()) {}

    GrPixelInfo(GrColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs, int w, int h)
            : fColorInfo(ct, at, std::move(cs)), fSize{w, h} {}

    GrPixelInfo(GrColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs, SkISize size)
            : fColorInfo(ct, at, std::move(cs)), fSize(size) {}

    GrPixelInfo(const GrPixelInfo&) = default;
    GrPixelInfo(GrPixelInfo&&) = default;
    GrPixelInfo& operator=(const GrPixelInfo&) = default;
    GrPixelInfo& operator=(GrPixelInfo&&) = default;

    GrPixelInfo makeColorType(GrColorType ct) {
        return {ct, this->alphaType(), this->refColorSpace(), this->width(), this->height()};
    }

    GrPixelInfo makeAlphaType(SkAlphaType at) {
        return {this->colorType(), at, this->refColorSpace(), this->width(), this->height()};
    }

    GrPixelInfo makeWH(int width, int height) {
        return {this->colorType(), this->alphaType(), this->refColorSpace(), width, height};
    }

    const GrColorSpaceInfo& colorSpaceInfo() const { return fColorInfo; }

    GrColorType colorType() const { return fColorInfo.colorType(); }

    SkAlphaType alphaType() const { return fColorInfo.alphaType(); }

    SkColorSpace* colorSpace() const { return fColorInfo.colorSpace(); }

    sk_sp<SkColorSpace> refColorSpace() const { return fColorInfo.refColorSpace(); }

    SkISize size() const { return fSize; }

    int width() const { return fSize.width(); }

    int height() const { return fSize.height(); }

    size_t bpp() const { return GrColorTypeBytesPerPixel(this->colorType()); }

    size_t minRowBytes() const { return this->bpp() * this->width(); }

    /**
     * Place this pixel rect in a surface of dimensions surfaceWidth x surfaceHeight size offset at
     * surfacePt and then clip the pixel rectangle to the bounds of the surface. If the pixel rect
     * does not intersect the rectangle or is empty then return false. If clipped, the input
     * surfacePt, the width/height of this GrPixelInfo, and the data pointer will be modified to
     * reflect the clipped rectangle.
     */
    template <typename T>
    bool clip(int surfaceWidth, int surfaceHeight, SkIPoint* surfacePt, T** data, size_t rowBytes) {
        auto bounds = SkIRect::MakeWH(surfaceWidth, surfaceHeight);
        auto rect = SkIRect::MakeXYWH(surfacePt->fX, surfacePt->fY, this->width(), this->height());
        if (!rect.intersect(bounds)) {
            return false;
        }
        *data = SkTAddOffset<T>(*data, (rect.fTop  - surfacePt->fY) * rowBytes +
                                       (rect.fLeft - surfacePt->fX) * this->bpp());
        surfacePt->fX = rect.fLeft;
        surfacePt->fY = rect.fTop;
        fSize = rect.size();
        return true;
    }

    bool isValid() const { return fColorInfo.isValid() && this->width() > 0 && this->height() > 0; }

private:
    GrColorSpaceInfo fColorInfo = {};
    SkISize fSize;
};

// Swizzle param is applied after loading and before converting from srcInfo to dstInfo.
bool GrConvertPixels(const GrPixelInfo& dstInfo,       void* dst, size_t dstRB,
                     const GrPixelInfo& srcInfo, const void* src, size_t srcRB,
                     bool flipY = false);

#endif
