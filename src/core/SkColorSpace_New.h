/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_New_DEFINED
#define SkColorSpace_New_DEFINED

#include "SkColorSpace.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"

class SkRasterPipeline;

class SkColorSpace_New final : public SkColorSpace {
public:
    class ICCProfile;   // TODO: == SkICC?

    struct TransferFn : public SkRefCnt {
        virtual ~TransferFn() = default;

        // TODO: one day maybe we'd like to not need this call,
        // instead using the more active methods below instead.
        virtual SkColorSpaceTransferFn parameterize() const = 0;

        // Append stages to use this transfer function with SkRasterPipeline-based rendering.
        virtual void linearizeDst(SkRasterPipeline*) const = 0;
        virtual void linearizeSrc(SkRasterPipeline*) const = 0;
        virtual void    encodeSrc(SkRasterPipeline*) const = 0;

        // TODO: Ganesh hooks.

        // May return false even when this is equivalent to TransferFn,
        // but must always be equivalent when this returns true.  (No false positives.)
        // Implemented by default with parameterize().
        virtual bool equals(const TransferFn&) const;

        // TODO: ???
        // Implemented by default with parameterize().
        virtual void updateICCProfile(ICCProfile*) const;

        static sk_sp<TransferFn> MakeLinear();
        static sk_sp<TransferFn> MakeSRGB();
        static sk_sp<TransferFn> MakeGamma(float);
    };

    enum class Blending { Linear, AsEncoded };

    SkColorSpace_New(sk_sp<TransferFn>, SkMatrix44 toXYZD50, Blending);

    const SkMatrix44&   toXYZD50() const { return fToXYZD50;    }
    const SkMatrix44& fromXYZD50() const { return fFromXYZD50;  }
    const TransferFn& transferFn() const { return *fTransferFn; }
    Blending            blending() const { return fBlending;    }

    // Transfer-function-related overrides.
    sk_sp<SkColorSpace> makeLinearGamma() const override;
    sk_sp<SkColorSpace>   makeSRGBGamma() const override;
    SkGammaNamed           onGammaNamed() const override;
    bool             onGammaCloseToSRGB() const override;
    bool                onGammaIsLinear() const override;
    bool onIsNumericalTransferFn(SkColorSpaceTransferFn*) const override;

    // Gamut-related overrides.
    const SkMatrix44* onFromXYZD50() const override { return &fFromXYZD50; }
    const SkMatrix44*   onToXYZD50() const override { return   &fToXYZD50; }
    uint32_t        onToXYZD50Hash() const override { return fToXYZD50Hash; }

private:
    sk_sp<TransferFn> fTransferFn;
    SkMatrix44        fFromXYZD50;
    SkMatrix44        fToXYZD50;
    uint32_t          fToXYZD50Hash;
    Blending          fBlending;
};
#endif//SkColorSpace_New_DEFINED
