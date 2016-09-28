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
    enum Named : uint8_t {
        /**
         *  By far the most common color space.
         *  This is the default space for images, unmarked content, and monitors.
         */
        kSRGB_Named,

        /**
         *  Very common wide gamut color space.
         *  Often used by images and monitors.
         */
        kAdobeRGB_Named,

        /**
         *  Colorspace with the sRGB primaries, but a linear (1.0) gamma. Commonly used for
         *  half-float surfaces, and high precision individual colors (gradient stops, etc...)
         */
        kSRGBLinear_Named,
    };

    enum RenderTargetGamma : uint8_t {
        kLinear_RenderTargetGamma,

        /**
         *  Transfer function is the canonical sRGB curve, which has a short linear segment
         *  followed by a 2.4f exponential.
         */
        kSRGB_RenderTargetGamma,
    };

    /**
     *  Create an SkColorSpace from a transfer function and a color gamut transform to D50 XYZ.
     */
    static sk_sp<SkColorSpace> NewRGB(RenderTargetGamma gamma, const SkMatrix44& toXYZD50);

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

    /**
     *  Returns true if the color space gamma is near enough to be approximated as sRGB.
     */
    bool gammaCloseToSRGB() const;

    /**
     *  Returns true if the color space gamma is linear.
     */
    bool gammaIsLinear() const;

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
    SkColorSpace() {}
};

#endif
