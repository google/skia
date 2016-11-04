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
// matrix operations, as well as have multiple TRC functions. The CLUT also has
// the potential to allow conversion from input color spaces with a different
// number of channels such as CMYK (4) or GRAY (1), but that is not supported yet.
//
// Currently AtoBType A2B0 tag types are supported. There are also lut8Type,
// lut16Type and MPET (multi-processing-elements) A2B0 tags which allow you to
// combine these 3 primitives (TRC, CLUT, matrix) in any order/quantitiy,
// but support for that is not implemented.
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
    
    bool onGammaCloseToSRGB() const override {
        // There is no single gamma curve in an A2B0 profile
        return false;
    }
    
    bool onGammaIsLinear() const override {
        // There is no single gamma curve in an A2B0 profile
        return false;
    }

    sk_sp<SkColorSpace> makeLinearGamma() override {
        // TODO: Analyze the extrema of our projection into XYZ and use suitable primaries?
        // For now, just fall back to a default, because we don't have a good answer.
        return SkColorSpace::MakeNamed(SkColorSpace::kSRGBLinear_Named);
    }

    sk_sp<SkColorSpace> makeSRGBGamma() override {
        // See comment in makeLinearGamma
        return SkColorSpace::MakeNamed(SkColorSpace::kSRGB_Named);
    }

    Type type() const override { return Type::kA2B; }

    class Element {
    public:
        explicit Element(SkGammaNamed gammaNamed)
            : fType(Type::kGammaNamed)
            , fGammaNamed(gammaNamed)
            , fMatrix(SkMatrix44::kUninitialized_Constructor)
        {}

        explicit Element(sk_sp<SkGammas> gammas)
            : fType(Type::kGammas)
            , fGammas(std::move(gammas))
            , fMatrix(SkMatrix44::kUninitialized_Constructor)  
        {}

        explicit Element(sk_sp<SkColorLookUpTable> colorLUT)
            : fType(Type::kCLUT)
            , fCLUT(std::move(colorLUT))
            , fMatrix(SkMatrix44::kUninitialized_Constructor)
        {}

        explicit Element(const SkMatrix44& matrix)
            : fType(Type::kMatrix)
            , fMatrix(matrix)
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

    private:
        Type                      fType;
        SkGammaNamed              fGammaNamed;
        sk_sp<SkGammas>           fGammas;
        sk_sp<SkColorLookUpTable> fCLUT;
        SkMatrix44                fMatrix;
    };
    const Element& element(size_t i) const { return fElements[i]; }
    
    size_t count() const { return fElements.size(); }

    // the intermediate profile connection space that this color space
    // represents the transformation to
    enum class PCS : uint8_t {
        kLAB, // CIELAB
        kXYZ  // CIEXYZ
    };
    
    PCS pcs() const { return fPCS; }

private:
    SkColorSpace_A2B(PCS pcs, sk_sp<SkData> profileData, std::vector<Element> elements);

    PCS                  fPCS;
    std::vector<Element> fElements;

    friend class SkColorSpace;
    typedef SkColorSpace_Base INHERITED;
};

#endif
