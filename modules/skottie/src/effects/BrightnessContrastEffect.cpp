/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkColorFilter.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"

namespace skottie::internal {

namespace  {

class BrightnessContrastAdapter final : public DiscardableAdapterBase<BrightnessContrastAdapter,
                                                                      sksg::ExternalColorFilter> {
public:
    BrightnessContrastAdapter(const skjson::ArrayValue& jprops,
                              const AnimationBuilder& abuilder,
                              sk_sp<sksg::RenderNode> layer)
        : INHERITED(sksg::ExternalColorFilter::Make(std::move(layer))) {
        enum : size_t {
            kBrightness_Index = 0,
              kContrast_Index = 1,
             kUseLegacy_Index = 2,
        };

        EffectBinder(jprops, abuilder, this)
            .bind(kBrightness_Index, fBrightness)
            .bind(  kContrast_Index, fContrast  )
            .bind( kUseLegacy_Index, fUseLegacy );
    }

private:
    void onSync() override {
        this->node()->setColorFilter(SkScalarRoundToInt(fUseLegacy)
                                        ? this->makeLegacyCF()
                                        : this->makeCF());
    }

    sk_sp<SkColorFilter> makeLegacyCF() const {
        // In 'legacy' mode, brightness is
        //
        //   - in the [-100..100] range
        //   - applied component-wise as a direct offset (255-based)
        //   - (neutral value: 0)
        //   - transfer function: https://www.desmos.com/calculator/zne0oqwwzb
        //
        // while contrast is
        //
        //   - in the [-100..100] range
        //   - applied as a component-wise linear transformation (scale+offset), such that
        //
        //       -100 always yields mid-gray: contrast(x, -100) == 0.5
        //          0 is the neutral value:   contrast(x,    0) == x
        //        100 always yields white:    contrast(x,  100) == 1
        //
        //   - transfer function: https://www.desmos.com/calculator/x5rxzhowhs
        //

        // Normalize to [-1..1]
        const auto brightness = SkTPin(fBrightness, -100.0f, 100.0f) / 255, // [-100/255 .. 100/255]
                   contrast   = SkTPin(fContrast  , -100.0f, 100.0f) / 100; // [      -1 ..       1]

        // The component scale is derived from contrast:
        //
        //   Contrast[-1 .. 0] -> Scale[0 .. 1]
        //   Contrast( 0 .. 1] -> Scale(1 .. +inf)
        const auto S = contrast > 0
            ? 1 / std::max(1 - contrast, SK_ScalarNearlyZero)
            : 1 + contrast;

        // The component offset is derived from both brightness and contrast:
        //
        //   Brightness[-100/255 .. 100/255] -> Offset[-100/255 .. 100/255]
        //   Contrast  [      -1 ..       0] -> Offset[     0.5 ..       0]
        //   Contrast  (       0 ..       1] -> Offset(       0 ..    -inf)
        //
        // Why do these pre/post compose depending on contrast scale, you ask?
        // Because AE - that's why!
        const auto B = 0.5f * (1 - S) + brightness * std::max(S, 1.0f);

        const float cm[] = {
            S, 0, 0, 0, B,
            0, S, 0, 0, B,
            0, 0, S, 0, B,
            0, 0, 0, 1, 0,
        };

        return SkColorFilters::Matrix(cm);
    }

    sk_sp<SkColorFilter> makeCF() const {
        // TODO: lots of non-linear curve fitting fun
        return nullptr;
    }

    ScalarValue fBrightness = 0,
                fContrast   = 0,
                fUseLegacy  = 0;

    using INHERITED = DiscardableAdapterBase<BrightnessContrastAdapter, sksg::ExternalColorFilter>;
};

} // namespace

sk_sp<sksg::RenderNode> EffectBuilder::attachBrightnessContrastEffect(
        const skjson::ArrayValue& jprops, sk_sp<sksg::RenderNode> layer) const {
    return fBuilder->attachDiscardableAdapter<BrightnessContrastAdapter>(jprops,
                                                                         *fBuilder,
                                                                         std::move(layer));
}

} // namespace skottie::internal
