/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageInfo_DEFINED
#define GrImageInfo_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrColorInfo.h"

class GrImageInfo {
public:
    GrImageInfo() = default;

    /* implicit */ GrImageInfo(const SkImageInfo& info)
            : fColorInfo(info.colorInfo()), fDimensions(info.dimensions()) {}

    GrImageInfo(GrColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs, int w, int h)
            : fColorInfo(ct, at, std::move(cs)), fDimensions{w,h} {}

    GrImageInfo(GrColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs, const SkISize& dimensions)
            : fColorInfo(ct, at, std::move(cs)), fDimensions(dimensions) {}

    GrImageInfo(const GrImageInfo&) = default;
    GrImageInfo(GrImageInfo&&) = default;
    GrImageInfo& operator=(const GrImageInfo&) = default;
    GrImageInfo& operator=(GrImageInfo&&) = default;

    GrImageInfo makeColorType(GrColorType ct) {
        return {ct, this->alphaType(), this->refColorSpace(), this->width(), this->height()};
    }

    GrImageInfo makeAlphaType(SkAlphaType at) {
        return {this->colorType(), at, this->refColorSpace(), this->width(), this->height()};
    }

    GrImageInfo makeWH(int width, int height) {
        return {this->colorType(), this->alphaType(), this->refColorSpace(), width, height};
    }

    GrColorType colorType() const { return fColorInfo.colorType(); }

    SkAlphaType alphaType() const { return fColorInfo.alphaType(); }

    SkColorSpace* colorSpace() const { return fColorInfo.colorSpace(); }

    sk_sp<SkColorSpace> refColorSpace() const { return fColorInfo.refColorSpace(); }

    SkISize dimensions() const { return fDimensions; }

    int width() const { return fDimensions.width(); }

    int height() const { return fDimensions.height(); }

    size_t bpp() const { return GrColorTypeBytesPerPixel(this->colorType()); }

    size_t minRowBytes() const { return this->bpp() * this->width(); }

    /**
     * Place this image rect in a surface of dimensions surfaceWidth x surfaceHeight size offset at
     * surfacePt and then clip the pixel rectangle to the bounds of the surface. If the pixel rect
     * does not intersect the rectangle or is empty then return false. If clipped, the input
     * surfacePt, the width/height of this GrImageInfo, and the data pointer will be modified to
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
        fDimensions = rect.size();
        return true;
    }

    bool isValid() const { return fColorInfo.isValid() && this->width() > 0 && this->height() > 0; }

private:
    GrColorInfo fColorInfo = {};
    SkISize fDimensions;
};

#endif
