/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/core/SkString.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkTPin.h"
#include "src/core/SkRuntimeEffectPriv.h"

sk_sp<SkColorFilter> SkHighContrastFilter::Make(const SkHighContrastConfig& config) {
    if (!config.isValid()) {
        return nullptr;
    }

    struct Uniforms { float grayscale, invertStyle, contrast; };

    SkString code{R"(
        uniform shader input;
        uniform half grayscale, invertStyle, contrast;
    )"};
    code += kRGB_to_HSL_sksl;
    code += kHSL_to_RGB_sksl;
    code += R"(
        half4 main() {
            half4 c = sample(input);  // linear unpremul RGBA in dst gamut.
            if (grayscale == 1) {
                c.rgb = dot(half3(0.2126, 0.7152, 0.0722), c.rgb).rrr;
            }
            if (invertStyle == 1/*brightness*/) {
                c.rgb = 1 - c.rgb;
            } else if (invertStyle == 2/*lightness*/) {
                c.rgb = rgb_to_hsl(c.rgb);
                c.b = 1 - c.b;
                c.rgb = hsl_to_rgb(c.rgb);
            }
            c.rgb = mix(half3(0.5), c.rgb, contrast);
            return half4(saturate(c.rgb), c.a);
        }
    )";

    sk_sp<SkRuntimeEffect> effect = SkMakeCachedRuntimeEffect(std::move(code));
    SkASSERT(effect);

    // A contrast setting of exactly +1 would divide by zero (1+c)/(1-c), so pull in to +1-ε.
    // I'm not exactly sure why we've historically pinned -1 up to -1+ε, maybe just symmetry?
    float c = SkTPin(config.fContrast,
                     -1.0f + FLT_EPSILON,
                     +1.0f - FLT_EPSILON);

    Uniforms uniforms = {
        config.fGrayscale ? 1.0f : 0.0f,
        (float)config.fInvertStyle,  // 0.0f for none, 1.0f for brightness, 2.0f for lightness
        (1+c)/(1-c),
    };

    sk_sp<SkColorFilter>    input = nullptr;
    skcms_TransferFunction linear = SkNamedTransferFn::kLinear;
    SkAlphaType          unpremul = kUnpremul_SkAlphaType;
    return SkColorFilters::WithWorkingFormat(
            effect->makeColorFilter(SkData::MakeWithCopy(&uniforms,sizeof(uniforms)), &input, 1),
            &linear, nullptr/*use dst gamut*/, &unpremul);
}

