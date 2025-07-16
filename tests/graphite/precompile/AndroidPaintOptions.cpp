/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GRAPHITE)
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/Precompile.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

#include "tests/graphite/precompile/PaintOptionsBuilder.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"

#if defined (SK_VULKAN)
#include "include/gpu/graphite/vk/precompile/VulkanPrecompileShader.h"
#include "include/gpu/vk/VulkanTypes.h"
#endif // SK_VULKAN

using namespace skgpu::graphite;
using PrecompileShaders::ImageShaderFlags;

using namespace PaintOptionsUtils;
using namespace PrecompileTestUtils;

// Used in lieu of SkEnumBitMask to avoid adding casts when copying in precompile cases.
static constexpr DrawTypeFlags operator|(DrawTypeFlags a, DrawTypeFlags b) {
    return static_cast<DrawTypeFlags>(static_cast<std::underlying_type<DrawTypeFlags>::type>(a) |
                                      static_cast<std::underlying_type<DrawTypeFlags>::type>(b));
}

#if defined(SK_VULKAN)
namespace {

sk_sp<PrecompileShader> vulkan_ycbcr_image_shader(uint64_t format,
                                                  VkSamplerYcbcrModelConversion model,
                                                  VkSamplerYcbcrRange range,
                                                  VkChromaLocation location,
                                                  bool pqCS = false) {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     pqCS ? SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ,
                                                  SkNamedGamut::kRec2020)
                          : nullptr };

    skgpu::VulkanYcbcrConversionInfo info;

    info.fExternalFormat = format;
    info.fYcbcrModel     = model;
    info.fYcbcrRange     = range;
    info.fXChromaOffset  = location;
    info.fYChromaOffset  = location;
    info.fChromaFilter   = VK_FILTER_LINEAR;

    return PrecompileShaders::VulkanYCbCrImage(info,
                                               PrecompileShaders::ImageShaderFlags::kExcludeCubic,
                                               { &ci, 1 },
                                               {});
}

} // anonymous namespace
#endif // SK_VULKAN

// Specifies the child shader to be created for a RE_LinearEffect
enum class ChildType {
    kSolidColor,
    kHWTexture,
#if defined(SK_VULKAN)
    kHWTextureYCbCr247,
#endif
};

namespace {

// Note: passing in a name to 'makeEffect' is a difference from Android's factory functions.
sk_sp<SkRuntimeEffect> makeEffect(const SkString& sksl, const char* name) {
    SkRuntimeEffect::Options options;
    options.fName = name;

    auto [effect, error] = SkRuntimeEffect::MakeForShader(sksl, options);
    if (!effect) {
        SkDebugf("%s\n", error.c_str());
    }
    return effect;
}

// This assumes there is some Singleton in Android that can provide RE_LinearEffects
// given some input. For this mock up, the input is just the parameter portion
// of the RE_LinearEffect Pipeline label (e.g., "UNKNOWN__SRGB__false__UNKNOWN").
// Presumably, irl, the parameters would be the actual types used to create the label.
class LinearEffectSingleton {
public:
    sk_sp<SkRuntimeEffect> findOrCreate(const char* parameterStr) {
        SkString name = SkStringPrintf("RE_LinearEffect_%s__Shader",
                                       parameterStr);

        auto result = fEffects.find(name.c_str());
        if (result != fEffects.end()) {
            return result->second;
        }

        // Each code snippet must be unique, otherwise Skia will internally find a match
        // and uniquify things. To avoid this we just add an arbitrary alpha constant
        // to the code.
        static float arbitraryAlpha = 0.051f;
        SkString linearEffectCode = SkStringPrintf(
            "uniform shader child;"
            "vec4 main(vec2 xy) {"
                "float3 linear = toLinearSrgb(child.eval(xy).rgb);"
                "return float4(fromLinearSrgb(linear), %f);"
            "}",
            arbitraryAlpha);
        arbitraryAlpha += 0.05f;

        sk_sp<SkRuntimeEffect> linearEffect = makeEffect(linearEffectCode, name.c_str());

        fEffects.insert({ name.c_str(), linearEffect });
        return linearEffect;
    }

private:
    std::map<std::string, sk_sp<SkRuntimeEffect>> fEffects;
};

sk_sp<PrecompileShader> create_child_shader(ChildType childType) {
    switch (childType) {
        case ChildType::kSolidColor:
            return PrecompileShaders::Color();
        case ChildType::kHWTexture: {
            SkColorInfo ci { kRGBA_8888_SkColorType,
                             kPremul_SkAlphaType,
                             SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                   SkNamedGamut::kAdobeRGB) };

            return PrecompileShaders::Image(PrecompileShaders::ImageShaderFlags::kExcludeCubic,
                                            { &ci, 1 },
                                            {});
        }
#if defined(SK_VULKAN)
        case ChildType::kHWTextureYCbCr247:
            // HardwareImage(3: kEwAAPcAAAAAAAAA)
            return vulkan_ycbcr_image_shader(247,
                                             VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
                                             VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                             VK_CHROMA_LOCATION_COSITED_EVEN,
                                             /* pqCS= */ true);
#endif
    }

    return nullptr;
}

} // anonymous namespace

skgpu::graphite::PaintOptions LinearEffect(const char* parameterStr,
                                           ChildType childType,
                                           SkBlendMode blendMode,
                                           bool paintColorIsOpaque = true,
                                           bool matrixColorFilter = false,
                                           bool dither = false) {
    static LinearEffectSingleton gLinearEffectSingleton;

    PaintOptions paintOptions;

    sk_sp<SkRuntimeEffect> linearEffect = gLinearEffectSingleton.findOrCreate(parameterStr);
    sk_sp<PrecompileShader> child = create_child_shader(childType);

    paintOptions.setShaders({ PrecompileRuntimeEffects::MakePrecompileShader(
                                            std::move(linearEffect),
                                            { { std::move(child) } }) });
    if (matrixColorFilter) {
        paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
    }
    paintOptions.setBlendModes({ blendMode });
    paintOptions.setPaintColorIsOpaque(paintColorIsOpaque);
    paintOptions.setDither(dither);

    return paintOptions;
}

// =======================================
//         PaintOptions
// =======================================
// NOTE: keep in sync with upstream external/skia/tests/graphite/precompile/PrecompileTestUtils.cpp
// clang-format off

namespace {

class MouriMap {
public:
    MouriMap() {
        // The following code blocks are just stubs for the Android code. For Skia's testing
        // purposes they only need to have the same name and number of children as the real code.
        // When the following PaintOptions are used in Android the real SkSL must be supplied.
        static const SkString kCrosstalkAndChunk16x16Code(R"(
            uniform shader img;
            vec4 main(vec2 xy) {
                float3 linear = img.eval(0.25 * xy).rgb;
                return float4(linear, 1.0);
            }
        )");

        fCrosstalkAndChunk16x16Effect = makeEffect(kCrosstalkAndChunk16x16Code,
                                                   "RE_MouriMap_CrossTalkAndChunk16x16Effect");

        static const SkString kChunk8x8Code(R"(
            uniform shader img;
            vec4 main(vec2 xy) {
                return float4(img.eval(0.33 * xy).rgb, 1.0);
            }
        )");

        fChunk8x8Effect = makeEffect(kChunk8x8Code, "RE_MouriMap_Chunk8x8Effect");


        static const SkString kBlurCode(R"(
            uniform shader img;
            vec4 main(vec2 xy) {
                return float4(img.eval(0.4 * xy).rgb, 0.0);
            }
        )");

        fBlurEffect = makeEffect(kBlurCode, "RE_MouriMap_BlurEffect");

        static const SkString kTonemapCode(R"(
            uniform shader image;
            uniform shader lux;
            vec4 main(vec2 xy) {
                float localMax = lux.eval(xy * 0.4).r;
                float4 rgba = image.eval(0.5 * xy);
                float3 linear = rgba.rgb * 0.7;

                return float4(linear, rgba.a);
            }
        )");

        fToneMapEffect = makeEffect(kTonemapCode, "RE_MouriMap_TonemapEffect");
    }

    sk_sp<SkRuntimeEffect> crosstalkAndChunk16x16Effect() const {
        return fCrosstalkAndChunk16x16Effect;
    }
    sk_sp<SkRuntimeEffect> chunk8x8Effect() const { return fChunk8x8Effect; }
    sk_sp<SkRuntimeEffect> blurEffect() const { return fBlurEffect; }
    sk_sp<SkRuntimeEffect> toneMapEffect() const { return fToneMapEffect; }

private:
    sk_sp<SkRuntimeEffect> fCrosstalkAndChunk16x16Effect;
    sk_sp<SkRuntimeEffect> fChunk8x8Effect;
    sk_sp<SkRuntimeEffect> fBlurEffect;
    sk_sp<SkRuntimeEffect> fToneMapEffect;
};

const MouriMap& MouriMap() {
    static class MouriMap MouriMap;

    return MouriMap;
}

} // anonymous namespace

// TODO(b/426601394): Update this to take an SkColorInfo for the input image.
// The other MouriMap* precompile paint options should use a linear SkColorInfo
// derived from this same input image.
skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16Passthrough() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> crosstalk = PrecompileRuntimeEffects::MakePrecompileShader(
            MouriMap().crosstalkAndChunk16x16Effect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(crosstalk) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16Premul() {
    // This usage of kUnpremul is non-obvious. It acts to short circuit the identity-colorspace
    // optimization for runtime effects. In this case, the Pipeline requires a
    // ColorSpaceTransformPremul instead of the (optimized) Passthrough.
    SkColorInfo ci { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> crosstalk = PrecompileRuntimeEffects::MakePrecompileShader(
            MouriMap().crosstalkAndChunk16x16Effect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(crosstalk) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapChunk8x8Effect() {
    SkColorInfo ci { kRGBA_F16_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear() };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> chunk8x8 = PrecompileRuntimeEffects::MakePrecompileShader(
            MouriMap().chunk8x8Effect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(chunk8x8) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapBlur() {
    SkColorInfo ci { kRGBA_F16_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear() };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> blur = PrecompileRuntimeEffects::MakePrecompileShader(
            MouriMap().blurEffect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(blur) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapToneMap() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> input = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                             { &ci, 1 },
                                                             {});

    SkColorInfo luxCI { kRGBA_F16_SkColorType,
                        kPremul_SkAlphaType,
                        SkColorSpace::MakeSRGBLinear() };
    sk_sp<PrecompileShader> lux = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                            { &luxCI, 1 },
                                                            {});

    sk_sp<PrecompileShader> toneMap = PrecompileRuntimeEffects::MakePrecompileShader(
            MouriMap().toneMapEffect(),
            { { std::move(input) }, { std::move(lux) } });
    sk_sp<PrecompileShader> inLinear =
            toneMap->makeWithWorkingColorSpace(luxCI.refColorSpace());

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(inLinear) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}


skgpu::graphite::PaintOptions KawaseBlurLowSrcSrcOver() {
    static const SkString kLowSampleBlurCode(R"(
        uniform shader img;

        half4 main(float2 xy) {
            half3 c = img.eval(0.55 * xy).rgb;
            return half4(c, 1.0);
        }
    )");

    sk_sp<SkRuntimeEffect> lowSampleBlurEffect = makeEffect(
            kLowSampleBlurCode,
            "RE_KawaseBlurDualFilter_LowSampleBlurEffect");

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> kawase = PrecompileRuntimeEffects::MakePrecompileShader(
            std::move(lowSampleBlurEffect),
            { { img } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(kawase) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc, SkBlendMode::kSrcOver });
    return paintOptions;
}

skgpu::graphite::PaintOptions KawaseBlurHighSrc() {
    SkString kHighSampleBlurCode(R"(
        uniform shader img;

        half4 main(float2 xy) {
            half3 c = img.eval(0.6 * xy).rgb;
            return half4(c * 0.5, 1.0);
        }
    )");

    sk_sp<SkRuntimeEffect> highSampleBlurEffect = makeEffect(
            kHighSampleBlurCode,
            "RE_KawaseBlurDualFilter_HighSampleBlurEffect");

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> kawase = PrecompileRuntimeEffects::MakePrecompileShader(
            std::move(highSampleBlurEffect),
            { { img } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(kawase) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

skgpu::graphite::PaintOptions BlurFilterMix() {
    static const SkString kMixCode(R"(
        uniform shader img1;
        uniform shader img2;

        half4 main(float2 xy) {
            return half4(mix(img1.eval(xy), img2.eval(xy), 0.5)).rgb1;
        }
    )");

    sk_sp<SkRuntimeEffect> mixEffect = makeEffect(kMixCode, "RE_BlurFilter_MixEffect");

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> mix = PrecompileRuntimeEffects::MakePrecompileShader(
            std::move(mixEffect),
            { { img }, { img } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(mix) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

#if defined(SK_VULKAN)

PaintOptions ImagePremulYCbCr238Srcover(bool narrow) {
    PaintOptions paintOptions;

    // HardwareImage(3: kHoAAO4AAAAAAAAA)
    paintOptions.setShaders({ vulkan_ycbcr_image_shader(238,
                                                        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                                        narrow ? VK_SAMPLER_YCBCR_RANGE_ITU_NARROW
                                                               : VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                                        VK_CHROMA_LOCATION_MIDPOINT) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions TransparentPaintImagePremulYCbCr238Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHoAAO4AAAAAAAAA)
    paintOptions.setShaders({ vulkan_ycbcr_image_shader(238,
                                                        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                                        VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                                        VK_CHROMA_LOCATION_MIDPOINT) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions ImagePremulYCbCr240Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHIAAPAAAAAAAAAA)
    paintOptions.setShaders({ vulkan_ycbcr_image_shader(240,
                                                        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                                        VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                                        VK_CHROMA_LOCATION_MIDPOINT) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions TransparentPaintImagePremulYCbCr240Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHIAAPAAAAAAAAAA)
    paintOptions.setShaders({ vulkan_ycbcr_image_shader(240,
                                                        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
                                                        VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
                                                        VK_CHROMA_LOCATION_MIDPOINT) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16YCbCr247() {
    PaintOptions paintOptions;

    // HardwareImage(3: kEwAAPcAAAAAAAAA)
    sk_sp<PrecompileShader> img = vulkan_ycbcr_image_shader(
            247,
            VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
            VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
            VK_CHROMA_LOCATION_COSITED_EVEN,
            /*pqCS=*/true);

    sk_sp<PrecompileShader> crosstalk = PrecompileRuntimeEffects::MakePrecompileShader(
            MouriMap().crosstalkAndChunk16x16Effect(),
            { { std::move(img) } });

    paintOptions.setShaders({ std::move(crosstalk) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

#endif // SK_VULKAN


namespace {

class EdgeExtension {
public:
    EdgeExtension() {
        // The following code block is just a stub for the Android code. For Skia's testing
        // purposes it only needs to have the same name and number of children as the real code.
        // When the following PaintOptions are used in Android the real SkSL must be supplied.
        static const SkString kEdgeExtensionCode(R"(
            uniform shader img;

            vec4 main(vec2 xy) {
                float3 sample = img.eval(0.115 * xy).rgb;
                return float4(sample, 1.0);
            }
        )");

        fEdgeExtensionEffect = makeEffect(kEdgeExtensionCode, "RE_EdgeExtensionEffect");
        SkASSERT(fEdgeExtensionEffect);
    }

    sk_sp<SkRuntimeEffect> edgeExtensionEffect() const { return fEdgeExtensionEffect; }

private:
    sk_sp<SkRuntimeEffect> fEdgeExtensionEffect;
};

const EdgeExtension& EdgeExtensionSingleton() {
    static class EdgeExtension sEdgeExtension;

    return sEdgeExtension;
}

} // anonymous namespace


skgpu::graphite::PaintOptions EdgeExtensionPassthroughSrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };

    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            EdgeExtensionSingleton().edgeExtensionEffect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(edgeEffect) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

skgpu::graphite::PaintOptions EdgeExtensionPremulSrcover() {
    // This usage of kUnpremul is non-obvious. It acts to short circuit the identity-colorspace
    // optimization for runtime effects. In this case, the Pipeline requires a
    // ColorSpaceTransformPremul instead of the (optimized) Passthrough.
    SkColorInfo ci { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr };

    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            EdgeExtensionSingleton().edgeExtensionEffect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(edgeEffect) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}



skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPassthroughMatrixCFDitherSrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            EdgeExtensionSingleton().edgeExtensionEffect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(edgeEffect) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    paintOptions.setDither(true);

    return paintOptions;
}

skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPassthroughSrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            EdgeExtensionSingleton().edgeExtensionEffect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(edgeEffect) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);

    return paintOptions;
}

skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPremulSrcover() {
    // This usage of kUnpremul is non-obvious. It acts to short circuit the identity-colorspace
    // optimization for runtime effects. In this case, the Pipeline requires a
    // ColorSpaceTransformPremul instead of the (optimized) Passthrough.
    SkColorInfo ci { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr };

    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            EdgeExtensionSingleton().edgeExtensionEffect(),
            { { std::move(img) } });

    PaintOptions paintOptions;
    paintOptions.setShaders({ std::move(edgeEffect) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);

    return paintOptions;
}

// clang-format on

// =======================================
//         RenderPassProperties
// =======================================
// NOTE: keep in sync with upstream external/skia/tests/graphite/precompile/AndroidPaintOptions.cpp
// clang-format off

// Single sampled R w/ just depth
const skgpu::graphite::RenderPassProperties kR_1_D {
        skgpu::graphite::DepthStencilFlags::kDepth,
        kAlpha_8_SkColorType,
        /* fDstCS= */ nullptr,
        /* fRequiresMSAA= */ false
};

// Single sampled RGBA w/ just depth
const skgpu::graphite::RenderPassProperties kRGBA_1_D {
        skgpu::graphite::DepthStencilFlags::kDepth,
        kRGBA_8888_SkColorType,
        /* fDstCS= */ nullptr,
        /* fRequiresMSAA= */ false
};

// The same as kRGBA_1_D but w/ an SRGB colorSpace
const skgpu::graphite::RenderPassProperties kRGBA_1_D_SRGB {
        skgpu::graphite::DepthStencilFlags::kDepth,
        kRGBA_8888_SkColorType,
        SkColorSpace::MakeSRGB(),
        /* fRequiresMSAA= */ false
};

// MSAA RGBA w/ depth and stencil
const skgpu::graphite::RenderPassProperties kRGBA_4_DS {
        skgpu::graphite::DepthStencilFlags::kDepthStencil,
        kRGBA_8888_SkColorType,
        /* fDstCS= */ nullptr,
        /* fRequiresMSAA= */ true
};

// The same as kRGBA_4_DS but w/ an SRGB colorSpace
const skgpu::graphite::RenderPassProperties kRGBA_4_DS_SRGB {
        skgpu::graphite::DepthStencilFlags::kDepthStencil,
        kRGBA_8888_SkColorType,
        SkColorSpace::MakeSRGB(),
        /* fRequiresMSAA= */ true
};

// Single sampled RGBA16F w/ just depth
const skgpu::graphite::RenderPassProperties kRGBA16F_1_D {
        skgpu::graphite::DepthStencilFlags::kDepth,
        kRGBA_F16_SkColorType,
        /* fDstCS= */ nullptr,
        /* fRequiresMSAA= */ false
};

// The same as kRGBA16F_1_D but w/ an SRGB colorSpace
const skgpu::graphite::RenderPassProperties kRGBA16F_1_D_SRGB {
        skgpu::graphite::DepthStencilFlags::kDepth,
        kRGBA_F16_SkColorType,
        SkColorSpace::MakeSRGB(),
        /* fRequiresMSAA= */ false
};

// The same as kRGBA16F_1_D but w/ a linear SRGB colorSpace
const skgpu::graphite::RenderPassProperties kRGBA16F_1_D_Linear {
        skgpu::graphite::DepthStencilFlags::kDepth,
        kRGBA_F16_SkColorType,
        SkColorSpace::MakeSRGBLinear(),
        /* fRequiresMSAA= */ false
};

// clang-format on

const RenderPassProperties kCombo_RGBA_1D_4DS[2] = { kRGBA_1_D, kRGBA_4_DS };
const RenderPassProperties kCombo_RGBA_1D_4DS_SRGB[2] = { kRGBA_1_D_SRGB, kRGBA_4_DS_SRGB };
const RenderPassProperties kCombo_RGBA_1D_SRGB_w16F[2] = { kRGBA_1_D_SRGB, kRGBA16F_1_D_SRGB };

// =======================================
//            DrawTypeFlags
// =======================================
// NOTE: keep in sync with upstream external/skia/tests/graphite/precompile/AndroidPaintOptions.cpp
// clang-format off

constexpr bool kWithAnalyticClip = true;

constexpr DrawTypeFlags kRRectAndNonAARect =
        static_cast<DrawTypeFlags>(DrawTypeFlags::kAnalyticRRect |
                                   DrawTypeFlags::kNonAAFillRect);

// clang-format on

void VisitPrecompileSettings(skgpu::graphite::PrecompileContext* precompileContext,
                             const std::function<void(skgpu::graphite::PrecompileContext*,
                                                      const PrecompileSettings&,
                                                      int index)>& func) {

    // For non-Vulkan configs, these settings cover 126 of the 170 cases in 'kCases'.
    // They create 153 Pipelines so only modestly over-generate (27 extra Pipelines - 18%).
    //
    // For Vulkan configs, the Vulkan-specific PrecompileSettings handle 24 more cases and
    // add 29 more Pipelines.
    //
    // These are sorted into groups based on (first) PaintOptions creation function and
    // then Render Pass Properties.
    // This helps observe DrawTypeFlags distributions.
    const PrecompileSettings kPrecompileCases[] = {
        // 100% (1/1) handles: 0
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D },

        // 100% (4/4) handles: 22 23 42 43
        { Builder().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (4/4) handles: 63 72 (+2 matching synthetic)
        { Builder().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // 100% (1/1) handles: 1
        { Builder().hwImg(kSRGB).srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 100% (4/4) handles: 24 44 45 110
        { Builder().hwImg(kSRGB).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 100% (4/4) handles: 9 28 95 106
        { Builder().transparent().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (4/4) handles 61 66 (+2 matching synthetic)
        { Builder().transparent().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // 100% (2/2) handles 10 29
        { Builder().transparent().hwImg(kSRGB).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB },

        // 63% (5/8) handles 27 56 57 58 94
        { Builder().src().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 75% (3/4) handles 74 86 (+1 matching synthetic)
        { Builder().srcOver(),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // 10: 75% (3/4) handles 19 38 128
        { Builder().hwImg(kPremul).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 75% (3/4) handles 12 123 124
        { Builder().transparent().hwImg(kPremul).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (2/2) handles 14 30
        { Builder().transparent().hwImg(kPremul).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // 100% (2/2) handles 68 (+1 matching synthetic)
        { Builder().transparent().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // 75% (3/4) handles 16 32 33
        { Builder().hwImg(kPremul).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (2/2) handles 69 (+1 matching synthetic)
        { Builder().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // 100% (2/2) handles 15 31
        { Builder().transparent().hwImg(kSRGB).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB },

        // 75% (3/4) handles 17 34 35
        { Builder().hwImg(kSRGB).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 50% (1/2) handles 77 - due to the w/o msaa load variants not being used
        { Builder().hwImg(kSRGB).matrixCF().dither().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_4_DS_SRGB },

        // 67% (2/3) handles 37 70 - due to the w/o msaa load variants not being used
        { Builder().hwImg(kAlphaSRGB).matrixCF().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS_SRGB },

        // 20: 100% (2/2) handles 41 100
        { Builder().hwImg(kPremul).src(),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // 100% (1/1) handles 59
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kPerEdgeAAQuad,
          kRGBA_1_D },

        // 100% (2/2) handles 71 (+1 matching synthetic)
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // TODO(b/426601394): Group these paint option settings into a function that accepts an
        // input image color space so that the intermediate linear color spaces adapt correctly.
        // 100% (1/1) handles 5
        { MouriMapBlur(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 100% (1/1) handles 55
        { MouriMapToneMap(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 7
        { MouriMapCrosstalkAndChunk16x16Passthrough(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 100% (1/1) handles 6
        { MouriMapChunk8x8Effect(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 100% (2/2) handles 52 53
        { KawaseBlurLowSrcSrcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (1/1) handles 51
        { KawaseBlurHighSrc(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (2/2) handles 49 99
        { BlurFilterMix(),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // These two are solid colors drawn w/ a LinearEffect

        // 30: 100% (1/1) handles 4
        { LinearEffect("UNKNOWN__SRGB__false__UNKNOWN",
                       ChildType::kSolidColor,
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 100% (1/1) handles 54
        { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                       ChildType::kSolidColor,
                       SkBlendMode::kSrc),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB },

        // 100% (2/2) handles 2 141
        { LinearEffect("UNKNOWN__SRGB__false__UNKNOWN",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_SRGB_w16F },

        // 67% (2/3) handles 26 64 - due to the w/o msaa load variants not being used
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS_SRGB },

        // 100% (2/2) handles 139 140
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 67% (2/3) handles 11 62 - due to the w/o msaa load variants not being used
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS_SRGB },

        // The next 3 have a RE_LinearEffect and a MatrixFilter along w/ different ancillary
        // additions
        // 100% (1/1) handles 20
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 13
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false,
                       /* matrixColorFilter= */ true),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 18
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 103
        { LinearEffect("V0_SRGB__V0_SRGB__true__UNKNOWN",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 40: 100% (1/1) handles 114
        { LinearEffect("V0_SRGB__V0_SRGB__true__UNKNOWN",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 108
        { LinearEffect("0x188a0000__V0_SRGB__true__0x9010000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 113
        { LinearEffect("0x188a0000__V0_SRGB__true__0x9010000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 120
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 131
        { LinearEffect("0x188a0000__DISPLAY_P3__false__0x90a0000",
                       ChildType::kHWTexture,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D_SRGB },

        // 62% (15/24) handles 65 75 76 78 80 81 83 84 (+7 synthetic for non-convex draws)
        { Builder().srcOver(),
          DrawTypeFlags::kNonSimpleShape,
          kRGBA_4_DS },

        // Note: this didn't get folded into #2 since the RRect draw isn't appearing w/ a clip
        // 50% (1/2) handles: 91
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // Note: this didn't get folded into #9 since the RRect draw isn't appearing w/ a clip
        // 75% (3/4) handles 89 92 (144)
        { Builder().src().srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // 50% (1/2) handles 60
        { {}, // ignored
          DrawTypeFlags::kDropShadows,
          kRGBA_1_D },

        // 75% (3/4) handles 82 85 (145)
        { {}, // ignored
          DrawTypeFlags::kDropShadows,
          kRGBA_4_DS },

        // 50: 100% (2/2) handles 96 112
        { EdgeExtensionPremulSrcover(),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // 100% (1/1) handles: 126
        { TransparentPaintEdgeExtensionPassthroughMatrixCFDitherSrcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (1/1) handles 97
        { TransparentPaintEdgeExtensionPassthroughSrcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (1/1) handles 98
        { TransparentPaintEdgeExtensionPremulSrcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 100% (2/2) handles 137 138
        { EdgeExtensionPassthroughSrcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 50% (1/2) handles 101
        { Builder().hwImg(kAlpha, kClamp).src(),
          DrawTypeFlags::kNonAAFillRect,
          kR_1_D },

        // 100% (2/2) handles 109 129
        { Builder().hwImg(kSRGB).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB },

        // 100% (1/1) handles 104
        { MouriMapCrosstalkAndChunk16x16Premul(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 67% (2/3) handles 107 146
        { Builder().blend().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS },

        // 100% (1/1) handles 122
        { Builder().blend().srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D },

        // 60: 100% (1/1) handles 117
        { Builder().transparent().blend().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

#if defined(SK_VULKAN) && defined(SK_BUILD_FOR_ANDROID)

        // 238 Full range (kHIAAO4AAAAAAAAA) block ----------------

        // (3/4) 134 135 150
        { ImagePremulYCbCr238Srcover(/* narrow= */ false),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS_SRGB,
          kWithAnalyticClip },

        // 238 Narrow range (kHoAAO4AAAAAAAAA) block ----------------

        // 75% (3/4) handles 25 47 48
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 75% (3/4) handles 93 119 148
        { TransparentPaintImagePremulYCbCr238Srcover(),
          kRRectAndNonAARect,
          kCombo_RGBA_1D_4DS },

        // 100% (1/1) handles 149
        { TransparentPaintImagePremulYCbCr238Srcover(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // 100% (2/2) handles 87 88
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // Note: this didn't get folded into the above since the RRect draw isn't appearing w/ a
        // clip
        // 100% (1/1) handles 90
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // 240 (kHIAAPAAAAAAAAAA) block ----------------

        // 100% (2/2) handles 46 136
        { ImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 100% (2/2) handles (73) 151
        { ImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // 67% (2/3) handles 67 118 (164)
        { TransparentPaintImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS },

        // 247 (kEwAAPcAAAAAAAAA) block ----------------

        // 70: 100% (1/1) handles 8
        { MouriMapCrosstalkAndChunk16x16YCbCr247(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // The next 2 have the same PaintOptions but different destination surfaces

        // 75% (3/4) handles 21 39 40
        { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                       ChildType::kHWTextureYCbCr247,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ true),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 100% (1/1) handles 79
        { LinearEffect("BT2020_ITU_PQ__BT2020__false__UNKNOWN",
                       ChildType::kHWTextureYCbCr247,
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ true),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS_SRGB },
#endif
    };

    for (size_t i = 0; i < std::size(kPrecompileCases); ++i) {
        func(precompileContext, kPrecompileCases[i], i);
    }
}

#endif // SK_GRAPHITE
