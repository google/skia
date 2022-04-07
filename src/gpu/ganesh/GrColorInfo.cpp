/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrColorInfo.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "src/core/SkColorSpacePriv.h"

#include <utility>

GrColorInfo::GrColorInfo(
        GrColorType colorType, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace)
        : fColorSpace(std::move(colorSpace)), fColorType(colorType), fAlphaType(alphaType) {
    // sRGB sources are very common (SkColor, etc...), so we cache that transformation
    fColorXformFromSRGB = GrColorSpaceXform::Make(sk_srgb_singleton(), kUnpremul_SkAlphaType,
                                                  fColorSpace.get(),   kUnpremul_SkAlphaType);
}

GrColorInfo::GrColorInfo(const SkColorInfo& ci)
        : GrColorInfo(SkColorTypeToGrColorType(ci.colorType()),
                      ci.alphaType(),
                      ci.refColorSpace()) {}

GrColorInfo::GrColorInfo() = default;
GrColorInfo::GrColorInfo(const GrColorInfo&) = default;
GrColorInfo& GrColorInfo::operator=(const GrColorInfo&) = default;
GrColorInfo::~GrColorInfo() = default;

bool GrColorInfo::operator==(const GrColorInfo& that) const {
    return fColorType == that.fColorType &&
           fAlphaType == that.fAlphaType &&
           SkColorSpace::Equals(fColorSpace.get(), that.fColorSpace.get());
}

GrColorInfo GrColorInfo::makeColorType(GrColorType ct) const {
    return GrColorInfo(ct, fAlphaType, this->refColorSpace());
}

bool GrColorInfo::isLinearlyBlended() const {
    return fColorSpace && fColorSpace->gammaIsLinear();
}

SkColorSpace* GrColorInfo::colorSpace() const { return fColorSpace.get(); }
sk_sp<SkColorSpace> GrColorInfo::refColorSpace() const { return fColorSpace; }
