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
         *  exponential curve.  Or the R, G, and B gammas do not match.
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

    /**
     *  Create an SkColorSpace with the same gamut as this color space, but with linear gamma.
     */
    sk_sp<SkColorSpace> makeLinearGamma();

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
     *  To be used only by UMA code.
     */
    bool gammasAreMatching() const;
    bool gammasAreNamed() const;
    bool gammasAreValues() const;
    bool gammasAreTables() const;
    bool gammasAreParams() const;

    /**
     *  Returns nullptr on failure.  Fails when we fallback to serializing ICC data and
     *  the data is too large to serialize.
     */
    sk_sp<SkData> serialize() const;

    /**
     *  If |memory| is nullptr, returns the size required to serialize.
     *  Otherwise, serializes into |memory| and returns the size.
     */
    size_t writeToMemory(void* memory) const;

    static sk_sp<SkColorSpace> Deserialize(const void* data, size_t length);

    /**
     *  If both are null, we return true.  If one is null and the other is not, we return false.
     *  If both are non-null, we do a deeper compare.
     */
    static bool Equals(const SkColorSpace* src, const SkColorSpace* dst);

protected:
    SkColorSpace(GammaNamed gammaNamed, const SkMatrix44& toXYZD50, Named named);

    const GammaNamed fGammaNamed;
    const SkMatrix44 fToXYZD50;
    const Named      fNamed;
};

#endif
