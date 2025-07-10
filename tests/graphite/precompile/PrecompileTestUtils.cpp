/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/PrecompileContextPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"
#include "tools/graphite/UniqueKeyUtils.h"

#if defined (SK_VULKAN)
#include "include/gpu/graphite/vk/precompile/VulkanPrecompileShader.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/base/SkBase64.h"
#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"
#endif // SK_VULKAN

#include <cstring>
#include <set>

using namespace skgpu::graphite;
using PrecompileShaders::ImageShaderFlags;

using ::skgpu::graphite::DrawTypeFlags;
using ::skgpu::graphite::PaintOptions;
using ::skgpu::graphite::RenderPassProperties;

namespace PrecompileTestUtils {

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

void Base642YCbCr(const char* str) {

    size_t expectedDstLength;
    SkBase64::Error error = SkBase64::Decode(str, strlen(str), nullptr, &expectedDstLength);
    if (error != SkBase64::kNoError) {
        return;
    }

    if (expectedDstLength % 4 != 0) {
        return;
    }

    int numInts = expectedDstLength / 4;
    skia_private::AutoTMalloc<uint32_t> dst(numInts);
    size_t actualDstLength;
    error = SkBase64::Decode(str, strlen(str), dst, &actualDstLength);
    if (error != SkBase64::kNoError || expectedDstLength != actualDstLength) {
        return;
    }

    SamplerDesc s(dst[0], dst[1], dst[2]);

    SkDebugf("tileModes: %d %d filterMode: %d mipmap: %d ",
             static_cast<int>(s.tileModeX()),
             static_cast<int>(s.tileModeY()),
             static_cast<int>(s.filterMode()),
             static_cast<int>(s.mipmap()));

    skgpu::VulkanYcbcrConversionInfo info =
            VulkanYcbcrConversion::FromImmutableSamplerInfo(s.immutableSamplerInfo());

    SkDebugf("VulkanYcbcrConversionInfo: format: %d extFormat: %llu model: %d range: %d "
             "xOff: %d yOff: %d filter: %d explicit: %u features: %u components: %d %d %d %d\n",
             info.fFormat,
             (unsigned long long) info.fExternalFormat,
             info.fYcbcrModel,
             info.fYcbcrRange,
             info.fXChromaOffset,
             info.fYChromaOffset,
             info.fChromaFilter,
             info.fForceExplicitReconstruction,
             info.fFormatFeatures,
             info.fComponents.r,
             info.fComponents.g,
             info.fComponents.b,
             info.fComponents.a);
}

#endif // SK_VULKAN

namespace {

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
                                           bool paintColorIsOpaque,
                                           bool matrixColorFilter,
                                           bool dither) {
    static LinearEffectSingleton gLinearEffectSingleton;

    PaintOptions paintOptions;

    sk_sp<SkRuntimeEffect> linearEffect = gLinearEffectSingleton.findOrCreate(parameterStr);
    sk_sp<PrecompileShader> child = create_child_shader(childType);

    paintOptions.setShaders({ PrecompileRuntimeEffects::MakePrecompileShader(
                                            std::move(linearEffect),
                                            { { std::move(child) } }) });
    if (matrixColorFilter) {
        paintOptions.setColorFilters({PrecompileColorFilters::Matrix()});
    }
    paintOptions.setBlendModes({ blendMode });
    paintOptions.setPaintColorIsOpaque(paintColorIsOpaque);
    paintOptions.setDither(dither);

    return paintOptions;
}

namespace {

[[maybe_unused]] void find_duplicates(SkSpan<const PipelineLabel> cases) {
    for (size_t i = 0; i < std::size(cases); ++i) {
        for (size_t j = i+1; j < std::size(cases); ++j) {
            if (!strcmp(cases[i].fString, cases[j].fString)) {
                SkDebugf("Duplicate %zu && %zu\n", i, j);
            }
        }
    }
}

std::string rm_whitespace(const std::string& s) {
    auto start = s.find_first_not_of(' ');
    auto end = s.find_last_not_of(' ');
    return s.substr(start, (end - start) + 1);
}

} // anonymous namespace

PipelineLabelInfoCollector::PipelineLabelInfoCollector(SkSpan<const PipelineLabel> cases,
                                                       SkipFunc skip) {
    for (size_t i = 0; i < std::size(cases); ++i) {
        const char* testStr = cases[i].fString;

        if (skip(testStr)) {
            fMap.insert({ testStr, PipelineLabelInfo(i, PipelineLabelInfo::kSkipped) });
        } else {
            fMap.insert({ testStr, PipelineLabelInfo(i) });
        }
    }
}

int PipelineLabelInfoCollector::processLabel(const std::string& precompiledLabel,
                                             int precompileCase) {
    ++fNumLabelsProcessed;

    auto result = fMap.find(precompiledLabel.c_str());
    if (result == fMap.end()) {
        SkDEBUGCODE(auto prior = fOverGenerated.find(precompiledLabel);)
        SkASSERTF(prior == fOverGenerated.end(),
                  "duplicate (unused) Pipeline found for cases %d %d:\n%s\n",
                  prior->second.fOriginatingSetting,
                  precompileCase,
                  precompiledLabel.c_str());
        fOverGenerated.insert({ precompiledLabel, OverGenInfo(precompileCase) });
        return -1;
    }

    // We expect each PrecompileSettings case to handle disjoint sets of labels. If this
    // assert fires some pair of PrecompileSettings are handling the same case.
    SkASSERTF(result->second.fPrecompileCase == PipelineLabelInfo::kUninit,
              "cases %d and %d cover the same label",
              result->second.fPrecompileCase, precompileCase);
    result->second.fPrecompileCase = precompileCase;
    return result->second.fCasesIndex;
}

void PipelineLabelInfoCollector::finalReport() {
    std::vector<int> skipped, missed;
    int numCovered = 0, numIntentionallySkipped = 0, numMissed = 0;

    for (const auto& iter : fMap) {
        if (iter.second.fPrecompileCase == PipelineLabelInfo::kSkipped) {
            ++numIntentionallySkipped;
            skipped.push_back(iter.second.fCasesIndex);
        } else if (iter.second.fPrecompileCase == PipelineLabelInfo::kUninit) {
            ++numMissed;
            missed.push_back(iter.second.fCasesIndex);
        } else {
            SkASSERT(iter.second.fPrecompileCase >= 0);
            ++numCovered;
        }
    }

    SkASSERT(numMissed == (int) missed.size());
    SkASSERT(numIntentionallySkipped == (int) skipped.size());

    SkDebugf("-----------------------\n");
    sort(missed.begin(), missed.end());
    SkDebugf("not covered: ");
    for (int i : missed) {
        SkDebugf("%d, ", i);
    }
    SkDebugf("\n");

    sort(skipped.begin(), skipped.end());
    SkDebugf("skipped: ");
    for (int i : skipped) {
        SkDebugf("%d, ", i);
    }
    SkDebugf("\n");

    SkASSERT(numCovered + static_cast<int>(fOverGenerated.size()) == fNumLabelsProcessed);

    SkDebugf("covered %d notCovered %d skipped %d total %zu\n",
             numCovered,
             numMissed,
             numIntentionallySkipped,
             fMap.size());
    SkDebugf("%d Pipelines were generated\n", fNumLabelsProcessed);
    SkDebugf("of that %zu Pipelines were over-generated:\n", fOverGenerated.size());
#if 0 // enable to print out a list of the over-generated Pipeline labels
    for (const auto& s : fOverGenerated) {
        SkDebugf("from %d: %s\n", s.second.fOriginatingSetting, s.first.c_str());
    }
#endif
}


// Precompile with the provided PrecompileSettings then verify that:
//   1) some case in 'kCases' is covered
//   2) more than 40% of the generated Pipelines are in kCases
void RunTest(skgpu::graphite::PrecompileContext* precompileContext,
             skiatest::Reporter* reporter,
             SkSpan<const PrecompileSettings> precompileSettings,
             int precompileSettingsIndex,
             SkSpan<const PipelineLabel> cases,
             PipelineLabelInfoCollector* collector) {
    using namespace skgpu::graphite;

    const PrecompileSettings& settings = precompileSettings[precompileSettingsIndex];

    precompileContext->priv().globalCache()->resetGraphicsPipelines();

    Precompile(precompileContext,
               settings.fPaintOptions,
               static_cast<DrawTypeFlags>(settings.fDrawTypeFlags.value()),
               settings.fRenderPassProps);

    if (settings.fAnalyticClipping) {
        SkASSERT(!(settings.fDrawTypeFlags & DrawTypeFlags::kAnalyticClip));

        SkEnumBitMask<DrawTypeFlags> newFlags = settings.fDrawTypeFlags |
                                                DrawTypeFlags::kAnalyticClip;

        Precompile(precompileContext,
                   settings.fPaintOptions,
                   static_cast<DrawTypeFlags>(newFlags.value()),
                   settings.fRenderPassProps);
    }

    std::set<std::string> generatedLabels;

    {
        const RendererProvider* rendererProvider = precompileContext->priv().rendererProvider();
        const ShaderCodeDictionary* dict = precompileContext->priv().shaderCodeDictionary();

        std::vector<skgpu::UniqueKey> generatedKeys;

        UniqueKeyUtils::FetchUniqueKeys(precompileContext, &generatedKeys);

        for (const skgpu::UniqueKey& key : generatedKeys) {
            GraphicsPipelineDesc pipelineDesc;
            RenderPassDesc renderPassDesc;
            UniqueKeyUtils::ExtractKeyDescs(precompileContext, key, &pipelineDesc, &renderPassDesc);

            const RenderStep* renderStep = rendererProvider->lookup(pipelineDesc.renderStepID());
            std::string tmp = GetPipelineLabel(dict, renderPassDesc, renderStep,
                                               pipelineDesc.paintParamsID());
            generatedLabels.insert(rm_whitespace(tmp));
        }
    }

    std::vector<bool> localMatches;
    std::vector<size_t> matchesInCases;

    for (const std::string& g : generatedLabels) {
        int matchInCases = collector->processLabel(g, precompileSettingsIndex);
        localMatches.push_back(matchInCases >= 0);

        if (matchInCases >= 0) {
            matchesInCases.push_back(matchInCases);
        }
    }

    REPORTER_ASSERT(reporter, matchesInCases.size() >= 1,   // This tests requirement 1, above
                    "%d: num matches: %zu", precompileSettingsIndex, matchesInCases.size());
    float utilization = ((float) matchesInCases.size())/generatedLabels.size();
    REPORTER_ASSERT(reporter, utilization >= 0.4f,         // This tests requirement 2, above
                    "%d: utilization: %f", precompileSettingsIndex, utilization);

#if defined(PRINT_COVERAGE)
    // This block will print out all the cases in 'kCases' that the given PrecompileSettings
    // covered.
    sort(matchesInCases.begin(), matchesInCases.end());
    SkDebugf("// %d: %d%% (%zu/%zu) handles: ",
             precompileSettingsIndex,
             SkScalarRoundToInt(utilization * 100),
             matchesInCases.size(), generatedLabels.size());
    for (size_t h : matchesInCases) {
        SkDebugf("%zu ", h);
    }
    SkDebugf("\n");
#endif

#if defined(PRINT_GENERATED_LABELS)
    // This block will print out all the labels from the given PrecompileSettings marked with
    // whether they were found in 'kCases'. This is useful for analyzing the set of Pipelines
    // generated by a single PrecompileSettings and is usually used along with 'kChosenCase'.
    SkASSERT(localMatches.size() == generatedLabels.size());

    int index = 0;
    for (const std::string& g : generatedLabels) {
        SkDebugf("%c %d: %s\n", localMatches[index] ? 'h' : ' ', index, g.c_str());
        ++index;
    }
#endif
}

} // namespace PrecompileTestUtils

#endif // SK_GRAPHITE
