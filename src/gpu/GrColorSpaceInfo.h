/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorSpaceInfo_DEFINED
#define GrColorSpaceInfo_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/GrColorSpaceXform.h"

/** Describes the color space properties of a surface context. */
class GrColorSpaceInfo {
public:
    GrColorSpaceInfo(SkAlphaType, sk_sp<SkColorSpace>, GrPixelConfig);

    bool isLinearlyBlended() const { return fColorSpace && fColorSpace->gammaIsLinear(); }

    SkColorSpace* colorSpace() const { return fColorSpace.get(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fColorSpace; }

    SkAlphaType alphaType() const { return fAlphaType; }

    GrColorSpaceXform* colorSpaceXformFromSRGB() const;
    sk_sp<GrColorSpaceXform> refColorSpaceXformFromSRGB() const {
        return sk_ref_sp(this->colorSpaceXformFromSRGB());
    }

    // TODO: Remove or replace with SkColorType
    GrPixelConfig config() const { return fConfig; }

private:
    sk_sp<SkColorSpace> fColorSpace;
    SkAlphaType fAlphaType;
    mutable sk_sp<GrColorSpaceXform> fColorXformFromSRGB;
    GrPixelConfig fConfig;
    mutable bool fInitializedColorSpaceXformFromSRGB;
};

#endif
