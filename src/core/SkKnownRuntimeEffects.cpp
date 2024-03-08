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

SkRuntimeEffect* make_blur_1D_effect(int kernelWidth, const SkRuntimeEffect::Options& options) {
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
                    "}", kMaxBlurSamples, kernelWidth).c_str(),
                    options);
}

SkRuntimeEffect* make_blur_2D_effect(int maxKernelSize, const SkRuntimeEffect::Options& options) {
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
                    "}", kMaxBlurSamples, maxKernelSize).c_str(),
                    options);
}

} // anonymous namespace

const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey stableKey) {
    SkRuntimeEffect::Options options;
    SkRuntimeEffectPriv::SetStableKey(&options, static_cast<uint32_t>(stableKey));

    switch (stableKey) {
        case StableKey::kInvalid:
            return nullptr;

        // Shaders
        case StableKey::k1DBlur4: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(4, options);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur8: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(8, options);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur12: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(12, options);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur16: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(16, options);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur20: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(20, options);
            return s1DBlurEffect;
        }
        case StableKey::k1DBlur28: {
            static SkRuntimeEffect* s1DBlurEffect = make_blur_1D_effect(28, options);
            return s1DBlurEffect;
        }
        case StableKey::k2DBlur4: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(4, options);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur8: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(8, options);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur12: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(12, options);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur16: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(16, options);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur20: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(20, options);
            return s2DBlurEffect;
        }
        case StableKey::k2DBlur28: {
            static SkRuntimeEffect* s2DBlurEffect = make_blur_2D_effect(28, options);
            return s2DBlurEffect;
        }
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
        case StableKey::kDecal: {
            static constexpr char kDecalShaderCode[] =
                "uniform shader image;"
                "uniform float4 decalBounds;"

                "half4 main(float2 coord) {"
                    "half4 d = half4(decalBounds - coord.xyxy) * half4(-1, -1, 1, 1);"
                    "d = saturate(d + 0.5);"
                    "return (d.x*d.y*d.z*d.w) * image.eval(coord);"
                "}";

            static const SkRuntimeEffect* sDecalEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        kDecalShaderCode,
                                        options);
            return sDecalEffect;
        }
        case StableKey::kDisplacement: {
            // NOTE: This uses dot product selection to work on all GLES2 hardware (enforced by
            // public runtime effect restrictions). Otherwise, this would use a "uniform ivec2"
            // and component indexing to convert the displacement color into a vector.
            static constexpr char kDisplacementShaderCode[] =
                "uniform shader displMap;"
                "uniform shader colorMap;"
                "uniform half2 scale;"
                "uniform half4 xSelect;" // Only one of RGBA will be 1, the rest are 0
                "uniform half4 ySelect;"

                "half4 main(float2 coord) {"
                    "half4 displColor = unpremul(displMap.eval(coord));"
                    "half2 displ = half2(dot(displColor, xSelect), dot(displColor, ySelect));"
                    "displ = scale * (displ - 0.5);"
                    "return colorMap.eval(coord + displ);"
                "}";

            static const SkRuntimeEffect* sDisplacementEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        kDisplacementShaderCode,
                                        options);
            return sDisplacementEffect;
        }
        case StableKey::kLighting: {
            static constexpr char kLightingShaderCode[] =
                "const half kConeAAThreshold = 0.016;"
                "const half kConeScale = 1.0 / kConeAAThreshold;"

                "uniform shader normalMap;"

                // Packs surface depth, shininess, material type (0 == diffuse) and light type
                // (< 0 = distant, 0 = point, > 0 = spot)
                "uniform half4 materialAndLightType;"

                "uniform half4 lightPosAndSpotFalloff;" // (x,y,z) are lightPos, w is spot falloff
                                                        // exponent
                "uniform half4 lightDirAndSpotCutoff;" // (x,y,z) are lightDir,
                                                       // w is spot cos(cutoffAngle)
                "uniform half3 lightColor;" // Material's k has already been multiplied in

                "half3 surface_to_light(half3 coord) {"
                    "if (materialAndLightType.w < 0) {"
                        "return lightDirAndSpotCutoff.xyz;"
                    "} else {"
                        // Spot and point have the same equation
                        "return normalize(lightPosAndSpotFalloff.xyz - coord);"
                    "}"
                "}"

                "half spotlight_scale(half3 surfaceToLight) {"
                    "half cosCutoffAngle = lightDirAndSpotCutoff.w;"
                    "half cosAngle = -dot(surfaceToLight, lightDirAndSpotCutoff.xyz);"
                    "if (cosAngle < cosCutoffAngle) {"
                        "return 0.0;"
                    "}"
                    "half scale = pow(cosAngle, lightPosAndSpotFalloff.w);"
                    "if (cosAngle < cosCutoffAngle + kConeAAThreshold) {"
                        "return scale * (cosAngle - cosCutoffAngle) * kConeScale;"
                    "} else {"
                        "return scale;"
                    "}"
                "}"

                "half4 compute_lighting(half3 normal, half3 surfaceToLight) {"
                    // Point and distant light color contributions are constant
                    "half3 color = lightColor;"
                    // Spotlights fade based on the angle away from its direction
                    "if (materialAndLightType.w > 0) {"
                        "color *= spotlight_scale(surfaceToLight);"
                    "}"

                    // Diffuse and specular reflections scale the light's "color" differently
                    "if (materialAndLightType.z == 0) {"
                        "half coeff = dot(normal, surfaceToLight);"
                        "color = saturate(coeff * color);"
                        "return half4(color, 1.0);"
                    "} else {"
                        "half3 halfDir = normalize(surfaceToLight + half3(0, 0, 1));"
                        "half shininess = materialAndLightType.y;"
                        "half coeff = pow(dot(normal, halfDir), shininess);"
                        "color = saturate(coeff * color);"
                        "return half4(color, max(max(color.r, color.g), color.b));"
                    "}"
                "}"

                "half4 main(float2 coord) {"
                    "half4 normalAndA = normalMap.eval(coord);"
                    "half depth = materialAndLightType.x;"
                    "half3 surfaceToLight = surface_to_light(half3(half2(coord),"
                                                                  "depth*normalAndA.a));"
                    "return compute_lighting(normalAndA.xyz, surfaceToLight);"
                "}";

            static const SkRuntimeEffect* sLightingEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        kLightingShaderCode,
                                        options);
            return sLightingEffect;
        }
        case StableKey::kLinearMorphology: {
            static constexpr char kLinearMorphologyShaderCode[] =
                // KEEP IN SYNC WITH SkMorphologyImageFilter.cpp DEFINITION
                "const int kMaxLinearRadius = 14;"

                "uniform shader child;"
                "uniform half2 offset;"
                "uniform half flip;" // -1 converts the max() calls to min()
                "uniform int radius;"

                "half4 main(float2 coord) {"
                    "half4 aggregate = flip*child.eval(coord);" // case 0 only samples once
                    "for (int i = 1; i <= kMaxLinearRadius; ++i) {"
                        "if (i > radius) break;"
                        "half2 delta = half(i) * offset;"
                        "aggregate = max(aggregate, max(flip*child.eval(coord + delta),"
                                                       "flip*child.eval(coord - delta)));"
                    "}"
                    "return flip*aggregate;"
                "}";

            static const SkRuntimeEffect* sLinearMorphologyEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        kLinearMorphologyShaderCode,
                                        options);
            return sLinearMorphologyEffect;
        }

        case StableKey::kMagnifier: {
            static constexpr char kMagnifierShaderCode[] =
                "uniform shader src;"
                "uniform float4 lensBounds;"
                "uniform float4 zoomXform;"
                "uniform float2 invInset;"

                "half4 main(float2 coord) {"
                    "float2 zoomCoord = zoomXform.xy + zoomXform.zw*coord;"
                    // edgeInset is the smallest distance to the lens bounds edges,
                    // in units of "insets".
                    "float2 edgeInset = min(coord - lensBounds.xy, lensBounds.zw - coord) *"
                                       "invInset;"

                    // The equations for 'weight' ensure that it is 0 along the outside of
                    // lensBounds so it seams with any un-zoomed, un-filtered content. The zoomed
                    // content fills a rounded rectangle that is 1 "inset" in from lensBounds with
                    // circular corners with radii equal to the inset distance. Outside of this
                    // region, there is a non-linear weighting to compress the un-zoomed content
                    // to the zoomed content. The critical zone about each corner is limited
                    // to 2x"inset" square.
                    "float weight = (edgeInset.x < 2.0 && edgeInset.y < 2.0)"
                        // Circular distortion weighted by distance to inset corner
                        "? (2.0 - length(2.0 - edgeInset))"
                        // Linear zoom, or single-axis compression outside of the inset
                        // area (if delta < 1)
                        ": min(edgeInset.x, edgeInset.y);"

                    // Saturate before squaring so that negative weights are clamped to 0
                    // before squaring
                    "weight = saturate(weight);"
                    "return src.eval(mix(coord, zoomCoord, weight*weight));"
                "}";

            static const SkRuntimeEffect* sMagnifierEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        kMagnifierShaderCode,
                                        options);
            return sMagnifierEffect;
        }
        case StableKey::kNormal: {
            static constexpr char kNormalShaderCode[] =
                "uniform shader alphaMap;"
                "uniform float4 edgeBounds;"
                "uniform half negSurfaceDepth;"

                "half3 normal(half3 alphaC0, half3 alphaC1, half3 alphaC2) {"
                    // The right column (or bottom row) terms of the Sobel filter. The left/top is
                    // just the negative, and the middle row/column is all 0s so those instructions
                    // are skipped.
                    "const half3 kSobel = 0.25 * half3(1,2,1);"
                    "half3 alphaR0 = half3(alphaC0.x, alphaC1.x, alphaC2.x);"
                    "half3 alphaR2 = half3(alphaC0.z, alphaC1.z, alphaC2.z);"
                    "half nx = dot(kSobel, alphaC2) - dot(kSobel, alphaC0);"
                    "half ny = dot(kSobel, alphaR2) - dot(kSobel, alphaR0);"
                    "return normalize(half3(negSurfaceDepth * half2(nx, ny), 1));"
                "}"

                "half4 main(float2 coord) {"
                   "half3 alphaC0 = half3("
                     "alphaMap.eval(clamp(coord + float2(-1,-1), edgeBounds.LT, edgeBounds.RB)).a,"
                     "alphaMap.eval(clamp(coord + float2(-1, 0), edgeBounds.LT, edgeBounds.RB)).a,"
                     "alphaMap.eval(clamp(coord + float2(-1, 1), edgeBounds.LT, edgeBounds.RB)).a);"
                   "half3 alphaC1 = half3("
                     "alphaMap.eval(clamp(coord + float2( 0,-1), edgeBounds.LT, edgeBounds.RB)).a,"
                     "alphaMap.eval(clamp(coord + float2( 0, 0), edgeBounds.LT, edgeBounds.RB)).a,"
                     "alphaMap.eval(clamp(coord + float2( 0, 1), edgeBounds.LT, edgeBounds.RB)).a);"
                   "half3 alphaC2 = half3("
                     "alphaMap.eval(clamp(coord + float2( 1,-1), edgeBounds.LT, edgeBounds.RB)).a,"
                     "alphaMap.eval(clamp(coord + float2( 1, 0), edgeBounds.LT, edgeBounds.RB)).a,"
                     "alphaMap.eval(clamp(coord + float2( 1, 1), edgeBounds.LT, edgeBounds.RB)).a);"

                   "half mainAlpha = alphaC1.y;" // offset = (0,0)
                   "return half4(normal(alphaC0, alphaC1, alphaC2), mainAlpha);"
                "}";

            static const SkRuntimeEffect* sNormalEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        kNormalShaderCode,
                                        options);
            return sNormalEffect;
        }
        case StableKey::kSparseMorphology: {
            static constexpr char kSparseMorphologyShaderCode[] =
                "uniform shader child;"
                "uniform half2 offset;"
                "uniform half flip;"

                "half4 main(float2 coord) {"
                    "half4 aggregate = max(flip*child.eval(coord + offset),"
                                          "flip*child.eval(coord - offset));"
                    "return flip*aggregate;"
                "}";

            static const SkRuntimeEffect* sSparseMorphologyEffect =
                    SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                        kSparseMorphologyShaderCode,
                                        options);
            return sSparseMorphologyEffect;
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
