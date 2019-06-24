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
    GrColorSpaceInfo(GrColorType, SkAlphaType, sk_sp<SkColorSpace>, GrPixelConfig);

    bool isLinearlyBlended() const { return fColorSpace && fColorSpace->gammaIsLinear(); }

    SkColorSpace* colorSpace() const { return fColorSpace.get(); }
    sk_sp<SkColorSpace> refColorSpace() const { return fColorSpace; }

    GrColorSpaceXform* colorSpaceXformFromSRGB() const;
    sk_sp<GrColorSpaceXform> refColorSpaceXformFromSRGB() const {
        return sk_ref_sp(this->colorSpaceXformFromSRGB());
    }

    GrColorType colorType() const { return fColorType; }
    SkAlphaType alphaType() const { return fAlphaType; }

    // TODO: Remove.
    GrPixelConfig config() const { return fConfig; }

private:
    sk_sp<SkColorSpace> fColorSpace;
    mutable sk_sp<GrColorSpaceXform> fColorXformFromSRGB;
    GrColorType fColorType;
    SkAlphaType fAlphaType;
    GrPixelConfig fConfig;
    mutable bool fInitializedColorSpaceXformFromSRGB;
};

#endif
