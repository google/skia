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

        //This is straight out of GrHSLToRGBFilterEffect.fp.
        "half3 hsl_to_rgb(half3 hsl) {"
            "half  C = (1 - abs(2 * hsl.z - 1)) * hsl.y;"
            "half3 p = hsl.xxx + half3(0, 2/3.0, 1/3.0);"
            "half3 q = saturate(abs(fract(p) * 6 - 3) - 1);"
            "return (q - 0.5) * C + hsl.z;"
        "}"

        // This is mostly from skvm's rgb->hsl code, with some GPU-related finesse pulled from
        // GrHighContrastFilterEffect.fp, see next comment.
        "half3 rgb_to_hsl(half3 c) {"
            "half mx = max(max(c.r,c.g),c.b),"
            "     mn = min(min(c.r,c.g),c.b),"
            "      d = mx-mn,                "
            "   invd = 1.0 / d,              "
            " g_lt_b = c.g < c.b ? 6.0 : 0.0;"

            // We'd prefer to write these tests like `mx == c.r`, but on some GPUs max(x,y) is
            // not always equal to either x or y.  So we use long form, c.r >= c.g && c.r >= c.b.
            "half h = (1/6.0) * (mx == mn                 ? 0.0 :"
            "     /*mx==c.r*/    c.r >= c.g && c.r >= c.b ? invd * (c.g - c.b) + g_lt_b :"
            "     /*mx==c.g*/    c.g >= c.b               ? invd * (c.b - c.r) + 2.0  "
            "     /*mx==c.b*/                             : invd * (c.r - c.g) + 4.0);"

            "half sum = mx+mn,"
            "       l = sum * 0.5,"
            "       s = mx == mn ? 0.0"
            "                    : d / (l > 0.5 ? 2.0 - sum : sum);"
            "return half3(h,s,l);"
        "}"
    };

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

