/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_A2B_DEFINED
#define SkColorSpace_A2B_DEFINED

#include "SkColorSpace_Base.h"

// An alternative SkColorSpace that represents all the color space data that
// is stored in an A2B0 ICC tag. This allows us to use alternative profile
// connection spaces (CIELAB instead of just CIEXYZ), use color-lookup-tables
// to do color space transformations not representable as TRC functions or
// matrix operations, as well as have multiple TRC functions. The CLUT also has
// the potential to allow conversion from input color spaces with a different
// number of channels such as CMYK (4) or GRAY (1), but that is not supported yet.
//
// Evaluation is done: A-curve => CLUT => M-curve => Matrix => B-curve
//
// There are also multi-processing-elements in the A2B0 tag which allow you to
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

    SkGammaNamed aCurveNamed() const { return fACurveNamed; }

    const SkGammas* aCurve() const { return fACurve.get(); }

    const SkColorLookUpTable* colorLUT() const { return fColorLUT.get(); }

    SkGammaNamed mCurveNamed() const { return fMCurveNamed; }
    
    const SkGammas* mCurve() const { return fMCurve.get(); }

    const SkMatrix44& matrix() const { return fMatrix; }

    SkGammaNamed bCurveNamed() const { return fBCurveNamed; }
    
    const SkGammas* bCurve() const { return fBCurve.get(); }

    // the intermediate profile connection space that this color space
    // represents the transformation to
    enum class PCS : uint8_t {
        kLAB, // CIELAB
        kXYZ  // CIEXYZ
    };
    
    PCS pcs() const { return fPCS; }

    Type type() const override { return Type::kA2B; }

private:
    SkColorSpace_A2B(SkGammaNamed aCurveNamed, sk_sp<SkGammas> aCurve,
                     sk_sp<SkColorLookUpTable> colorLUT,
                     SkGammaNamed mCurveNamed, sk_sp<SkGammas> mCurve,
                     const SkMatrix44& matrix,
                     SkGammaNamed bCurveNamed, sk_sp<SkGammas> bCurve,
                     PCS pcs, sk_sp<SkData> profileData);

    const SkGammaNamed        fACurveNamed;
    sk_sp<SkGammas>           fACurve;
    sk_sp<SkColorLookUpTable> fColorLUT;
    const SkGammaNamed        fMCurveNamed;
    sk_sp<SkGammas>           fMCurve;
    SkMatrix44                fMatrix;
    const SkGammaNamed        fBCurveNamed;
    sk_sp<SkGammas>           fBCurve;
    PCS                       fPCS;
    
    friend class SkColorSpace;
    typedef SkColorSpace_Base INHERITED;
};

#endif
