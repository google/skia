/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/core/SkColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkTPin.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"

namespace skottie::internal {

namespace  {

// The contrast effect transfer function can be approximated with the following
// 3rd degree polynomial:
//
//   f(x) = -2πC/3 * x³ + πC * x² + (1 - πC/3) * x
//
// where C is the normalized contrast value [-1..1].
//
// Derivation:
//
//   - start off with sampling the AE contrast effect for various contrast/input values [1]
//
//   - apply cubic polynomial curve fitting to determine best-fit coefficients for given
//     contrast values [2]
//
//   - observations:
//       * negative contrast appears clamped at -0.5 (-50)
//       * a,b coefficients vary linearly vs. contrast
//       * the b coefficient for max contrast (1.0) looks kinda familiar: 3.14757 - coincidence?
//         probably not.  let's run with it: b == πC
//
//   - additionally, we expect the following to hold:
//       * f(0  ) = 0   \     | d = 0
//       * f(1  ) = 1    | => | a = -2b/3
//       * f(0.5) = 0.5 /     | c = 1 - b/3
//
//   - this yields a pretty decent approximation: [3]
//
//
// Note (courtesy of mtklein, reed): [4] seems to yield a closer approximation, but requires
// a more expensive sin
//
//   f(x) = x + a * sin(2πx)/2π
//
// [1] https://www.desmos.com/calculator/oksptqpo8z
// [2] https://www.desmos.com/calculator/oukrf6yahn
// [3] https://www.desmos.com/calculator/ehem0vy3ft
// [4] https://www.desmos.com/calculator/5t4xi10q4v
//

#ifndef SKOTTIE_ACCURATE_CONTRAST_APPROXIMATION
static sk_sp<SkData> make_contrast_coeffs(float contrast) {
    struct { float a, b, c; } coeffs;

    coeffs.b = SK_ScalarPI * contrast;
    coeffs.a = -2 * coeffs.b / 3;
    coeffs.c =  1 - coeffs.b / 3;

    return SkData::MakeWithCopy(&coeffs, sizeof(coeffs));
}

static constexpr char CONTRAST_EFFECT[] = R"(
    uniform half a;
    uniform half b;
    uniform half c;
    uniform shader input;

    half4 main() {
        // C' = a*C^3 + b*C^2 + c*C
        half4 color = sample(input);
        color.rgb = ((a*color.rgb + b)*color.rgb + c)*color.rgb;
        return color;
    }
)";
#else
// More accurate (but slower) approximation:
//
//   f(x) = x + a * sin(2πx)
//
//   a = -contrast/3π
//
static sk_sp<SkData> make_contrast_coeffs(float contrast) {
    const auto coeff_a = -contrast / (3 * SK_ScalarPI);

    return SkData::MakeWithCopy(&coeff_a, sizeof(coeff_a));
}

static constexpr char CONTRAST_EFFECT[] = R"(
    uniform half a;
    uniform shader input;

    half4 main() {
        half4 color = sample(input);
        color.rgb += a * sin(color.rgb * 6.283185);
        return color;
    }
)";

#endif

// Brightness transfer function approximation:
//
//   f(x) = 1 - (1 - x)^(2^(1.8*B))
//
// where B is the normalized [-1..1] brightness value
//
// Visualization: https://www.desmos.com/calculator/wuyqa2wtol
//
static sk_sp<SkData> make_brightness_coeffs(float brightness) {
    const float coeff_a = std::pow(2.0f, brightness * 1.8f);

    return SkData::MakeWithCopy(&coeff_a, sizeof(coeff_a));
}

static constexpr char BRIGHTNESS_EFFECT[] = R"(
    uniform half a;
    uniform shader input;

    half4 main() {
        half4 color = sample(input);
        color.rgb = 1 - pow(1 - color.rgb, half3(a));
        return color;
    }
)";

class BrightnessContrastAdapter final : public DiscardableAdapterBase<BrightnessContrastAdapter,
                                                                      sksg::ExternalColorFilter> {
public:
    BrightnessContrastAdapter(const skjson::ArrayValue& jprops,
                              const AnimationBuilder& abuilder,
                              sk_sp<sksg::RenderNode> layer)
        : INHERITED(sksg::ExternalColorFilter::Make(std::move(layer)))
        , fBrightnessEffect(SkRuntimeEffect::Make(SkString(BRIGHTNESS_EFFECT)).effect)
        , fContrastEffect(SkRuntimeEffect::Make(SkString(CONTRAST_EFFECT)).effect) {
        SkASSERT(fBrightnessEffect);
        SkASSERT(fContrastEffect);

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
        const auto brightness = SkTPin(fBrightness, -150.0f, 150.0f) / 150, // [-1.0 .. 1]
                     contrast = SkTPin(fContrast  ,  -50.0f, 100.0f) / 100; // [-0.5 .. 1]

        sk_sp<SkColorFilter> input = nullptr;

        auto b_eff = SkScalarNearlyZero(brightness)
                   ? nullptr
                   : fBrightnessEffect->makeColorFilter(make_brightness_coeffs(brightness),
                                                        &input, 1),
             c_eff = SkScalarNearlyZero(fContrast)
                   ? nullptr
                   : fContrastEffect->makeColorFilter(make_contrast_coeffs(contrast), &input, 1);

        return SkColorFilters::Compose(std::move(c_eff), std::move(b_eff));
    }

    const sk_sp<SkRuntimeEffect> fBrightnessEffect,
                                   fContrastEffect;

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
