/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_DEFINED
#define SkColorSpace_DEFINED

#include "SkMatrix44.h"
#include "SkRefCnt.h"

class SkData;

class SK_API SkColorSpace : public SkRefCnt {
public:

    /**
     *  Common, named profiles that we can recognize.
     */
    enum Named {
        kUnknown_Named,
        kSRGB_Named,
        kAdobeRGB_Named,
    };

    enum GammaNamed {
        kLinear_GammaNamed,

        /**
         *  Gamma curve is a close match to the canonical sRGB curve, which has
         *  a short linear segment followed by a 2.4f exponential.
         */
        kSRGB_GammaNamed,

        /**
         *  Gamma curve is a close match to the 2.2f exponential curve.  This is
         *  used by Adobe RGB profiles and is common on monitors as well.
         */
        k2Dot2Curve_GammaNamed,

        /**
         *  Gamma is represented by a look-up table, a parametric curve, or an uncommon
         *  exponential curve.  Or there is an additional pre-processing step before the
         *  applying the gamma.
         */
        kNonStandard_GammaNamed,
    };

    /**
     *  Create an SkColorSpace from the src gamma and a transform from src gamut to D50 XYZ.
     */
    static sk_sp<SkColorSpace> NewRGB(GammaNamed gammaNamed, const SkMatrix44& toXYZD50);

    /**
     *  Create a common, named SkColorSpace.
     */
    static sk_sp<SkColorSpace> NewNamed(Named);

    /**
     *  Create an SkColorSpace from an ICC profile.
     */
    static sk_sp<SkColorSpace> NewICC(const void*, size_t);

    GammaNamed gammaNamed() const { return fGammaNamed; }

    /**
     *  Returns the matrix used to transform src gamut to XYZ D50.
     */
    const SkMatrix44& xyz() const { return fToXYZD50; }

    /**
     *  Returns true if the color space gamma is near enough to be approximated as sRGB.
     */
    bool gammaCloseToSRGB() const {
        return kSRGB_GammaNamed == fGammaNamed || k2Dot2Curve_GammaNamed == fGammaNamed;
    }

    /**
     *  Returns nullptr on failure.  Fails when we fallback to serializing ICC data and
     *  the data is too large to serialize.
     */
    sk_sp<SkData> serialize() const;

    static sk_sp<SkColorSpace> Deserialize(const void* data, size_t length);

protected:
    SkColorSpace(GammaNamed gammaNamed, const SkMatrix44& toXYZD50, Named named);

    const GammaNamed fGammaNamed;
    const SkMatrix44 fToXYZD50;
    const Named      fNamed;
};

#endif
