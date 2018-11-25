/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "SkColorSpaceXform_Base.h"
#include "SkOpts.h"

SkColorSpace_XYZ::SkColorSpace_XYZ(SkGammaNamed gammaNamed, const SkMatrix44& toXYZD50)
    : fProfileData(nullptr)
    , fGammaNamed(gammaNamed)
    , fGammas(nullptr)
    , fToXYZD50(toXYZD50)
    , fToXYZD50Hash(SkOpts::hash_fn(toXYZD50.values(), 16 * sizeof(SkMScalar), 0))
    , fFromXYZD50(SkMatrix44::kUninitialized_Constructor)
{}

SkColorSpace_XYZ::SkColorSpace_XYZ(SkGammaNamed gammaNamed, sk_sp<SkGammas> gammas,
                                   const SkMatrix44& toXYZD50, sk_sp<SkData> profileData)
    : fProfileData(std::move(profileData))
    , fGammaNamed(gammaNamed)
    , fGammas(std::move(gammas))
    , fToXYZD50(toXYZD50)
    , fToXYZD50Hash(SkOpts::hash_fn(toXYZD50.values(), 16 * sizeof(SkMScalar), 0))
    , fFromXYZD50(SkMatrix44::kUninitialized_Constructor) {
    SkASSERT(!fGammas || 3 == fGammas->channels());
    if (fGammas) {
        for (int i = 0; i < fGammas->channels(); ++i) {
            if (SkGammas::Type::kTable_Type == fGammas->type(i)) {
                SkASSERT(fGammas->data(i).fTable.fSize >= 2);
            }
        }
    }
}

const SkMatrix44* SkColorSpace_XYZ::onFromXYZD50() const {
    fFromXYZOnce([this] {
        if (!fToXYZD50.invert(&fFromXYZD50)) {
            // If a client gives us a dst gamut with a transform that we can't invert, we will
            // simply give them back a transform to sRGB gamut.
            SkDEBUGFAIL("Non-invertible XYZ matrix, defaulting to sRGB");
            SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
            srgbToxyzD50.set3x3RowMajorf(gSRGB_toXYZD50);
            srgbToxyzD50.invert(&fFromXYZD50);
        }
    });
    return &fFromXYZD50;
}

bool SkColorSpace_XYZ::onGammaCloseToSRGB() const {
    return kSRGB_SkGammaNamed == fGammaNamed || k2Dot2Curve_SkGammaNamed == fGammaNamed;
}

bool SkColorSpace_XYZ::onGammaIsLinear() const {
    return kLinear_SkGammaNamed == fGammaNamed;
}

bool SkColorSpace_XYZ::onIsNumericalTransferFn(SkColorSpaceTransferFn* coeffs) const {
    if (named_to_parametric(coeffs, fGammaNamed)) {
        return true;
    }

    SkASSERT(fGammas);
    if (!fGammas->allChannelsSame()) {
        return false;
    }

    if (fGammas->isValue(0)) {
        value_to_parametric(coeffs, fGammas->data(0).fValue);
        return true;
    }

    if (fGammas->isParametric(0)) {
        *coeffs = fGammas->params(0);
        return true;
    }

    return false;
}

sk_sp<SkColorSpace> SkColorSpace_XYZ::makeLinearGamma() const {
    if (this->gammaIsLinear()) {
        return sk_ref_sp(const_cast<SkColorSpace_XYZ*>(this));
    }
    return SkColorSpace::MakeRGB(kLinear_SkGammaNamed, fToXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace_XYZ::makeSRGBGamma() const {
    if (this->gammaCloseToSRGB()) {
        return sk_ref_sp(const_cast<SkColorSpace_XYZ*>(this));
    }
    return SkColorSpace::MakeRGB(kSRGB_SkGammaNamed, fToXYZD50);
}

sk_sp<SkColorSpace> SkColorSpace_XYZ::makeColorSpin() const {
    SkMatrix44 spin(SkMatrix44::kUninitialized_Constructor);
    spin.set3x3(0, 1, 0, 0, 0, 1, 1, 0, 0);
    spin.postConcat(fToXYZD50);
    return sk_sp<SkColorSpace>(new SkColorSpace_XYZ(fGammaNamed, fGammas, spin, fProfileData));
}

void SkColorSpace_XYZ::toDstGammaTables(const uint8_t* tables[3], sk_sp<SkData>* storage,
                                         int numTables) const {
    fToDstGammaOnce([this, numTables] {
        const bool gammasAreMatching = numTables <= 1;
        fDstStorage =
                SkData::MakeUninitialized(numTables * SkColorSpaceXform_Base::kDstGammaTableSize);
        SkColorSpaceXform_Base::BuildDstGammaTables(fToDstGammaTables,
                                                    (uint8_t*) fDstStorage->writable_data(), this,
                                                    gammasAreMatching);
    });

    *storage = fDstStorage;
    tables[0] = fToDstGammaTables[0];
    tables[1] = fToDstGammaTables[1];
    tables[2] = fToDstGammaTables[2];
}
