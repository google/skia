/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_A2B_DEFINED
#define SkColorSpace_A2B_DEFINED

#include "SkColorSpace_Base.h"

#include <vector>

// An alternative SkColorSpace that represents all the color space data that
// is stored in an A2B0 ICC tag. This allows us to use alternative profile
// connection spaces (CIELAB instead of just CIEXYZ), use color-lookup-tables
// to do color space transformations not representable as TRC functions or
// matrix operations, as well as have multiple TRC functions. The CLUT also
// allows conversion between non-3-channel input color spaces ie CMYK(4) to
// a workable PCS (ie XYZ).
//
// AtoBType, lut8Type and lut16Type A2B0 tag types are supported. There are
// also MPET (multi-processing-elements) A2B0 tags in the standard which allow
// you to combine these 3 primitives (TRC, CLUT, matrix) in any order/quantity.
// MPET tags are currently unsupported by the MakeICC parser, could be supported
// here by the nature of the design.
class SkColorSpace_A2B : public SkColorSpace_Base {
public:
    const SkMatrix44* toXYZD50() const override {
        // the matrix specified in A2B0 profiles is not necessarily
        // a to-XYZ matrix, as to-Lab is supported as well so returning
        // that could be misleading. Additionally, B-curves are applied
        // after the matrix is, but a toXYZD50 matrix is the last thing
        // applied in order to get into the (XYZ) profile connection space.
        return nullptr;
    }

    uint32_t toXYZD50Hash() const override {
        // See toXYZD50()'s comment.
        return 0;
    }

    const SkMatrix44* fromXYZD50() const override {
        // See toXYZD50()'s comment. Also, A2B0 profiles are not supported
        // as destination color spaces, so an inverse matrix is never wanted.
        return nullptr;
    }

    // There is no single gamma curve in an A2B0 profile
    bool onGammaCloseToSRGB() const override { return false; }
    bool onGammaIsLinear() const override { return false; }
    bool onIsNumericalTransferFn(SkColorSpaceTransferFn* coeffs) const override { return false; }

    bool onIsCMYK() const override { return kCMYK_ICCTypeFlag == fICCType; }

    sk_sp<SkColorSpace> makeLinearGamma() const override {
        // TODO: Analyze the extrema of our projection into XYZ and use suitable primaries?
        // For now, just fall back to a default, because we don't have a good answer.
        return SkColorSpace::MakeSRGBLinear();
    }

    sk_sp<SkColorSpace> makeSRGBGamma() const override {
        // See comment in makeLinearGamma
        return SkColorSpace::MakeSRGB();
    }

    Type type() const override { return Type::kA2B; }

    class Element {
    public:
        Element(SkGammaNamed gammaNamed, int channelCount)
            : fType(Type::kGammaNamed)
            , fGammaNamed(gammaNamed)
            , fMatrix(SkMatrix44::kUninitialized_Constructor)
            , fInputChannels(channelCount)
            , fOutputChannels(channelCount) {
            SkASSERT(gammaNamed != kNonStandard_SkGammaNamed);
        }

        explicit Element(sk_sp<SkGammas> gammas)
            : fType(Type::kGammas)
            , fGammas(std::move(gammas))
            , fMatrix(SkMatrix44::kUninitialized_Constructor)
            , fInputChannels(fGammas->channels())
            , fOutputChannels(fGammas->channels()) {
            for (int i = 0; i < fGammas->channels(); ++i) {
                if (SkGammas::Type::kTable_Type == fGammas->type(i)) {
                    SkASSERT(fGammas->data(i).fTable.fSize >= 2);
                }
            }
        }

        explicit Element(sk_sp<SkColorLookUpTable> colorLUT)
            : fType(Type::kCLUT)
            , fCLUT(std::move(colorLUT))
            , fMatrix(SkMatrix44::kUninitialized_Constructor)
            , fInputChannels(fCLUT->inputChannels())
            , fOutputChannels(fCLUT->outputChannels())
        {}

        explicit Element(const SkMatrix44& matrix)
            : fType(Type::kMatrix)
            , fMatrix(matrix)
            , fInputChannels(3)
            , fOutputChannels(3)
        {}

        enum class Type {
            kGammaNamed,
            kGammas,
            kCLUT,
            kMatrix
        };

        Type type() const { return fType; }

        SkGammaNamed gammaNamed() const {
            SkASSERT(Type::kGammaNamed == fType);
            return fGammaNamed;
        }

        const SkGammas& gammas() const {
            SkASSERT(Type::kGammas == fType);
            return *fGammas;
        }

        const SkColorLookUpTable& colorLUT() const {
            SkASSERT(Type::kCLUT == fType);
            return *fCLUT;
        }

        const SkMatrix44& matrix() const {
            SkASSERT(Type::kMatrix == fType);
            return fMatrix;
        }

        int inputChannels() const { return fInputChannels; }

        int outputChannels() const { return fOutputChannels; }

    private:
        Type                      fType;
        SkGammaNamed              fGammaNamed;
        sk_sp<SkGammas>           fGammas;
        sk_sp<SkColorLookUpTable> fCLUT;
        SkMatrix44                fMatrix;
        int                       fInputChannels;
        int                       fOutputChannels;
    };
    const Element& element(int i) const { return fElements[i]; }

    int count() const { return (int)fElements.size(); }

    // the intermediate profile connection space that this color space
    // represents the transformation to
    enum class PCS : uint8_t {
        kLAB, // CIELAB
        kXYZ  // CIEXYZ
    };

    PCS pcs() const { return fPCS; }

    ICCTypeFlag iccType() const { return fICCType; }

    SkColorSpace_A2B(ICCTypeFlag iccType, std::vector<Element> elements, PCS pcs,
                     sk_sp<SkData> profileData);

private:
    ICCTypeFlag          fICCType;
    std::vector<Element> fElements;
    PCS                  fPCS;

    friend class SkColorSpace_Base;
    friend class ColorSpaceXformTest;
    typedef SkColorSpace_Base INHERITED;
};

#endif
