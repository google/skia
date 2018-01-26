/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_XYZ_DEFINED
#define SkColorSpace_XYZ_DEFINED

#include "SkColorSpace.h"
#include "SkData.h"
#include "SkGammas.h"
#include "SkOnce.h"

class SkColorSpace_XYZ : public SkColorSpace {
public:
    const SkMatrix44* onToXYZD50() const override { return &fToXYZD50; }
    uint32_t onToXYZD50Hash() const override { return fToXYZD50Hash; }

    const SkMatrix44* onFromXYZD50() const override;

    bool onGammaCloseToSRGB() const override;
    bool onGammaIsLinear() const override;
    bool onIsNumericalTransferFn(SkColorSpaceTransferFn* coeffs) const override;

    const SkData* onProfileData() const override { return fProfileData.get(); }

    sk_sp<SkColorSpace> makeLinearGamma() const override;
    sk_sp<SkColorSpace> makeSRGBGamma() const override;
    sk_sp<SkColorSpace> makeColorSpin() const override;

    SkGammaNamed onGammaNamed() const override { return fGammaNamed; }

    const SkGammas* gammas() const { return fGammas.get(); }

    void toDstGammaTables(const uint8_t* tables[3], sk_sp<SkData>* storage, int numTables) const;

    SkColorSpace_XYZ(SkGammaNamed gammaNamed, const SkMatrix44& toXYZ);

    SkColorSpace_XYZ(SkGammaNamed gammaNamed, sk_sp<SkGammas> gammas,
                     const SkMatrix44& toXYZ, sk_sp<SkData> profileData);

private:
    sk_sp<SkData>          fProfileData;

    const SkGammaNamed     fGammaNamed;
    sk_sp<SkGammas>        fGammas;
    const SkMatrix44       fToXYZD50;
    uint32_t               fToXYZD50Hash;

    mutable SkMatrix44     fFromXYZD50;
    mutable SkOnce         fFromXYZOnce;

    mutable sk_sp<SkData>  fDstStorage;
    mutable const uint8_t* fToDstGammaTables[3];
    mutable SkOnce         fToDstGammaOnce;

    friend class SkColorSpace;
    friend class ColorSpaceXformTest;
};

#endif
