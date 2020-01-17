/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkTableColorFilter.h"
#include "modules/skottie/src/Animator.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "src/utils/SkJSON.h"

#include <cmath>

namespace skottie {
namespace internal {

// Levels color correction effect.
//
// Maps the selected channels from [inBlack...inWhite] to [outBlack, outWhite],
// based on a gamma exponent.
//
// For [i0..i1] -> [o0..o1]:
//
//   c' = o0 + (o1 - o0) * ((c - i0) / (i1 - i0)) ^ G
//
// The output is optionally clipped to the output range.
//
// In/out intervals are clampped to [0..1].  Inversion is allowed.

namespace  {

class LevelsEffectAdapter final : public AnimatablePropertyContainer {
public:
    static sk_sp<LevelsEffectAdapter> Make(const skjson::ArrayValue& jprops,
                                           sk_sp<sksg::RenderNode> layer,
                                           const AnimationBuilder* abuilder) {
        return sk_sp<LevelsEffectAdapter>(new LevelsEffectAdapter(jprops,
                                                                  std::move(layer),
                                                                  abuilder));
    }

    sk_sp<sksg::RenderNode> node() const { return fEffect; }

private:
    LevelsEffectAdapter(const skjson::ArrayValue& jprops,
                        sk_sp<sksg::RenderNode> layer,
                        const AnimationBuilder* abuilder)
        : fEffect(sksg::ExternalColorFilter::Make(std::move(layer))) {
        enum : size_t {
            kChannel_Index        = 0,
            // ???                = 1,
            kInputBlack_Index     = 2,
            kInputWhite_Index     = 3,
            kGamma_Index          = 4,
            kOutputBlack_Index    = 5,
            kOutputWhite_Index    = 6,
            kClipToOutBlack_Index = 7,
            kClipToOutWhite_Index = 8,
        };

        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,     kChannel_Index), &fChannel );
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,  kInputBlack_Index), &fInBlack );
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,  kInputWhite_Index), &fInWhite );
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops,       kGamma_Index), &fGamma   );
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops, kOutputBlack_Index), &fOutBlack);
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops, kOutputWhite_Index), &fOutWhite);
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops, kClipToOutBlack_Index),
                   &fClipBlack);
        this->bind(*abuilder, EffectBuilder::GetPropValue(jprops, kClipToOutWhite_Index),
                   &fClipWhite);
    }

    void onSync() override {
        enum LottieChannel {
            kRGB_Channel = 1,
              kR_Channel = 2,
              kG_Channel = 3,
              kB_Channel = 4,
              kA_Channel = 5,
        };

        const auto channel = SkScalarTruncToInt(fChannel);
        if (channel < kRGB_Channel || channel > kA_Channel) {
            fEffect->setColorFilter(nullptr);
            return;
        }

        auto in_0 = SkTPin(fInBlack,  0.0f, 1.0f),
             in_1 = SkTPin(fInWhite,  0.0f, 1.0f),
            out_0 = SkTPin(fOutBlack, 0.0f, 1.0f),
            out_1 = SkTPin(fOutWhite, 0.0f, 1.0f),
                g = sk_ieee_float_divide(1, std::max(fGamma, 0.0f));

        float clip[] = {0, 1};
        const auto kLottieDoClip = 1;
        if (SkScalarTruncToInt(fClipBlack) == kLottieDoClip) {
            const auto idx = fOutBlack <= fOutWhite ? 0 : 1;
            clip[idx] = out_0;
        }
        if (SkScalarTruncToInt(fClipWhite) == kLottieDoClip) {
            const auto idx = fOutBlack <= fOutWhite ? 1 : 0;
            clip[idx] = out_1;
        }
        SkASSERT(clip[0] <= clip[1]);

        auto dIn  =  in_1 -  in_0,
             dOut = out_1 - out_0;

        if (SkScalarNearlyZero(dIn)) {
            // Degenerate dIn == 0 makes the arithmetic below explode.
            //
            // We could specialize the builder to deal with that case, or we could just
            // nudge by epsilon to make it all work.  The latter approach is simpler
            // and doesn't have any noticeable downsides.
            //
            // Also nudge in_0 towards 0.5, in case it was sqashed against an extremity.
            // This allows for some abrupt transition when the output interval is not
            // collapsed, and produces results closer to AE.
            static constexpr auto kEpsilon = 2 * SK_ScalarNearlyZero;
            dIn  += std::copysign(kEpsilon, dIn);
            in_0 += std::copysign(kEpsilon, .5f - in_0);
            SkASSERT(!SkScalarNearlyZero(dIn));
        }

        uint8_t lut[256];

        auto t =      -in_0 / dIn,
            dT = 1 / 255.0f / dIn;

        // TODO: is linear gamma common-enough to warrant a fast path?
        for (size_t i = 0; i < 256; ++i) {
            const auto out = out_0 + dOut * std::pow(std::max(t, 0.0f), g);
            SkASSERT(!SkScalarIsNaN(out));

            lut[i] = static_cast<uint8_t>(std::round(SkTPin(out, clip[0], clip[1]) * 255));

            t += dT;
        }

        fEffect->setColorFilter(SkTableColorFilter::MakeARGB(
            channel == kA_Channel                            ? lut : nullptr,
            channel == kR_Channel || channel == kRGB_Channel ? lut : nullptr,
            channel == kG_Channel || channel == kRGB_Channel ? lut : nullptr,
            channel == kB_Channel || channel == kRGB_Channel ? lut : nullptr
        ));
    }

    const sk_sp<sksg::ExternalColorFilter> fEffect;

    ScalarValue fChannel   = 1, // 1: RGB, 2: R, 3: G, 4: B, 5: A
                fInBlack   = 0,
                fInWhite   = 1,
                fOutBlack  = 0,
                fOutWhite  = 1,
                fGamma     = 1,
                fClipBlack = 1, // 1: clip, 2,3: don't clip
                fClipWhite = 1; // ^
};

} // anonymous ns

sk_sp<sksg::RenderNode> EffectBuilder::attachLevelsEffect(const skjson::ArrayValue& jprops,
                                                          sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<LevelsEffectAdapter>(jprops,
                                                                   std::move(layer),
                                                                   fBuilder);
}

} // namespace internal
} // namespace skottie
