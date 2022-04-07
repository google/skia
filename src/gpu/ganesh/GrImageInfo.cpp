/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrImageInfo.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"

#include <utility>

GrImageInfo::GrImageInfo() = default;

GrImageInfo::GrImageInfo(const SkImageInfo& info)
        : fColorInfo(info.colorInfo()), fDimensions(info.dimensions()) {}

GrImageInfo::GrImageInfo(GrColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs, int w, int h)
        : fColorInfo(ct, at, std::move(cs)), fDimensions{w,h} {}

GrImageInfo::GrImageInfo(GrColorType ct, SkAlphaType at, sk_sp<SkColorSpace> cs,
                         const SkISize& dimensions)
        : fColorInfo(ct, at, std::move(cs)), fDimensions(dimensions) {}

GrImageInfo::GrImageInfo(const GrColorInfo& info, const SkISize& dimensions)
        : fColorInfo(info), fDimensions(dimensions) {}

GrImageInfo::GrImageInfo(GrColorInfo&& info, const SkISize& dimensions)
        : fColorInfo(std::move(info)), fDimensions(dimensions) {}

GrImageInfo::GrImageInfo(const GrImageInfo&) = default;
GrImageInfo::GrImageInfo(GrImageInfo&&) = default;
GrImageInfo& GrImageInfo::operator=(const GrImageInfo&) = default;
GrImageInfo& GrImageInfo::operator=(GrImageInfo&&) = default;

GrImageInfo GrImageInfo::makeColorType(GrColorType ct) const {
    return {this->colorInfo().makeColorType(ct), this->dimensions()};
}

GrImageInfo GrImageInfo::makeAlphaType(SkAlphaType at) const {
    return {this->colorType(), at, this->refColorSpace(), this->width(), this->height()};
}

GrImageInfo GrImageInfo::makeColorSpace(sk_sp<SkColorSpace> cs) const {
    return {this->colorType(), this->alphaType(), std::move(cs), this->width(), this->height()};
}

GrImageInfo GrImageInfo::makeDimensions(SkISize dimensions) const {
    return {this->colorType(), this->alphaType(), this->refColorSpace(), dimensions};
}

GrImageInfo GrImageInfo::makeWH(int width, int height) const {
    return {this->colorType(), this->alphaType(), this->refColorSpace(), width, height};
}

sk_sp<SkColorSpace> GrImageInfo::refColorSpace() const { return fColorInfo.refColorSpace(); }
