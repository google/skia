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

/**
 *  Describes a color gamut with primaries and a white point.
 */
struct SK_API SkColorSpacePrimaries {
    float fRX, fRY;
    float fGX, fGY;
    float fBX, fBY;
    float fWX, fWY;

    /**
     *  Convert primaries and a white point to a toXYZD50 matrix, the preferred color gamut
     *  representation of SkColorSpace.
     */
    bool toXYZD50(SkMatrix44* toXYZD50) const;
};

/**
 *  Contains the coefficients for a common transfer function equation, specified as
 *  a transformation from a curved space to linear.
 *
 *  LinearVal = C*InputVal + F        , for 0.0f <= InputVal <  D
 *  LinearVal = (A*InputVal + B)^G + E, for D    <= InputVal <= 1.0f
 *
 *  Function is undefined if InputVal is not in [ 0.0f, 1.0f ].
 *  Resulting LinearVals must be in [ 0.0f, 1.0f ].
 *  Function must be positive and increasing.
 */
struct SK_API SkColorSpaceTransferFn {
    float fG;
    float fA;
    float fB;
    float fC;
    float fD;
    float fE;
    float fF;
};

class SK_API SkColorSpace : public SkRefCnt {
public:

    /**
     *  Create the sRGB color space.
     */
    static sk_sp<SkColorSpace> MakeSRGB();

    /**
     *  Colorspace with the sRGB primaries, but a linear (1.0) gamma. Commonly used for
     *  half-float surfaces, and high precision individual colors (gradient stops, etc...)
     */
    static sk_sp<SkColorSpace> MakeSRGBLinear();

    enum RenderTargetGamma : uint8_t {
        kLinear_RenderTargetGamma,

        /**
         *  Transfer function is the canonical sRGB curve, which has a short linear segment
         *  followed by a 2.4f exponential.
         */
        kSRGB_RenderTargetGamma,
    };

    enum Gamut {
        kSRGB_Gamut,
        kAdobeRGB_Gamut,
        kDCIP3_D65_Gamut,
        kRec2020_Gamut,
    };

    /**
     *  Create an SkColorSpace from a transfer function and a color gamut.
     *
     *  Transfer function can be specified as an enum or as the coefficients to an equation.
     *  Gamut can be specified as an enum or as the matrix transformation to XYZ D50.
     */
    static sk_sp<SkColorSpace> MakeRGB(RenderTargetGamma gamma, Gamut gamut);
    static sk_sp<SkColorSpace> MakeRGB(RenderTargetGamma gamma, const SkMatrix44& toXYZD50);
    static sk_sp<SkColorSpace> MakeRGB(const SkColorSpaceTransferFn& coeffs, Gamut gamut);
    static sk_sp<SkColorSpace> MakeRGB(const SkColorSpaceTransferFn& coeffs,
                                       const SkMatrix44& toXYZD50);

    /**
     *  Create an SkColorSpace from an ICC profile.
     */
    static sk_sp<SkColorSpace> MakeICC(const void*, size_t);

    /**
     *  Returns true if the color space gamma is near enough to be approximated as sRGB.
     */
    bool gammaCloseToSRGB() const;

    /**
     *  Returns true if the color space gamma is linear.
     */
    bool gammaIsLinear() const;

    /**
     *  Returns true and sets |toXYZD50| if the color gamut can be described as a matrix.
     *  Returns false otherwise.
     */
    bool toXYZD50(SkMatrix44* toXYZD50) const;

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
