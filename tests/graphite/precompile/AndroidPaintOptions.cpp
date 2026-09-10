/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GRAPHITE)
#include "include/core/SkRefCnt.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/PrecompileContext.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
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

#include "tests/graphite/precompile/AndroidRuntimeEffectManager.h"

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


skgpu::VulkanYcbcrConversionInfo ycbcr_info(uint64_t externalFormat,
                                            VkSamplerYcbcrModelConversion model,
                                            VkSamplerYcbcrRange range,
                                            VkChromaLocation location,
                                            VkFilter filter = VK_FILTER_LINEAR,
                                            bool samplerFilterMustMatchChromaFilter = true,
                                            bool supportsLinearFilter = false) {
    VkComponentMapping components = { VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY,
                                      VK_COMPONENT_SWIZZLE_IDENTITY };

    VkFormatFeatureFlags formatFeatures = 0;
    if (!samplerFilterMustMatchChromaFilter) {
        formatFeatures |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT;
    }
    if (supportsLinearFilter) {
        formatFeatures |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    }

    return skgpu::VulkanYcbcrConversionInfo(externalFormat, model, range, location, location,
                                            filter, /*forceExplicitReconstruction=*/false,
                                            components, formatFeatures);
}

sk_sp<PrecompileShader> vulkan_ycbcr_image_shader(const skgpu::VulkanYcbcrConversionInfo& ycbcrInfo,
                                                  const SkColorInfo& ci) {
    return PrecompileShaders::VulkanYCbCrImage(ycbcrInfo,
                                               PrecompileShaders::ImageShaderFlags::kExcludeCubic,
                                               { &ci, 1 },
                                               {});
}

} // anonymous namespace
#endif // SK_VULKAN

namespace {

sk_sp<PrecompileShader> create_hw_image_precompile_shader() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                           SkNamedGamut::kAdobeRGB) };

    return PrecompileShaders::Image(PrecompileShaders::ImageShaderFlags::kExcludeCubic,
                                    { &ci, 1 },
                                    {});
}

} // anonymous namespace

skgpu::graphite::PaintOptions LinearEffect(sk_sp<SkRuntimeEffect> linearEffect,
                                           sk_sp<PrecompileShader> childShader,
                                           SkBlendMode blendMode,
                                           bool paintColorIsOpaque = true,
                                           bool matrixColorFilter = false,
                                           bool dither = false,
                                           sk_sp<SkColorSpace> cs = nullptr) {
    PaintOptions paintOptions;
    sk_sp<PrecompileShader> linearEffectShader = PrecompileRuntimeEffects::MakePrecompileShader(
        std::move(linearEffect),
        {{ {{ std::move(childShader) }} }});
    if (cs) {
        linearEffectShader = linearEffectShader->makeWithWorkingColorSpace(nullptr, cs);
    }
    paintOptions.setShaders({{ std::move(linearEffectShader) }});
    if (matrixColorFilter) {
        paintOptions.setColorFilters(SKSPAN_INIT_ONE( PrecompileColorFilters::Matrix() ));
    }
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( blendMode ));
    paintOptions.setPaintColorIsOpaque(paintColorIsOpaque);
    paintOptions.setDither(dither);

    return paintOptions;
}

// =======================================
//         PaintOptions
// =======================================
// NOTE: keep in sync with upstream external/skia/tests/graphite/precompile/AndroidPaintOptions.cpp
// clang-format off

// TODO(b/426601394): Update this to take an SkColorInfo for the input image.
// The other MouriMap* precompile paint options should use a linear SkColorInfo
// derived from this same input image.
skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16Passthrough(
        RuntimeEffectManager& effectManager) {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> crosstalk = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kMouriMap_CrossTalkAndChunk16x16Effect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(crosstalk) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrc ));
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16Premul(
        RuntimeEffectManager& effectManager) {
    // This usage of kUnpremul is non-obvious. It acts to short circuit the identity-colorspace
    // optimization for runtime effects. In this case, the Pipeline requires a
    // ColorSpaceTransformPremul instead of the (optimized) Passthrough.
    SkColorInfo ci { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> crosstalk = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kMouriMap_CrossTalkAndChunk16x16Effect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(crosstalk) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrc ));
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapChunk8x8Effect(RuntimeEffectManager& effectManager) {
    SkColorInfo ci { kRGBA_F16_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear() };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> chunk8x8 = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kMouriMap_Chunk8x8Effect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(chunk8x8) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrc ));
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapBlur(RuntimeEffectManager& effectManager) {
    SkColorInfo ci { kRGBA_F16_SkColorType, kPremul_SkAlphaType, SkColorSpace::MakeSRGBLinear() };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> blur = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kMouriMap_BlurEffect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(blur) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrc ));
    return paintOptions;
}

static sk_sp<PrecompileShader> create_tone_map(RuntimeEffectManager& effectManager) {
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
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kMouriMap_TonemapEffect),
            {{ {{ std::move(input) }}, {{ std::move(lux) }} }});
    sk_sp<PrecompileShader> inLinear = toneMap->makeWithWorkingColorSpace(luxCI.refColorSpace());

    return inLinear;
}

skgpu::graphite::PaintOptions MouriMapToneMap(RuntimeEffectManager& effectManager) {

    PaintOptions paintOptions;
    paintOptions.setShaders({{ create_tone_map(effectManager) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrc ));
    return paintOptions;
}

skgpu::graphite::PaintOptions GainMap(RuntimeEffectManager& effectManager) {

    SkColorInfo hdrCI { kRGBA_F16_SkColorType,
                        kPremul_SkAlphaType,
                        SkColorSpace::MakeSRGB() };
    sk_sp<PrecompileShader> hdr = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                            { &hdrCI, 1 },
                                                            {});

    sk_sp<PrecompileShader> gainMap = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kGainmapEffect),
            {{ {{ create_tone_map(effectManager) }}, {{ std::move(hdr) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(gainMap) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrc ));
    return paintOptions;
}

skgpu::graphite::PaintOptions BoxShadow(RuntimeEffectManager& effectManager) {
    sk_sp<PrecompileShader> boxShadow = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kBoxShadowEffect));

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(boxShadow) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    return paintOptions;
}

skgpu::graphite::PaintOptions BlurFilterMix(RuntimeEffectManager& effectManager) {
    sk_sp<SkRuntimeEffect> mixEffect = effectManager.getKnownRuntimeEffect(
            RuntimeEffectManager::KnownId::kBlurFilter_MixEffect);

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> mix = PrecompileRuntimeEffects::MakePrecompileShader(
            std::move(mixEffect),
            {{ {{ img }}, {{ img }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(mix) }});
    paintOptions.setBlendModes({ SkBlendMode::kSrc, SkBlendMode::kSrcOver });
    return paintOptions;
}

#if defined(SK_VULKAN)

PaintOptions ImagePremulYCbCr238Srcover(bool narrow) {
    PaintOptions paintOptions;

    // HardwareImage(3: kHoAAO4AAAAAAAAA)
    const skgpu::VulkanYcbcrConversionInfo ycbcrInfo = ycbcr_info(
        238,
        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
        narrow ? VK_SAMPLER_YCBCR_RANGE_ITU_NARROW
               : VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
        VK_CHROMA_LOCATION_MIDPOINT);
    const SkColorInfo kRGBA8Premul(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);

    paintOptions.setShaders({{ vulkan_ycbcr_image_shader(ycbcrInfo, kRGBA8Premul) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    return paintOptions;
}

PaintOptions TransparentPaintImagePremulYCbCr238Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHoAAO4AAAAAAAAA)
    const skgpu::VulkanYcbcrConversionInfo ycbcrInfo = ycbcr_info(
        238,
        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
        VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
        VK_CHROMA_LOCATION_MIDPOINT);
    const SkColorInfo kRGBA8Premul(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);

    paintOptions.setShaders({{ vulkan_ycbcr_image_shader(ycbcrInfo, kRGBA8Premul) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions ImagePremulYCbCr240Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHIAAPAAAAAAAAAA)
    const skgpu::VulkanYcbcrConversionInfo ycbcrInfo = ycbcr_info(
        240,
        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
        VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
        VK_CHROMA_LOCATION_MIDPOINT);
    const SkColorInfo kRGBA8Premul(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);

    paintOptions.setShaders({{ vulkan_ycbcr_image_shader(ycbcrInfo, kRGBA8Premul) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    return paintOptions;
}

PaintOptions TransparentPaintImagePremulYCbCr240Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHIAAPAAAAAAAAAA)
    const skgpu::VulkanYcbcrConversionInfo ycbcrInfo = ycbcr_info(
        240,
        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
        VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
        VK_CHROMA_LOCATION_MIDPOINT);
    const SkColorInfo kRGBA8Premul(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);

    paintOptions.setShaders({{ vulkan_ycbcr_image_shader(ycbcrInfo, kRGBA8Premul) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16YCbCr247(
        RuntimeEffectManager& effectManager) {
    PaintOptions paintOptions;

    // HardwareImage(3: kEwAAPcAAAAAAAAA)
    const skgpu::VulkanYcbcrConversionInfo ycbcrInfo = ycbcr_info(
        247,
        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
        VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
        VK_CHROMA_LOCATION_COSITED_EVEN);
    const SkColorInfo kRGBA8PremulPQ(kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType,
                                     SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ,
                                                           SkNamedGamut::kRec2020));

    sk_sp<PrecompileShader> img = vulkan_ycbcr_image_shader(ycbcrInfo, kRGBA8PremulPQ);

    sk_sp<PrecompileShader> crosstalk = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kMouriMap_CrossTalkAndChunk16x16Effect),
            {{ {{ std::move(img) }} }});

    paintOptions.setShaders({{ std::move(crosstalk) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrc ));
    return paintOptions;
}

PaintOptions LinearAndLUTEffectImageYCbCr54(RuntimeEffectManager& effectManager) {
    sk_sp<SkRuntimeEffect> k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    // DCI-P3 sRGB Extended range
                    /* inputDataspace= */ static_cast<ui::Dataspace>(0x188a0000),
                    /* outputDataspace= */ ui::Dataspace::DISPLAY_P3, // DCI-P3 sRGB Full range
                    /* undoPremultipliedAlpha= */ false,
                    // DCI-P3 gamma 2.2 Full range
                    /* fakeOutputDataspace= */ static_cast<ui::Dataspace>(0x90a0000),
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

    sk_sp<SkRuntimeEffect> lutEffect = effectManager.getKnownRuntimeEffect(
            RuntimeEffectManager::KnownId::kLutEffect);

    const skgpu::VulkanYcbcrConversionInfo info = ycbcr_info(
        54,
        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
        VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
        VK_CHROMA_LOCATION_MIDPOINT,
        VK_FILTER_NEAREST,
        /* samplerFilterMustMatchChromaFilter= */ false,
        /* supportsLinearFilter= */ true);
    const SkColorInfo kRGBA8PremulPQ(kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType,
                                     SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ,
                                                           SkNamedGamut::kRec2020));

    sk_sp<PrecompileShader> ycbcr = vulkan_ycbcr_image_shader(info, kRGBA8PremulPQ);

    const SkColorInfo kRGBA8PremulColorSpin(kRGBA_8888_SkColorType,
                                            kPremul_SkAlphaType,
                                            SkColorSpace::MakeSRGB()->makeColorSpin());

    sk_sp<PrecompileShader> lutImg = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                              { kRGBA8PremulColorSpin },
                                                              {});

    sk_sp<PrecompileShader> lutShader = PrecompileRuntimeEffects::MakePrecompileShader(
            std::move(lutEffect),
            {{ {{ std::move(ycbcr) }}, {{ std::move(lutImg) }} }});

    sk_sp<SkColorSpace> cs = SkColorSpace::MakeSRGB()->makeColorSpin();
    sk_sp<PrecompileShader> wrappedLUTShader = lutShader->makeWithWorkingColorSpace(std::move(cs));

    return LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                        std::move(wrappedLUTShader),
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ true,
                        /* matrixColorFilter= */ false,
                        /* dither= */ true,
                        SkColorSpace::MakeSRGBLinear());
}

PaintOptions ImagePremulYCbCr769Srcover(RuntimeEffectManager& effectManager) {
    PaintOptions paintOptions;

    const skgpu::VulkanYcbcrConversionInfo info = ycbcr_info(
        769,
        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709,
        VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
        VK_CHROMA_LOCATION_MIDPOINT,
        VK_FILTER_NEAREST,
        /* samplerFilterMustMatchChromaFilter= */ false,
        /* supportsLinearFilter= */ true);
    const SkColorInfo kRGBA8Premul(kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);

    paintOptions.setShaders({{ vulkan_ycbcr_image_shader(info, kRGBA8Premul) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    return paintOptions;
}

PaintOptions LinearEffectImageYCbCr54(RuntimeEffectManager& effectManager) {
    const sk_sp<SkRuntimeEffect> kBT2020_HLG__UNKNOWN__false__UNKNOWN__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    /* inputDataspace= */ ui::Dataspace::BT2020_HLG,
                    /* outputDataspace= */ ui::Dataspace::UNKNOWN,
                    /* undoPremultipliedAlpha= */ false,
                    /* fakeOutputDataspace= */ ui::Dataspace::UNKNOWN,
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

    const skgpu::VulkanYcbcrConversionInfo info = ycbcr_info(
        54,
        VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
        VK_SAMPLER_YCBCR_RANGE_ITU_FULL,
        VK_CHROMA_LOCATION_MIDPOINT,
        VK_FILTER_NEAREST,
        /* samplerFilterMustMatchChromaFilter= */ false,
        /* supportsLinearFilter= */ true);
    const SkColorInfo kRGBA8PremulPQ(kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType,
                                     SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ,
                                                           SkNamedGamut::kRec2020));

    sk_sp<PrecompileShader> yuvShader = vulkan_ycbcr_image_shader(info, kRGBA8PremulPQ);

    return LinearEffect(kBT2020_HLG__UNKNOWN__false__UNKNOWN__Shader,
                        std::move(yuvShader),
                        SkBlendMode::kSrcOver,
                        /* paintColorIsOpaque= */ true,
                        /* matrixColorFilter= */ false,
                        /* dither= */ true,
                        SkColorSpace::MakeSRGBLinear());
}

#endif // SK_VULKAN


skgpu::graphite::PaintOptions EdgeExtensionPassthroughSrcover(RuntimeEffectManager& effectManager) {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };

    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kEdgeExtensionEffect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(edgeEffect) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    return paintOptions;
}

skgpu::graphite::PaintOptions EdgeExtensionPremulSrcover(RuntimeEffectManager& effectManager) {
    // This usage of kUnpremul is non-obvious. It acts to short circuit the identity-colorspace
    // optimization for runtime effects. In this case, the Pipeline requires a
    // ColorSpaceTransformPremul instead of the (optimized) Passthrough.
    SkColorInfo ci { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr };

    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kEdgeExtensionEffect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(edgeEffect) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    return paintOptions;
}



skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPassthroughMatrixCFDitherSrcover(
        RuntimeEffectManager& effectManager) {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kEdgeExtensionEffect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(edgeEffect) }});
    paintOptions.setColorFilters({{ PrecompileColorFilters::Matrix() }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    paintOptions.setPaintColorIsOpaque(false);
    paintOptions.setDither(true);

    return paintOptions;
}

skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPassthroughSrcover(
        RuntimeEffectManager& effectManager) {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kEdgeExtensionEffect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(edgeEffect) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
    paintOptions.setPaintColorIsOpaque(false);

    return paintOptions;
}

skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPremulSrcover(
        RuntimeEffectManager& effectManager) {
    // This usage of kUnpremul is non-obvious. It acts to short circuit the identity-colorspace
    // optimization for runtime effects. In this case, the Pipeline requires a
    // ColorSpaceTransformPremul instead of the (optimized) Passthrough.
    SkColorInfo ci { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, nullptr };

    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});

    sk_sp<PrecompileShader> edgeEffect = PrecompileRuntimeEffects::MakePrecompileShader(
            effectManager.getKnownRuntimeEffect(
                    RuntimeEffectManager::KnownId::kEdgeExtensionEffect),
            {{ {{ std::move(img) }} }});

    PaintOptions paintOptions;
    paintOptions.setShaders({{ std::move(edgeEffect) }});
    paintOptions.setBlendModes(SKSPAN_INIT_ONE( SkBlendMode::kSrcOver ));
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

const skgpu::graphite::RenderPassProperties kRGBA_1_D_Linear {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGBLinear(),
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
const RenderPassProperties kCombo_RGBA_1D_4DS_16F[3] = { kRGBA_1_D, kRGBA_4_DS, kRGBA16F_1_D };

// =======================================
//            DrawTypeFlags
// =======================================
// NOTE: keep in sync with upstream external/skia/tests/graphite/precompile/AndroidPaintOptions.cpp
// clang-format off

constexpr bool kWithAnalyticClip = true;

constexpr DrawTypeFlags kRRectAndNonAARect =
        static_cast<DrawTypeFlags>(DrawTypeFlags::kAnalyticRRect |
                                   DrawTypeFlags::kNonAAFillRect);

PrecompileSettings linear_and_tonemap(RuntimeEffectManager* effectManager,
                                      sk_sp<SkRuntimeEffect> rte) {
    return { LinearEffect(std::move(rte),
                          create_tone_map(*effectManager),
                          SkBlendMode::kSrcOver,
                          /* paintColorIsOpaque= */ true,
                          /* matrixColorFilter= */ false,
                          /* dither= */ false,
                          SkColorSpace::MakeSRGBLinear()),
             kRRectAndNonAARect,
             kCombo_RGBA_1D_SRGB_w16F };
}

// The 'rrectStyle' flag selects between:
//    rect with and w/o an analytic clip
//    rrects
// This is to avoid overgenerating rrects w/ analytic clips
PrecompileSettings linear_hw_image(sk_sp<SkRuntimeEffect> rte,
                                   bool rrectStyle,
                                   bool opaquePaint = true) {
    return { LinearEffect(rte,
                          create_hw_image_precompile_shader(),
                          SkBlendMode::kSrcOver,
                          /* paintColorIsOpaque= */ opaquePaint,
                          /* matrixColorFilter= */ false,
                          /* dither= */ false,
                          SkColorSpace::MakeSRGBLinear()),
             rrectStyle ? DrawTypeFlags::kAnalyticRRect: DrawTypeFlags::kNonAAFillRect,
             kCombo_RGBA_1D_4DS_SRGB,
             rrectStyle ? false : kWithAnalyticClip };
}

// This is the same as linear_hw_image but with a different color space
PrecompileSettings linear_hw_image_srgb(sk_sp<SkRuntimeEffect> rte,
                                        bool rrectStyle,
                                        bool opaquePaint = true) {
    return { LinearEffect(rte,
                          create_hw_image_precompile_shader(),
                          SkBlendMode::kSrcOver,
                          /* paintColorIsOpaque= */ opaquePaint,
                          /* matrixColorFilter= */ false,
                          /* dither= */ false,
                          SkColorSpace::MakeSRGB()), // note: not MakeSRGBLinear
             rrectStyle ? DrawTypeFlags::kAnalyticRRect: DrawTypeFlags::kNonAAFillRect,
             kCombo_RGBA_1D_4DS_SRGB,
             rrectStyle ? false : kWithAnalyticClip };
}

PrecompileSettings linear_hw_image_matrixfilter(sk_sp<SkRuntimeEffect> rte, bool rrectStyle) {
    return { LinearEffect(rte,
                          create_hw_image_precompile_shader(),
                          SkBlendMode::kSrcOver,
                          /* paintColorIsOpaque= */ true,
                          /* matrixColorFilter= */ true,
                          /* dither= */ false,
                          SkColorSpace::MakeSRGBLinear()),
             rrectStyle ? DrawTypeFlags::kAnalyticRRect: DrawTypeFlags::kNonAAFillRect,
             kRGBA_1_D_SRGB,
             rrectStyle ? false : kWithAnalyticClip };
}

// This is the same as linear_hw_image_matrixfilter but with a different color space and dest
PrecompileSettings linear_hw_image_matrixfilter_srgb(sk_sp<SkRuntimeEffect> rte, bool rrectStyle) {
    return { LinearEffect(rte,
                          create_hw_image_precompile_shader(),
                          SkBlendMode::kSrcOver,
                          /* paintColorIsOpaque= */ true,
                          /* matrixColorFilter= */ true,
                          /* dither= */ false,
                          SkColorSpace::MakeSRGB()), // note: not MakeSRGBLinear
             rrectStyle ? DrawTypeFlags::kAnalyticRRect: DrawTypeFlags::kNonAAFillRect,
             kRGBA_1_D,
             rrectStyle ? false : kWithAnalyticClip };
}

PrecompileSettings linear_hw_image_matrixfilter_dither(sk_sp<SkRuntimeEffect> rte, bool rrectStyle) {
    return { LinearEffect(rte,
                          create_hw_image_precompile_shader(),
                          SkBlendMode::kSrcOver,
                          /* paintColorIsOpaque= */ true,
                          /* matrixColorFilter= */ true,
                          /* dither= */ true,
                          SkColorSpace::MakeSRGBLinear()),
             rrectStyle ? DrawTypeFlags::kAnalyticRRect: DrawTypeFlags::kNonAAFillRect,
             kCombo_RGBA_1D_4DS_SRGB,
             rrectStyle ? false : kWithAnalyticClip };
}

PrecompileSettings linear_solid(sk_sp<SkRuntimeEffect> rte) {
    return { LinearEffect(rte,
                          PrecompileShaders::Color(),
                          SkBlendMode::kSrc,
                          /* paintColorIsOpaque= */ true,
                          /* matrixColorFilter= */ false,
                          /* dither= */ false,
                          SkColorSpace::MakeSRGBLinear()),
             DrawTypeFlags::kNonAAFillRect,
             kRGBA_1_D };
}

// clang-format on

void VisitAndroidPrecompileSettings_Old(
            skgpu::graphite::PrecompileContext* precompileContext,
            RuntimeEffectManager& effectManager,
            const std::function<void(skgpu::graphite::PrecompileContext*,
                                     const PrecompileSettings&,
                                     int index)>& func) {
    // Easy references to SkRuntimeEffects for various LinearEffects that may be reused in multiple
    // precompilation scenarios.
    // clang-format off
    const auto kUNKNOWN__SRGB__false__UNKNOWN__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    /* inputDataspace= */ ui::Dataspace::UNKNOWN, // Default
                    /* outputDataspace= */  ui::Dataspace::SRGB,   // (deprecated) sRGB sRGB Full range
                    /* undoPremultipliedAlpha= */ false,
                    /* fakeOutputDataspace= */ ui::Dataspace::UNKNOWN, // Default
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

    const auto kBT2020_ITU_PQ__DISPLAY_P3__false__0x90a0000__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    /* inputDataspace= */ ui::Dataspace::BT2020_ITU_PQ, // BT2020 SMPTE 2084 Limited range
                    /* outputDataspace= */ ui::Dataspace::DISPLAY_P3, // DCI-P3 sRGB Full range
                    /* undoPremultipliedAlpha= */ false,
                    /* fakeOutputDataspace= */ static_cast<ui::Dataspace>(0x90a0000), // DCI-P3 gamma 2.2 Full range
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

    const auto kBT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    /* inputDataspace= */ ui::Dataspace::BT2020_ITU_PQ, // BT2020 SMPTE 2084 Limited range
                    /* outputDataspace= */ ui::Dataspace::BT2020, // BT2020 SMPTE_170M Full range
                    /* undoPremultipliedAlpha= */ false,
                    /* fakeOutputDataspace= */ ui::Dataspace::UNKNOWN, // Default
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

    const auto k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    /* inputDataspace= */ static_cast<ui::Dataspace>(0x188a0000), // DCI-P3 sRGB Extended range
                    /* outputDataspace= */ ui::Dataspace::DISPLAY_P3, // DCI-P3 sRGB Full range
                    /* undoPremultipliedAlpha= */ false,
                    /* fakeOutputDataspace= */ static_cast<ui::Dataspace>(0x90a0000), // DCI-P3 gamma 2.2 Full range
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

    const auto kBT2020_HLG__UNKNOWN__false__UNKNOWN__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    /* inputDataspace= */ ui::Dataspace::BT2020_HLG,
                    /* outputDataspace= */ ui::Dataspace::UNKNOWN,
                    /* undoPremultipliedAlpha= */ false,
                    /* fakeOutputDataspace= */ ui::Dataspace::UNKNOWN, // Default
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

    const auto kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    /* inputDataspace= */ ui::Dataspace::V0_SRGB,
                    /* outputDataspace= */ ui::Dataspace::V0_SRGB,
                    /* undoPremultipliedAlpha= */ true,
                    /* fakeOutputDataspace= */ ui::Dataspace::UNKNOWN, // Default
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

    const auto k0x188a0000__V0_SRGB__true__0x9010000__Shader =
            effectManager.getOrCreateLinearRuntimeEffect({
                    /* inputDataspace= */ static_cast<ui::Dataspace>(0x188a0000), // DCI-P3 sRGB Extended range
                    /* outputDataspace= */ ui::Dataspace::V0_SRGB,
                    /* undoPremultipliedAlpha= */ true,
                    /* fakeOutputDataspace= */ static_cast<ui::Dataspace>(0x9010000),
                    /* type= */ shaders::LinearEffect::SkSLType::Shader,
            });

#if defined(SK_VULKAN) && defined(SK_BUILD_FOR_ANDROID)
    const SkColorInfo kRGBA8PremulPQ(kRGBA_8888_SkColorType,
                                     kPremul_SkAlphaType,
                                     SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ,
                                                           SkNamedGamut::kRec2020));
#endif

    // clang-format on

    // =======================================
    //            Combinations
    // =======================================
    // NOTE: keep in sync with upstream
    // external/skia/tests/graphite/precompile/AndroidPaintOptions.cpp
    // clang-format off

    // For non-Vulkan configs, these settings cover 126 of the 170 cases in 'kCases'.
    // They create 153 Pipelines so only modestly over-generate (27 extra Pipelines - 18%).
    //
    // For Vulkan configs, the Vulkan-specific PrecompileSettings handle 24 more cases and
    // add 29 more Pipelines.
    //
    // These are sorted into groups based on (first) PaintOptions creation function and
    // then Render Pass Properties.
    // This helps observe DrawTypeFlags distributions.
    const PrecompileSettings precompileCases[] = {
        // 0: 100% (1/1) handles: 0
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D },

        // 1: 100% (4/4) handles: 22 23 42 43
        { Builder().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 2: 100% (2/2) handles: 165 171
        { Builder().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // 3: 100% (1/1) handles: 1
        { Builder().hwImg(kSRGB).srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 4: 100% (4/4) handles: 24 44 45 110
        { Builder().hwImg(kSRGB).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 5: 100% (4/4) handles: 9 28 95 106
        { Builder().transparent().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 6: 100% (2/2) handles: 164 166
        { Builder().transparent().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_4_DS },

        // 7: 75% (3/4) handles: 10 29 208
        { Builder().transparent().hwImg(kSRGB).srcOver(),
          kRRectAndNonAARect,
          kCombo_RGBA_1D_4DS_SRGB },

        // 8: 75% (9/12) handles: 56 57 58 89 92 94 172 250 251
        { Builder().src().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS_16F,
          kWithAnalyticClip },

        // 9: 100% (3/4) handles: 27 86 326
        { Builder().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS,
          kWithAnalyticClip },

        // 10: 75% (3/4) handles: 19 38 128
        { Builder().hwImg(kPremul).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 11: 75% (3/4) handles: 12 123 124
        { Builder().transparent().hwImg(kPremul).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 12: 100% (1/1) handles: 14
        { Builder().transparent().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D },

        // 13: 100% (1/1) handles: 168
        { Builder().transparent().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // 14: 75% (3/4) handles: 16 32 33
        { Builder().hwImg(kPremul).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 15: 100% (1/1) handles: 169
        { Builder().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // 16: 100% (2/2) handles: 15 31
        { Builder().transparent().hwImg(kSRGB).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB },

        // 17: 75% (4/4) handles: 17 34 35 325
        { Builder().hwImg(kSRGB).matrixCF().dither().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 18: 100% (1/1) handles: 77
        { Builder().hwImg(kSRGB).matrixCF().dither().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_4_DS_SRGB },

        // 19: 50% (1/2) handles: 37
        { Builder().hwImg(kAlphaSRGB).matrixCF().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS_SRGB },

        // 20: 100% (1/1) handles: 100
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D },

        // 21: 100% (1/1) handles: 59
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kPerEdgeAAQuad,
          kRGBA_1_D },

        // 22: 100% (1/1) handles: 170
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },

        // TODO(b/426601394): Group these paint option settings into a function that accepts an
        // input image color space so that the intermediate linear color spaces adapt correctly.
        // 23: 100% (1/1) handles: 5
        { MouriMapBlur(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 24: 100% (1/1) handles: 55
        { MouriMapToneMap(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB },

        // 25: 100% (1/1) handles: 7
        { MouriMapCrosstalkAndChunk16x16Passthrough(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 26: 100% (1/1) handles: 6
        { MouriMapChunk8x8Effect(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_Linear },

        // 27: 100% (4/4) handles: 49 99 199 305
        { BlurFilterMix(effectManager),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // These two are solid colors drawn w/ a LinearEffect

        // 28: 67% (8/12) handles: 78 80 81 83 84 157 159 161
        { Builder().srcOver(),
          DrawTypeFlags::kNonSimpleShape,
          kRGBA_4_DS },

        // Note: this didn't get folded into #2 since the RRect draw isn't appearing w/ a clip
        // 29: 100% (1/1) handles: 91
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },

        // 30: 50% (1/2) handles: 60
        { {}, // ignored
          DrawTypeFlags::kDropShadows,
          kRGBA_1_D },

        // 31: 100% (2/2) handles: 82 85
        { {}, // ignored
          DrawTypeFlags::kDropShadows,
          kRGBA_4_DS },

        // 32: 100% (2/2) handles: 96 112
        { EdgeExtensionPremulSrcover(effectManager),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // 33: 100% (1/1) handles: 126
        { TransparentPaintEdgeExtensionPassthroughMatrixCFDitherSrcover(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 34: 100% (1/1) handles: 97
        { TransparentPaintEdgeExtensionPassthroughSrcover(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 35: 100% (2/2) handles: 98 211
        { TransparentPaintEdgeExtensionPremulSrcover(effectManager),
          kRRectAndNonAARect,
          kRGBA_1_D },

        // 36: 100% (2/2) handles: 137 13
        { EdgeExtensionPassthroughSrcover(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D,
          kWithAnalyticClip },

        // 37: 50% (1/2) handles: 101
        { Builder().hwImg(kAlpha, kClamp).src(),
          DrawTypeFlags::kNonAAFillRect,
          kR_1_D },

        // 38: 100% (2/2) handles: 109 129
        { Builder().hwImg(kSRGB).matrixCF().srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB },

        // 39: 100% (1/1) handles: 104
        { MouriMapCrosstalkAndChunk16x16Premul(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 40: 100% (2/2) handles: 107 146
        { Builder().blend().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS },

        // 41: 100% (1/1) handles: 122
        { Builder().blend().srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D },

        // 42: 100% (1/1) handles: 117
        { Builder().transparent().blend().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 43: 100% (1/1) handles: 197
        { BoxShadow(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D  },

        // 44: 100% (1/1) handles: 198
        { GainMap(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_Linear  },

        // 45: 100% (1/1) handles: 202
        // Just 52 but for RRects w/o clips
        { EdgeExtensionPassthroughSrcover(effectManager),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D, },

        // 46: 100% (2/2) handles: 30 204
        // Just 12 for rects with and w/o clips
        { Builder().transparent().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D,
          kWithAnalyticClip},

        // 47: 100% (2/2) handles: 41 212
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D,
          kWithAnalyticClip },

        //------------------------------------------------------------------------------------------

        // 48: 100% (1/1) handles: 201
        linear_solid(kBT2020_HLG__UNKNOWN__false__UNKNOWN__Shader),

        //------------------------------------------------------------------------------------------
        // 49: 100% (1/1) handles: 265
        linear_solid(kBT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader),

        // 50: 100% (1/1) handles: 54
        { LinearEffect(kBT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader,
                       PrecompileShaders::Color(),
                       SkBlendMode::kSrc),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB },

        //------------------------------------------------------------------------------------------
        // k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader

        // 51: 75% (3/4) handles: 180 227 229
        linear_hw_image(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader, /* rrectStyle= */ false),

        // 52: 100% (2/2) handles: 228 230
        linear_hw_image(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader, /* rrectStyle= */ true),

        // 53: 75% (3/4) handles: 176 205 234
        linear_hw_image(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                        /* rrectStyle= */ false, /* opaquePaint= */ false),

        // 54: 100% (2/2) handles: 231 233
        linear_hw_image(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                        /* rrectStyle= */ true, /* opaquePaint= */ false),

        // 55: 100% (1/1) handles: 174
        linear_hw_image_matrixfilter(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                                     /* rrectStyle= */true),

        // 56: 100% (2/2) handles: 177 255
        linear_hw_image_matrixfilter(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                                     /* rrectStyle= */false),

        // 57: 100% (1/1) handles: 173
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false,
                       /* matrixColorFilter= */ true,
                       /* dither= */ false,
                       SkColorSpace::MakeSRGBLinear()),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 58: 50% (2/4) handles: 179 246
        linear_hw_image_matrixfilter_dither(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                                            /* rrectStyle= */ false),
        // 59: 50% (1/2) handles: 175
        linear_hw_image_matrixfilter_dither(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                                            /* rrectStyle= */ true),

        // 60: 75% (3/4) handles: 196 209 275
        linear_hw_image_srgb(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                             /* rrectStyle= */ false),
        // 61: 50% (1/2) handles: 187
        linear_hw_image_srgb(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                             /* rrectStyle= */ true),

        // 62: 75% (3/4) handles: 189 259 260
        linear_hw_image_srgb(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                             /* rrectStyle= */ false, /* opaquePaint= */ false),
        // 63: 100% (2/2) handles: 184 261
        linear_hw_image_srgb(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                             /* rrectStyle= */ true, /* opaquePaint= */ false),

        // 64: 100% (1/1) handles: 266
        linear_solid(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader),

        // 65: 100% (2/2) handles: 194 195
        linear_hw_image_matrixfilter_srgb(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader, false),
        // 66: 100% (1/1) handles: 186
        linear_hw_image_matrixfilter_srgb(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader, true),

        // 67: 100% (1/1) handles: 207
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
               create_hw_image_precompile_shader(),
               SkBlendMode::kSrcOver,
               /* paintColorIsOpaque= */ false,
               /* matrixColorFilter= */ true,
               /* dither= */ true,
               SkColorSpace::MakeSRGBLinear()),
        DrawTypeFlags::kAnalyticRRect,
        kRGBA_1_D  },

        // 68: 50% (1/2) handles: 252
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
               create_hw_image_precompile_shader(),
               SkBlendMode::kSrcOver,
               /* paintColorIsOpaque= */ false,
               /* matrixColorFilter= */ true,
               /* dither= */ true,
               SkColorSpace::MakeSRGBLinear()),
        DrawTypeFlags::kNonAAFillRect,
        kRGBA_1_D,
        kWithAnalyticClip },

        // 69: 100% (1/1) handles: 190
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false,
                       /* matrixColorFilter= */ true,
                       /* dither= */ false,
                       SkColorSpace::MakeSRGB()), // note: not MakeSRGBLinear
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D  },

        // 70: 100% (1/1) handles: 191
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true,
                       SkColorSpace::MakeSRGB()), // note: not MakeSRGBLinear
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D  },

        // 71: 75% (3/4) handles: 185 192 193
        // This is a modified version of 67:
        //    opaque, matrix color filter, dither and analytic clip
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true,
                       SkColorSpace::MakeSRGB()), // note: not MakeSRGBLinear
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip  },

        // 72: 100% (1/1) handles: 120
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D_SRGB },

        // 73: 100% (1/1) handles: 131
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_1_D_SRGB },

        // The next 3 have a RE_LinearEffect and a MatrixFilter along w/ different ancillary
        // additions
        // 74: 100% (1/1) handles: 20
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true,
                       /* dither= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 75: 100% (1/1) handles: 13
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false,
                       /* matrixColorFilter= */ true,
                       /* dither= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 76: 100% (1/1) handles: 18
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true),   // note: null colorspace
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 77: 50% (1/2) handles: 26
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS_SRGB },

        // 78: 100% (2/2) handles: 139 140
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 79: 50% (1/2) handles: 11
        { LinearEffect(k0x188a0000__DISPLAY_P3__false__0x90a0000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS_SRGB },

        //------------------------------------------------------------------------------------------
        // k0x188a0000__V0_SRGB__true__0x9010000__Shader

        // 80: 75% (3/4) handles: 222 223 226
        linear_hw_image(k0x188a0000__V0_SRGB__true__0x9010000__Shader, /* rrectStyle= */ false),
        // 81: 50% (1/2) handles: 224
        linear_hw_image(k0x188a0000__V0_SRGB__true__0x9010000__Shader, /* rrectStyle= */ true),

        // 82: 75% (3/4) handles: 235 236 238
        linear_hw_image(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                        /* rrectStyle= */ false, /* opaquePaint= */ false),
        // 83: 100% (2/2) handles: 237 239
        linear_hw_image(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                        /* rrectStyle= */ true, /* opaquePaint= */ false),

        // 84: 100% (1/1) handles: 210
        linear_hw_image_matrixfilter(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                                     /* rrectStyle= */ true),

        // 85: 50% (1/2) handles: 258
        linear_hw_image_matrixfilter(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                                     /* rrectStyle= */ false),

// No matching labels
//        linear_hw_image_matrixfilter_dither(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
//                                            /* rrectStyle= */ false),
        // 86: 50% (1/2) handles: 248
        linear_hw_image_matrixfilter_dither(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                                            /* rrectStyle= */ true),

// No matching labels
//        linear_hw_image_srgb(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
//                             /* rrectStyle= */ false),
        // 87: 50% (1/2) handles: 213
        linear_hw_image_srgb(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                             /* rrectStyle= */ true),

        // 88: 50% (2/4) handles: 262 263
        linear_hw_image_srgb(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                             /* rrectStyle= */ false, /* opaquePaint= */ false),
        // 89: 50% (1/2) handles: 264
        linear_hw_image_srgb(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                             /* rrectStyle= */ true, /* opaquePaint= */ false),

        // 90: 100% (1/1) handles: 270(fake)
        linear_solid(k0x188a0000__V0_SRGB__true__0x9010000__Shader),

        // 91: 50% (1/2) handles: 272(fake)
        linear_hw_image_matrixfilter_srgb(k0x188a0000__V0_SRGB__true__0x9010000__Shader, false),
        // 92: 100% (1/1) handles: 273
        linear_hw_image_matrixfilter_srgb(k0x188a0000__V0_SRGB__true__0x9010000__Shader, true),

        // 93: 100% (1/1) handles: 108
        { LinearEffect(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true),  // note: null colorspace
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 94: 100% (1/1) handles: 113
        { LinearEffect(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        //------------------------------------------------------------------------------------------
        // kUNKNOWN__SRGB__false__UNKNOWN__Shader

        // 95: 50% (2/4) handles: 214 215
        linear_hw_image(kUNKNOWN__SRGB__false__UNKNOWN__Shader, /* rrectStyle= */ false),
        // 96: 50% (1/2) handles: 216
        linear_hw_image(kUNKNOWN__SRGB__false__UNKNOWN__Shader, /* rrectStyle= */ true),
        // 66 is in this set for 16F/182

        // 97: 50% (2/4) handles: 240 241
        linear_hw_image(kUNKNOWN__SRGB__false__UNKNOWN__Shader,
                        /* rrectStyle= */ false, /* opaquePaint= */ false),
        // 98: 50% (1/2) handles: 242
        linear_hw_image(kUNKNOWN__SRGB__false__UNKNOWN__Shader,
                        /* rrectStyle= */ true, /* opaquePaint= */ false),

        // 99: 100% (1/1) handles: 182
        // A linear_hw_image but 16F
        { LinearEffect(kUNKNOWN__SRGB__false__UNKNOWN__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false,
                       SkColorSpace::MakeSRGBLinear()),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 100: 75% (3/4) handles: 178 181 188
        // This is just 24 wrapped in a very specific LinearEffect
        linear_and_tonemap(&effectManager, kUNKNOWN__SRGB__false__UNKNOWN__Shader),

        // 101: 100% (1/1) handles: 203
        { LinearEffect(kUNKNOWN__SRGB__false__UNKNOWN__Shader,
                       create_tone_map(effectManager),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false,
                       SkColorSpace::MakeSRGBLinear()),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },

        // 102: 100% (2/2) handles: 2 141
        { LinearEffect(kUNKNOWN__SRGB__false__UNKNOWN__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_SRGB_w16F },

        // 103: 100% (1/1) handles: 183
        { LinearEffect(kUNKNOWN__SRGB__false__UNKNOWN__Shader,
                       PrecompileShaders::Color(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ false,
                       SkColorSpace::MakeSRGBLinear()),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },                                // note: 16F

        // 104: 100% (1/1) handles: 4
        { LinearEffect(kUNKNOWN__SRGB__false__UNKNOWN__Shader,
                       PrecompileShaders::Color(),
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        //------------------------------------------------------------------------------------------
        // kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader

        // 105: 75% (3/4) handles: 217 218 220
        linear_hw_image(kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader, /* rrectStyle= */ false),
        // 106: 50% (1/2) handles: 219
        linear_hw_image(kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader, /* rrectStyle= */ true),

        // 107: 50% (2/4) handles: 243 244(fake)
        linear_hw_image(kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader,
                        /* rrectStyle= */ false, /* opaquePaint= */ false),
        // 108: 50% (1/2) handles: 245
        linear_hw_image(kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader,
                        /* rrectStyle= */ true, /* opaquePaint= */ false),

        // 109: 75% (3/4) handles: 206 249 310
        linear_and_tonemap(&effectManager, kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader),

        // 110: 100% (1/1) handles: 254
        { LinearEffect(k0x188a0000__V0_SRGB__true__0x9010000__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ false,
                       /* matrixColorFilter= */ true,
                       /* dither= */ true,
                       SkColorSpace::MakeSRGBLinear()),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        // 111: 100% (1/1) handles: 103
        { LinearEffect(kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // 112: 100% (1/1) handles: 114
        { LinearEffect(kV0_SRGB__V0_SRGB__true__UNKNOWN__Shader,
                       create_hw_image_precompile_shader(),
                       SkBlendMode::kSrcOver),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_1_D_SRGB },

        //------------------------------------------------------------------------------------------
        // kBT2020_ITU_PQ__DISPLAY_P3__false__0x90a0000__Shader

        // 113: 100% (1/1) handles: 268
        linear_solid(kBT2020_ITU_PQ__DISPLAY_P3__false__0x90a0000__Shader),

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
        { MouriMapCrosstalkAndChunk16x16YCbCr247(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA16F_1_D_SRGB },

        // The next 2 have the same PaintOptions but different destination surfaces

        // 75% (3/4) handles 21 39 40
        { LinearEffect(kBT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader,
                       vulkan_ycbcr_image_shader(
                           ycbcr_info(247,
                                      VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
                                      VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                      VK_CHROMA_LOCATION_COSITED_EVEN),
                          kRGBA8PremulPQ),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ true),
          kRRectAndNonAARect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },

        // 100% (1/1) handles 79
        { LinearEffect(kBT2020_ITU_PQ__BT2020__false__UNKNOWN__Shader,
                       vulkan_ycbcr_image_shader(
                           ycbcr_info(247,
                                      VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020,
                                      VK_SAMPLER_YCBCR_RANGE_ITU_NARROW,
                                      VK_CHROMA_LOCATION_COSITED_EVEN),
                           kRGBA8PremulPQ),
                       SkBlendMode::kSrcOver,
                       /* paintColorIsOpaque= */ true,
                       /* matrixColorFilter= */ false,
                       /* dither= */ true),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS_SRGB },

        //----------------
        // 100% (1/1) handles: 154
        { LinearAndLUTEffectImageYCbCr54(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },
        // 100% (1/1) handles: 155
        { ImagePremulYCbCr769Srcover(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },
        // 100% (1/1) handles: 156
        { LinearEffectImageYCbCr54(effectManager),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB },
#endif
    };

    for (size_t i = 0; i < std::size(precompileCases); ++i) {
        func(precompileContext, precompileCases[i], i);
    }
}

/*
 * This set of PaintOptions was generated by finding all the labels that were already
 * covered by the old set of PaintOptions and then copying over the generating
 * PaintOptions. Since these PaintOptions were already in the old set it is assumed that the
 * duplicate labels were from the Protected Context.
 *
 * On a Pixel9 this addresses 28 Pipelines while generating 36 (78%)
 */
void VisitAndroidPrecompileSettings_Protected(
         skgpu::graphite::PrecompileContext* precompileContext,
         RuntimeEffectManager& effectManager,
         const std::function<void(skgpu::graphite::PrecompileContext*,
                                  const PrecompileSettings&,
                                  int index)>& func) {
    // The number in parentheses after the label indices following the "handles" keyword is the
    // index of the label in the old set.
    const PrecompileSettings precompileCases[] = {
        // 0: 100% (1/1) handles: 0 (63*) and 105 (63)
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_4_DS },
        // 1: 100% (1/1) handles: 1 (61*)
        { Builder().transparent().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_4_DS },
        // 2: 100% (1/1) handles: 4 (71*) and 49 (71)
        { Builder().hwImg(kPremul).src(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },
        // 3: 100% (1/1) handles: 5 (86)
        { Builder().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_4_DS },
        // 4: 50% (1/2) handles: 10 (82) -- only need the VerticesRenderStep[TrisColor] version!
        { {}, // ignored
          DrawTypeFlags::kDropShadows,
          kRGBA_4_DS },
        // 5: 100% (1/1) handles: 15 (91)
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },
        // 6: 100% (1/1) handles: 28 (146)
        { Builder().blend().srcOver(),
          DrawTypeFlags::kAnalyticRRect,
          kRGBA_4_DS },
        // 7: 50% (1/2) handles: 35 (89)
        { Builder().srcOver(),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },
        // 8: 100% (1/1) handles: 46 (166 - 69*)
        { Builder().hwImg(kPremul).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },
        // 9: 75% (3/4) handles: 74 (28) 144 (9) 246 (95)
        { Builder().transparent().hwImg(kPremul).srcOver(),
          kRRectAndNonAARect,
          kRGBA_1_D,
          kWithAnalyticClip },
        // 10: 100% (2/2) handles: 85 (99) 192 (49)
        { BlurFilterMix(effectManager),
          kRRectAndNonAARect,
          kRGBA_1_D },
        // 11: 100% (2/2) handles: 97 (43) 114 (42)
        { Builder().hwImg(kPremul).srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D,
          kWithAnalyticClip },
        // 12: 50% (1/2) handles: 165 (57)
        { Builder().src().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },
        // 13: 50% (1/2) handles: 204 (35)
        { Builder().hwImg(kSRGB).matrixCF().dither().srcOver(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D_SRGB,
          kWithAnalyticClip },
        // 14: 50% (1/2) handles: 228 (60)
        { {}, // ignored
          DrawTypeFlags::kDropShadows,
          kRGBA_1_D },

#if defined(SK_VULKAN) && defined(SK_BUILD_FOR_ANDROID)
        // 15: 100% (1/1) handles: 2 (46)
        { ImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },
        // 16: 100% (1/1) handles: 7 (67*)
        { TransparentPaintImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },
        // 17: 50% (1/2) handles: 12 (148)
        { TransparentPaintImagePremulYCbCr238Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kCombo_RGBA_1D_4DS },
        // 18: 100% (1/1) handles: 22 (87)
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },
        // 19: 100% (1/1) handles: 34 (151)
        { ImagePremulYCbCr240Srcover(),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS },
        // 20: 100% (1/1) handles: 130 (90)
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          DrawTypeFlags::kNonAAFillRect | DrawTypeFlags::kAnalyticClip,
          kRGBA_4_DS },
        // 21: 50% (1/2) handles: 171 (93)
        { TransparentPaintImagePremulYCbCr238Srcover(),
          DrawTypeFlags::kAnalyticRRect,
          kCombo_RGBA_1D_4DS },
        // 22: 100% (1/1) handles: 179 (150)
        { ImagePremulYCbCr238Srcover(/* narrow= */ false),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_4_DS_SRGB },
        // 23: 100% (1/1) handles: 217 (47)
        { ImagePremulYCbCr238Srcover(/* narrow= */ true),
          DrawTypeFlags::kNonAAFillRect,
          kRGBA_1_D },
#endif
    };

    for (size_t i = 0; i < std::size(precompileCases); ++i) {
        func(precompileContext, precompileCases[i], i);
    }
}

#endif // SK_GRAPHITE
