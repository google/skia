/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/effects/SkHighContrastFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkTPin.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkRuntimeEffectPriv.h"

#include <cfloat>

sk_sp<SkColorFilter> SkHighContrastFilter::Make(const SkHighContrastConfig& config) {
    if (!config.isValid()) {
        return nullptr;
    }

    struct Uniforms { float grayscale, invertStyle, contrast; };

    static constexpr char kHighContrastFilterCode[] =
            "uniform half grayscale, invertStyle, contrast;"

            // TODO(skia:13540): Investigate using $rgb_to_hsl from sksl_shared instead.
            "half3 rgb_to_hsl(half3 c) {"
                "half mx = max(max(c.r,c.g),c.b),"
                     "mn = min(min(c.r,c.g),c.b),"
                      "d = mx-mn,"
                   "invd = 1.0 / d,"
                 "g_lt_b = c.g < c.b ? 6.0 : 0.0;"

            // We'd prefer to write these tests like `mx == c.r`, but on some GPUs max(x,y) is
            // not always equal to either x or y.  So we use long form, c.r >= c.g && c.r >= c.b.
                "half h = (1/6.0) * (mx == mn                 ? 0.0 :"
                     /*mx==c.r*/    "c.r >= c.g && c.r >= c.b ? invd * (c.g - c.b) + g_lt_b :"
                     /*mx==c.g*/    "c.g >= c.b               ? invd * (c.b - c.r) + 2.0"
                     /*mx==c.b*/                             ": invd * (c.r - c.g) + 4.0);"
                "half sum = mx+mn,"
                       "l = sum * 0.5,"
                       "s = mx == mn ? 0.0"
                                    ": d / (l > 0.5 ? 2.0 - sum : sum);"
                "return half3(h,s,l);"
            "}"
            "half4 main(half4 inColor) {"
                "half3 c = inColor.rgb;"
                "if (grayscale == 1) {"
                    "c = dot(half3(0.2126, 0.7152, 0.0722), c).rrr;"
                "}"
                "if (invertStyle == 1) {"  // brightness
                    "c = 1 - c;"
                "} else if (invertStyle == 2) {"  // lightness
                    "c = rgb_to_hsl(c);"
                    "c.b = 1 - c.b;"
                    "c = $hsl_to_rgb(c);"
                "}"
                "c = mix(half3(0.5), c, contrast);"
                "return half4(saturate(c), inColor.a);"
            "}";

    static const SkRuntimeEffect* effect = SkMakeCachedRuntimeEffect(
        SkRuntimeEffect::MakeForColorFilter,
        SkString(kHighContrastFilterCode)
    ).release();

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

    skcms_TransferFunction linear = SkNamedTransferFn::kLinear;
    SkAlphaType          unpremul = kUnpremul_SkAlphaType;
    return SkColorFilterPriv::WithWorkingFormat(
            effect->makeColorFilter(SkData::MakeWithCopy(&uniforms,sizeof(uniforms))),
            &linear, nullptr/*use dst gamut*/, &unpremul);
}
