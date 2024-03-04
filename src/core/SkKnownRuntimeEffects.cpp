/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKnownRuntimeEffects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace SkKnownRuntimeEffects {

const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {

    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::SetStableKey(&options, static_cast<uint32_t>(stableKey));

    switch (stableKey) {
        case StableKey::kInvalid:
            return nullptr;

        case StableKey::kBlend: {
            static constexpr char kBlendShaderCode[] =
                "uniform shader s, d;"
                "uniform blender b;"
                "half4 main(float2 xy) {"
                    "return b.eval(s.eval(xy), d.eval(xy));"
                "}";

            static const SkRuntimeEffect* sBlendEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        kBlendShaderCode,
                                        options);
            return sBlendEffect;
        }

        case StableKey::kArithmetic: {
            static constexpr char kArithmeticBlenderCode[] =
                "uniform half4 k;"
                "uniform half pmClamp;"

                "half4 main(half4 src, half4 dst) {"
                    "half4 c = saturate(k.x * src * dst + k.y * src + k.z * dst + k.w);"
                    "c.rgb = min(c.rgb, max(c.a, pmClamp));"
                    "return c;"
                "}";

            static const SkRuntimeEffect* sArithmeticEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForBlender,
                                        kArithmeticBlenderCode,
                                        options);
            return sArithmeticEffect;
        }

        case StableKey::kHighContrast: {
            static constexpr char kHighContrastFilterCode[] =
                "uniform half grayscale, invertStyle, contrast;"

                "half3 rgb_to_hsl(half3 c) {"
                    "half mx = max(max(c.r,c.g),c.b),"
                         "mn = min(min(c.r,c.g),c.b),"
                          "d = mx-mn,"
                       "invd = 1.0 / d,"
                     "g_lt_b = c.g < c.b ? 6.0 : 0.0;"

                // We'd prefer to write these tests like `mx == c.r`, but on some GPUs max(x,y) is
                // not always equal to either x or y. So we use long form, c.r >= c.g && c.r >= c.b.
                    "half h = (1/6.0) * (mx == mn"               "? 0.0 :"
                         /*mx==c.r*/    "c.r >= c.g && c.r >= c.b ? invd * (c.g - c.b) + g_lt_b :"
                         /*mx==c.g*/    "c.g >= c.b"             "? invd * (c.b - c.r) + 2.0"
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

            static const SkRuntimeEffect* sHighContrastEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                        kHighContrastFilterCode,
                                        options);
            return sHighContrastEffect;
        }

        case StableKey::kLerp: {
            static constexpr char kLerpFilterCode[] =
                "uniform colorFilter cf0;"
                "uniform colorFilter cf1;"
                "uniform half weight;"

                "half4 main(half4 color) {"
                    "return mix(cf0.eval(color), cf1.eval(color), weight);"
                "}";

            static const SkRuntimeEffect* sLerpEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                        kLerpFilterCode,
                                        options);
            return sLerpEffect;
        }

        case StableKey::kLuma: {
            static constexpr char kLumaFilterCode[] =
                "half4 main(half4 inColor) {"
                    "return saturate(dot(half3(0.2126, 0.7152, 0.0722), inColor.rgb)).000r;"
                "}";

            static const SkRuntimeEffect* sLumaEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                        kLumaFilterCode,
                                        options);
            return sLumaEffect;
        }

        case StableKey::kOverdraw: {
            static constexpr char kOverdrawFilterCode[] =
                "uniform half4 color0, color1, color2, color3, color4, color5;"

                "half4 main(half4 color) {"
                    "half alpha = 255.0 * color.a;"
                    "return alpha < 0.5 ? color0"
                         ": alpha < 1.5 ? color1"
                         ": alpha < 2.5 ? color2"
                         ": alpha < 3.5 ? color3"
                         ": alpha < 4.5 ? color4 : color5;"
                "}";

            static const SkRuntimeEffect* sOverdrawEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                        kOverdrawFilterCode,
                                        options);
            return sOverdrawEffect;
        }
    }

    SkUNREACHABLE;
}

} // namespace SkKnownRuntimeEffects
