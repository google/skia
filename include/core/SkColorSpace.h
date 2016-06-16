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

    /**
     *  Create an SkColorSpace from the src gamma and a transform from src gamut to D50 XYZ.
     */
    static sk_sp<SkColorSpace> NewRGB(const float gammas[3], const SkMatrix44& toXYZD50);

    /**
     *  Create a common, named SkColorSpace.
     */
    static sk_sp<SkColorSpace> NewNamed(Named);

    /**
     *  Create an SkColorSpace from an ICC profile.
     */
    static sk_sp<SkColorSpace> NewICC(const void*, size_t);

    enum GammaNamed {
        kLinear_GammaNamed,

        /**
         *  Gamma curve is a close match to the 2.2f exponential curve.  This is by far
         *  the most common gamma, and is used by sRGB and Adobe RGB profiles.
         */
        k2Dot2Curve_GammaNamed,

        /**
         *  Gamma is represented by a look-up table, a parametric curve, or an uncommon
         *  exponential curve.  Or there is an additional pre-processing step before the
         *  applying the gamma.
         */
        kNonStandard_GammaNamed,
    };

    GammaNamed gammaNamed() const { return fGammaNamed; }

    /**
     *  Returns the matrix used to transform src gamut to XYZ D50.
     */
    const SkMatrix44& xyz() const { return fToXYZD50; }

protected:

    SkColorSpace(GammaNamed gammaNamed, const SkMatrix44& toXYZD50, Named named);

    const GammaNamed fGammaNamed;
    const SkMatrix44 fToXYZD50;
    const Named      fNamed;
};

#endif
