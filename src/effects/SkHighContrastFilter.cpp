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

sk_sp<SkColorFilter> SkHighContrastFilter::Make(const SkHighContrastConfig& userConfig) {
    if (!userConfig.isValid()) {
        return nullptr;
    }

    // A contrast setting of exactly +1 would divide by zero (1+c)/(1-c), so pull in to +1-ε.
    // I'm not exactly sure why we've historically pinned -1 up to -1+ε, maybe just symmetry?
    SkHighContrastConfig config = userConfig;
    config.fContrast = SkTPin(config.fContrast,
                              -1.0f + FLT_EPSILON,
                              +1.0f - FLT_EPSILON);

    struct { float M; } uniforms;
    SkString code{
        "uniform shader input;"
        "uniform half M;"
    };

    code += kRGB_to_HSL_sksl;
    code += kHSL_to_RGB_sksl;

    code += "half4 main() {";
    if (true) {
        code += "half4 c = sample(input);"; // c is linear unpremul RGBA in the dst gamut.
    }
    if (config.fGrayscale) {
        code += "c.rgb = dot(half3(0.2126, 0.7152, 0.0722), c.rgb).rrr;";
    }
    if (config.fInvertStyle == SkHighContrastConfig::InvertStyle::kInvertBrightness) {
        code += "c.rgb = 1 - c.rgb;";
    }
    if (config.fInvertStyle == SkHighContrastConfig::InvertStyle::kInvertLightness) {
        code += "c.rgb = rgb_to_hsl(c.rgb);";
        code += "c.b = 1 - c.b;";
        code += "c.rgb = hsl_to_rgb(c.rgb);";
    }
    if (float c = config.fContrast) {
        uniforms.M = (1+c)/(1-c);
        code += "c.rgb = (c.rgb - 0.5)*M + 0.5;";
    }
    if (true) {
        code += "return saturate(c);";
    }
    code += "}";

    auto [effect, err] = SkRuntimeEffect::Make(code);
    if (!err.isEmpty()) {
        SkDebugf("%s\n%s\n", code.c_str(), err.c_str());
    }
    SkASSERT(effect && err.isEmpty());

    sk_sp<SkColorFilter>    input = nullptr;
    skcms_TransferFunction linear = SkNamedTransferFn::kLinear;
    SkAlphaType          unpremul = kUnpremul_SkAlphaType;
    return SkColorFilters::WithWorkingFormat(
            effect->makeColorFilter(SkData::MakeWithCopy(&uniforms,sizeof(uniforms)), &input, 1),
            &linear, nullptr/*use dst gamut*/, &unpremul);
}

