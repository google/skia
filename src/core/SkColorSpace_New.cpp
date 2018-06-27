/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_New.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"

// ~~~~~~~~~~~~~~~~~~~~~~~ SkColorSpace_New::TransferFn ~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

namespace {

    struct LinearTransferFn : public SkColorSpace_New::TransferFn {
        SkColorSpaceTransferFn parameterize() const override {
            return { 1,1, 0,0,0,0,0 };
        }

        void linearizeDst(SkRasterPipeline*) const override {}
        void linearizeSrc(SkRasterPipeline*) const override {}
        void    encodeSrc(SkRasterPipeline*) const override {}
    };

    struct SRGBTransferFn : public SkColorSpace_New::TransferFn {
        SkColorSpaceTransferFn parameterize() const override {
            return { 2.4f, 1/1.055f, 0.055f/1.055f, 1/12.92f, 0.04045f, 0, 0 };
        }

        void linearizeDst(SkRasterPipeline* p) const override {
            p->append(SkRasterPipeline::from_srgb_dst);
        }
        void linearizeSrc(SkRasterPipeline* p) const override {
            p->append(SkRasterPipeline::from_srgb);
        }
        void encodeSrc(SkRasterPipeline* p) const override {
            p->append(SkRasterPipeline::to_srgb);
        }
    };

    struct GammaTransferFn : public SkColorSpace_New::TransferFn {
        float fGamma;
        float fInv;

        explicit GammaTransferFn(float gamma) : fGamma(gamma), fInv(1.0f/gamma) {}

        SkColorSpaceTransferFn parameterize() const override {
            return { fGamma, 1, 0,0,0,0,0 };
        }

        void linearizeDst(SkRasterPipeline* p) const override {
            p->append(SkRasterPipeline::gamma_dst, &fGamma);
        }
        void linearizeSrc(SkRasterPipeline* p) const override {
            p->append(SkRasterPipeline::gamma, &fGamma);
        }
        void encodeSrc(SkRasterPipeline* p) const override {
            p->append(SkRasterPipeline::gamma, &fInv);
        }
    };

}

sk_sp<SkColorSpace_New::TransferFn> SkColorSpace_New::TransferFn::MakeLinear() {
    return sk_make_sp<LinearTransferFn>();
}
sk_sp<SkColorSpace_New::TransferFn> SkColorSpace_New::TransferFn::MakeSRGB() {
    return sk_make_sp<SRGBTransferFn>();
}
sk_sp<SkColorSpace_New::TransferFn> SkColorSpace_New::TransferFn::MakeGamma(float gamma) {
    if (gamma == 1) {
        return MakeLinear();
    }
    return sk_make_sp<GammaTransferFn>(gamma);
}

bool SkColorSpace_New::TransferFn::equals(const SkColorSpace_New::TransferFn& other) const {
    SkColorSpaceTransferFn a = this->parameterize(),
                           b = other.parameterize();
    return 0 == memcmp(&a,&b, sizeof(SkColorSpaceTransferFn));
}

void SkColorSpace_New::TransferFn::updateICCProfile(ICCProfile*) const {
    // TODO
}

// ~~~~~~~~~~~~~~~~~~~~~~~ SkColorSpace_New ~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

SkColorSpace_New::SkColorSpace_New(sk_sp<TransferFn> transferFn,
                                   SkMatrix44 toXYZD50,
                                   Blending blending)
    : fTransferFn(std::move(transferFn))
    , fFromXYZD50(SkMatrix44::kUninitialized_Constructor)
    , fToXYZD50(toXYZD50)
    , fToXYZD50Hash(SkOpts::hash_fn(&toXYZD50, 16*sizeof(SkMScalar), 0))
    , fBlending(blending)
{
    // It's pretty subtle what do to if the to-XYZ matrix is not invertible.
    // That means the same point in XYZ is mapped to from more than one point in RGB,
    // or put another way, we threw information away when mapping RGB -> XYZ.
    //
    // We'd probably like to set fToXYZD50 as one of the family of matrices that
    // will correctly roundtrip XYZ -> RGB -> XYZ.  Choosing which is an open problem.
    SkAssertResult(fToXYZD50.invert(&fFromXYZD50));
}

sk_sp<SkColorSpace> SkColorSpace_New::makeLinearGamma() const {
    return sk_make_sp<SkColorSpace_New>(TransferFn::MakeLinear(), fToXYZD50, fBlending);
}
sk_sp<SkColorSpace> SkColorSpace_New::makeSRGBGamma() const {
    return sk_make_sp<SkColorSpace_New>(TransferFn::MakeSRGB(), fToXYZD50, fBlending);
}

SkGammaNamed SkColorSpace_New::onGammaNamed() const {
    return kNonStandard_SkGammaNamed;  // TODO
}

bool SkColorSpace_New::onGammaCloseToSRGB() const {
    return fTransferFn->equals(*TransferFn::MakeSRGB());  // TODO: more efficient?
}

bool SkColorSpace_New::onGammaIsLinear() const {
    return fTransferFn->equals(*TransferFn::MakeLinear());  // TODO: more efficient?
}

bool SkColorSpace_New::onIsNumericalTransferFn(SkColorSpaceTransferFn* fn) const {
    *fn = fTransferFn->parameterize();
    return true;
}
