/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_Base_DEFINED
#define SkColorSpace_Base_DEFINED

#include "SkColorSpace.h"
#include "SkData.h"
#include "SkTemplates.h"

struct SkGammaCurve {
    bool isNamed() const {
       bool result = (SkColorSpace::kNonStandard_GammaNamed != fNamed);
       SkASSERT(!result || (0.0f == fValue));
       SkASSERT(!result || (0 == fTableSize));
       SkASSERT(!result || (0.0f == fG && 0.0f == fE));
       return result;
    }

    bool isValue() const {
        bool result = (0.0f != fValue);
        SkASSERT(!result || SkColorSpace::kNonStandard_GammaNamed == fNamed);
        SkASSERT(!result || (0 == fTableSize));
        SkASSERT(!result || (0.0f == fG && 0.0f == fE));
        return result;
    }

    bool isTable() const {
        bool result = (0 != fTableSize);
        SkASSERT(!result || SkColorSpace::kNonStandard_GammaNamed == fNamed);
        SkASSERT(!result || (0.0f == fValue));
        SkASSERT(!result || (0.0f == fG && 0.0f == fE));
        SkASSERT(!result || fTable);
        return result;
    }

    bool isParametric() const {
        bool result = (0.0f != fG || 0.0f != fE);
        SkASSERT(!result || SkColorSpace::kNonStandard_GammaNamed == fNamed);
        SkASSERT(!result || (0.0f == fValue));
        SkASSERT(!result || (0 == fTableSize));
        return result;
    }

    // We have four different ways to represent gamma.
    // (1) A known, named type:
    SkColorSpace::GammaNamed fNamed;

    // (2) A single value:
    float                    fValue;

    // (3) A lookup table:
    uint32_t                 fTableSize;
    std::unique_ptr<float[]> fTable;

    // (4) Parameters for a curve:
    //     Y = (aX + b)^g + c  for X >= d
    //     Y = eX + f          otherwise
    float                    fG;
    float                    fA;
    float                    fB;
    float                    fC;
    float                    fD;
    float                    fE;
    float                    fF;

    SkGammaCurve()
        : fNamed(SkColorSpace::kNonStandard_GammaNamed)
        , fValue(0.0f)
        , fTableSize(0)
        , fTable(nullptr)
        , fG(0.0f)
        , fA(0.0f)
        , fB(0.0f)
        , fC(0.0f)
        , fD(0.0f)
        , fE(0.0f)
        , fF(0.0f)
    {}

    bool quickEquals(const SkGammaCurve& that) const {
        return (this->fNamed == that.fNamed) && (this->fValue == that.fValue) &&
                (this->fTableSize == that.fTableSize) && (this->fTable == that.fTable) &&
                (this->fG == that.fG) && (this->fA == that.fA) && (this->fB == that.fB) &&
                (this->fC == that.fC) && (this->fD == that.fD) && (this->fE == that.fE) &&
                (this->fF == that.fF);
    }
};

struct SkGammas : public SkRefCnt {
public:
    static SkColorSpace::GammaNamed Named(SkGammaCurve curves[3]) {
        if (SkColorSpace::kLinear_GammaNamed == curves[0].fNamed &&
            SkColorSpace::kLinear_GammaNamed == curves[1].fNamed &&
            SkColorSpace::kLinear_GammaNamed == curves[2].fNamed)
        {
            return SkColorSpace::kLinear_GammaNamed;
        }

        if (SkColorSpace::kSRGB_GammaNamed == curves[0].fNamed &&
            SkColorSpace::kSRGB_GammaNamed == curves[1].fNamed &&
            SkColorSpace::kSRGB_GammaNamed == curves[2].fNamed)
        {
            return SkColorSpace::kSRGB_GammaNamed;
        }

        if (SkColorSpace::k2Dot2Curve_GammaNamed == curves[0].fNamed &&
            SkColorSpace::k2Dot2Curve_GammaNamed == curves[1].fNamed &&
            SkColorSpace::k2Dot2Curve_GammaNamed == curves[2].fNamed)
        {
            return SkColorSpace::k2Dot2Curve_GammaNamed;
        }

        return SkColorSpace::kNonStandard_GammaNamed;
    }

    const SkGammaCurve& operator[](int i) const {
        SkASSERT(0 <= i && i < 3);
        return (&fRed)[i];
    }

    const SkGammaCurve fRed;
    const SkGammaCurve fGreen;
    const SkGammaCurve fBlue;

    SkGammas(SkGammaCurve red, SkGammaCurve green, SkGammaCurve blue)
        : fRed(std::move(red))
        , fGreen(std::move(green))
        , fBlue(std::move(blue))
    {}

    SkGammas() {}

    friend class SkColorSpace;
};

struct SkColorLookUpTable : public SkRefCnt {
    uint8_t                  fInputChannels;
    uint8_t                  fOutputChannels;
    uint8_t                  fGridPoints[3];
    std::unique_ptr<float[]> fTable;

    SkColorLookUpTable()
        : fInputChannels(0)
        , fOutputChannels(0)
        , fTable(nullptr)
    {
        fGridPoints[0] = fGridPoints[1] = fGridPoints[2] = 0;
    }
};

class SkColorSpace_Base : public SkColorSpace {
public:

    static sk_sp<SkColorSpace> NewRGB(float gammas[3], const SkMatrix44& toXYZD50);

    const SkGammas* gammas() const { return fGammas.get(); }

    const SkColorLookUpTable* colorLUT() const { return fColorLUT.get(); }

    /**
     *  Writes this object as an ICC profile.
     */
    sk_sp<SkData> writeToICC() const;

private:

    static sk_sp<SkColorSpace> NewRGB(GammaNamed gammaNamed, const SkMatrix44& toXYZD50);

    SkColorSpace_Base(GammaNamed gammaNamed, const SkMatrix44& toXYZ, Named named);

    SkColorSpace_Base(sk_sp<SkColorLookUpTable> colorLUT, sk_sp<SkGammas> gammas,
                      const SkMatrix44& toXYZ, sk_sp<SkData> profileData);

    sk_sp<SkColorLookUpTable> fColorLUT;
    sk_sp<SkGammas>           fGammas;
    sk_sp<SkData>             fProfileData;

    friend class SkColorSpace;
    friend class ColorSpaceXformTest;
    typedef SkColorSpace INHERITED;
};

static inline SkColorSpace_Base* as_CSB(SkColorSpace* colorSpace) {
    return static_cast<SkColorSpace_Base*>(colorSpace);
}

static inline const SkColorSpace_Base* as_CSB(const SkColorSpace* colorSpace) {
    return static_cast<const SkColorSpace_Base*>(colorSpace);
}

static inline SkColorSpace_Base* as_CSB(const sk_sp<SkColorSpace>& colorSpace) {
    return static_cast<SkColorSpace_Base*>(colorSpace.get());
}

#endif
