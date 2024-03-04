/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkKnownRuntimeEffects.h"

#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace SkKnownRuntimeEffects {

namespace {

// This must be kept in sync w/ the version in BlurUtils.h
static constexpr int kMaxBlurSamples = 28;

SkRuntimeEffect* make_blur_1D_effect(int kernelWidth) {
    SkASSERT(kernelWidth <= kMaxBlurSamples);
    // The SkSL structure performs two kernel taps; if the kernel has an odd width the last
    // sample will be skipped with the current loop limit calculation.
    SkASSERT(kernelWidth % 2 == 0);
    return SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            SkStringPrintf(
                    // The coefficients are always stored for the max radius to keep the
                    // uniform block consistent across all effects.
                    "const int kMaxUniformKernelSize = %d / 2;"
                    // But we generate an exact loop over the kernel size. Note that this
                    // program can be used for kernels smaller than the constructed max as long
                    // as the kernel weights for excess entries are set to 0.
                    "const int kMaxLoopLimit = %d / 2;"

                    "uniform half4 offsetsAndKernel[kMaxUniformKernelSize];"
                    "uniform half2 dir;"

                    "uniform shader child;"

                    "half4 main(float2 coord) {"
                        "half4 sum = half4(0);"
                        "for (int i = 0; i < kMaxLoopLimit; ++i) {"
                            "half4 s = offsetsAndKernel[i];"
                            "sum += s.y * child.eval(coord + s.x*dir);"
                            "sum += s.w * child.eval(coord + s.z*dir);"
                        "}"
                        "return sum;"
                    "}", kMaxBlurSamples, kernelWidth).c_str());
}

SkRuntimeEffect* make_blur_2D_effect(int maxKernelSize) {
    SkASSERT(maxKernelSize % 4 == 0);
    return SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            SkStringPrintf(
                    // The coefficients are always stored for the max radius to keep the
                    // uniform block consistent across all effects.
                    "const int kMaxUniformKernelSize = %d / 4;"
                    "const int kMaxUniformOffsetsSize = 2*kMaxUniformKernelSize;"
                    // But we generate an exact loop over the kernel size. Note that this
                    // program can be used for kernels smaller than the constructed max as long
                    // as the kernel weights for excess entries are set to 0.
                    "const int kMaxLoopLimit = %d / 4;"

                    // Pack scalar coefficients into half4 for better packing on std140, and
                    // upload offsets to avoid having to transform the 1D index into a 2D coord
                    "uniform half4 kernel[kMaxUniformKernelSize];"
                    "uniform half4 offsets[kMaxUniformOffsetsSize];"

                    "uniform shader child;"

                    "half4 main(float2 coord) {"
                        "half4 sum = half4(0);"

                        "for (int i = 0; i < kMaxLoopLimit; ++i) {"
                            "half4 k = kernel[i];"
                            "half4 o = offsets[2*i];"
                            "sum += k.x * child.eval(coord + o.xy);"
                            "sum += k.y * child.eval(coord + o.zw);"
                            "o = offsets[2*i + 1];"
                            "sum += k.z * child.eval(coord + o.xy);"
                            "sum += k.w * child.eval(coord + o.zw);"
                        "}"
                        "return sum;"
                    "}", kMaxBlurSamples, maxKernelSize).c_str());
}

} // anonymous namespace

const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {
    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::SetStableKey(&options, static_cast<uint32_t>(stableKey));

    switch (stableKey) {
        case StableKey::kInvalid:
            return nullptr;

            // Shaders
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

        case StableKey::k1DBlur4: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(4);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur8: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(8);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur12: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(12);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur16: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(16);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur20: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(20);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur28: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(28);
            return s1DBlurEffect;
        }
        case StableKey::k2DBlur4: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(4);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur8: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(8);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur12: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(12);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur16: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(16);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur20: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(20);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur28: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(28);
            return s2DBlurEffect;
        }

        // Blenders
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

        // Color Filters
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
