/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImageInfo_DEFINED
#define GrImageInfo_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrColorInfo.h"

#include <cstddef>

class SkColorSpace;
enum SkAlphaType : int;
struct SkImageInfo;

class GrImageInfo {
public:
    GrImageInfo();
    GrImageInfo(const SkImageInfo& info);
    GrImageInfo(GrColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs, int w, int h);
    GrImageInfo(GrColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs, const SkISize& dimensions);
    GrImageInfo(const GrColorInfo& info, const SkISize& dimensions);
    GrImageInfo(GrColorInfo&& info, const SkISize& dimensions);

    GrImageInfo(const GrImageInfo&);
    GrImageInfo(GrImageInfo&&);
    GrImageInfo& operator=(const GrImageInfo&);
    GrImageInfo& operator=(GrImageInfo&&);

    GrImageInfo makeColorType(GrColorType ct) const;
    GrImageInfo makeAlphaType(SkAlphaType at) const;
    GrImageInfo makeColorSpace(sk_sp<SkColorSpace> cs) const;
    GrImageInfo makeDimensions(SkISize dimensions) const ;
    GrImageInfo makeWH(int width, int height) const;

    const GrColorInfo& colorInfo() const { return fColorInfo; }

    GrColorType colorType() const { return fColorInfo.colorType(); }

    SkAlphaType alphaType() const { return fColorInfo.alphaType(); }

    SkColorSpace* colorSpace() const { return fColorInfo.colorSpace(); }

    sk_sp<SkColorSpace> refColorSpace() const;

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
