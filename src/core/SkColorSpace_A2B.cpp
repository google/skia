/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_A2B.h"

SkColorSpace_A2B::SkColorSpace_A2B(SkGammaNamed aCurveNamed, sk_sp<SkGammas> aCurve,
                                   sk_sp<SkColorLookUpTable> colorLUT,
                                   SkGammaNamed mCurveNamed, sk_sp<SkGammas> mCurve,
                                   const SkMatrix44& matrix,
                                   SkGammaNamed bCurveNamed, sk_sp<SkGammas> bCurve,
                                   PCS pcs, sk_sp<SkData> profileData)
    : INHERITED(std::move(profileData))
    , fACurveNamed(aCurveNamed)
    , fACurve(std::move(aCurve))
    , fColorLUT(std::move(colorLUT))
    , fMCurveNamed(mCurveNamed)
    , fMCurve(std::move(mCurve))
    , fMatrix(matrix)
    , fBCurveNamed(bCurveNamed)
    , fBCurve(std::move(bCurve))
    , fPCS(pcs)
{}
