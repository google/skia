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

    GrImageInfo(const GrColorInfo& info, const SkISize& dimensions)
            : fColorInfo(info), fDimensions(dimensions) {}

    GrImageInfo(GrColorInfo&& info, const SkISize& dimensions)
            : fColorInfo(std::move(info)), fDimensions(dimensions) {}

    GrImageInfo(const GrImageInfo&) = default;
    GrImageInfo(GrImageInfo&&) = default;
    GrImageInfo& operator=(const GrImageInfo&) = default;
    GrImageInfo& operator=(GrImageInfo&&) = default;

    GrImageInfo makeColorType(GrColorType ct) const {
        return {ct, this->alphaType(), this->refColorSpace(), this->width(), this->height()};
    }

    GrImageInfo makeAlphaType(SkAlphaType at) const {
        return {this->colorType(), at, this->refColorSpace(), this->width(), this->height()};
    }

    GrImageInfo makeDimensions(SkISize dimensions) const {
        return {this->colorType(), this->alphaType(), this->refColorSpace(), dimensions};
    }

    GrImageInfo makeWH(int width, int height) const {
        return {this->colorType(), this->alphaType(), this->refColorSpace(), width, height};
    }

    const GrColorInfo& colorInfo() const { return fColorInfo; }

    GrColorType colorType() const { return fColorInfo.colorType(); }

    SkAlphaType alphaType() const { return fColorInfo.alphaType(); }

    SkColorSpace* colorSpace() const { return fColorInfo.colorSpace(); }

    sk_sp<SkColorSpace> refColorSpace() const { return fColorInfo.refColorSpace(); }

    SkISize dimensions() const { return fDimensions; }

    int width() const { return fDimensions.width(); }

    int height() const { return fDimensions.height(); }

    size_t bpp() const { return GrColorTypeBytesPerPixel(this->colorType()); }

    size_t minRowBytes() const { return this->bpp() * this->width(); }

    bool isValid() const { return fColorInfo.isValid() && this->width() > 0 && this->height() > 0; }

private:
    GrColorInfo fColorInfo = {};
    SkISize fDimensions;
};

#endif
