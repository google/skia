/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#if defined(SK_GRAPHITE)

#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
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
using PrecompileShaders::GradientShaderFlags;
using PrecompileShaders::ImageShaderFlags;
using PrecompileShaders::YUVImageShaderFlags;

using ::skgpu::graphite::DepthStencilFlags;
using ::skgpu::graphite::DrawTypeFlags;
using ::skgpu::graphite::PaintOptions;
using ::skgpu::graphite::RenderPassProperties;

namespace PrecompileTestUtils {

PaintOptions SolidSrcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions SolidMatrixCFSrcover() {
    PaintOptions paintOptions;

    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    return paintOptions;
}

PaintOptions LinearGradSmSrcover() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient(GradientShaderFlags::kSmall) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions LinearGradSRGBSmMedDitherSrcover() {
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::LinearGradient(
            GradientShaderFlags::kNoLarge,
            { SkGradientShader::Interpolation::InPremul::kNo,
              SkGradientShader::Interpolation::ColorSpace::kSRGB,
              SkGradientShader::Interpolation::HueMethod::kShorter }) });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setDither(true);

    return paintOptions;
}

PaintOptions TransparentPaintImagePremulHWAndClampSrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    SkTileMode tm = SkTileMode::kClamp;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       { &tm, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions TransparentPaintImagePremulHWOnlyMatrixCFSrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions TransparentPaintImagePremulHWOnlyMatrixCFDitherSrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    paintOptions.setDither(true);
    return paintOptions;
}

PaintOptions TransparentPaintImageSRGBHWOnlyMatrixCFDitherSrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;

    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    paintOptions.setDither(true);
    return paintOptions;
}

PaintOptions TransparentPaintImagePremulHWOnlySrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions TransparentPaintImageSRGBHWOnlySrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;

    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions TransparentPaintSrcover() {
    PaintOptions paintOptions;

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
    return paintOptions;
}

PaintOptions SolidClearSrcSrcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kClear,
                                 SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions SolidSrcSrcover() {
    PaintOptions paintOptions;
    paintOptions.setBlendModes({ SkBlendMode::kSrc, SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImagePremulNoCubicSrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    SkTileMode tm = SkTileMode::kClamp;
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       { &tm, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImagePremulHWOnlySrc() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

PaintOptions ImagePremulHWOnlySrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImagePremulClampNoCubicDstin() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    SkTileMode tm = SkTileMode::kClamp;
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       { &tm, 1}) });
    paintOptions.setBlendModes({ SkBlendMode::kDstIn });
    return paintOptions;
}

PaintOptions ImagePremulHWOnlyDstin() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kDstIn });
    return paintOptions;
}

PaintOptions YUVImageSRGBNoCubicSrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::YUVImage(YUVImageShaderFlags::kExcludeCubic,
                                                          { &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions YUVImageSRGBSrcover2() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::YUVImage(
                                                YUVImageShaderFlags::kNoCubicNoNonSwizzledHW,
                                                { &ci, 1 }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImagePremulNoCubicSrcSrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    PaintOptions paintOptions;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc,
                                 SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImageSRGBNoCubicSrc() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                           SkNamedGamut::kAdobeRGB) };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

PaintOptions BlendPorterDuffCFSrcover() {
    PaintOptions paintOptions;
    // kSrcOver will trigger the PorterDuffBlender
    paintOptions.setColorFilters(
            { PrecompileColorFilters::Blend({ SkBlendMode::kSrcOver }) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });

    return paintOptions;
}

PaintOptions ImageAlphaHWOnlySrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kAlpha_8_SkColorType, kUnpremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImageAlphaPremulHWOnlyMatrixCFSrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kAlpha_8_SkColorType, kUnpremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImageAlphaSRGBHWOnlyMatrixCFSrcover() {
    // Note: this is different from the other SRGB ColorInfos
    SkColorInfo ci { kAlpha_8_SkColorType,
                     kUnpremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;

    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImageAlphaNoCubicSrc() {
    PaintOptions paintOptions;

    SkColorInfo ci { kAlpha_8_SkColorType, kUnpremul_SkAlphaType, nullptr };
    SkTileMode tm = SkTileMode::kRepeat;
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       { &tm, 1}) });
    paintOptions.setBlendModes({ SkBlendMode::kSrc });
    return paintOptions;
}

PaintOptions ImagePremulHWOnlyPorterDuffCFSrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters(
                { PrecompileColorFilters::Blend({ SkBlendMode::kSrcOver }) });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImagePremulHWOnlyMatrixCFSrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImagePremulHWOnlyMatrixCFDitherSrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setDither(true);

    return paintOptions;
}

PaintOptions ImageSRGBHWOnlyMatrixCFDitherSrcover() {
    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kAdobeRGB) };

    PaintOptions paintOptions;

    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });
    paintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setDither(true);

    return paintOptions;
}

PaintOptions ImageHWOnlySRGBSrcover() {
    PaintOptions paintOptions;

    SkColorInfo ci { kRGBA_8888_SkColorType,
                     kPremul_SkAlphaType,
                     SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                           SkNamedGamut::kAdobeRGB) };
    paintOptions.setShaders({ PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                       { &ci, 1 },
                                                       {}) });

    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

#if defined(SK_VULKAN)
namespace {
sk_sp<PrecompileShader> vulkan_ycbcr_709_image_shader(uint64_t format,
                                                      VkSamplerYcbcrRange range) {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };

    skgpu::VulkanYcbcrConversionInfo info;

    info.fExternalFormat = format;
    info.fYcbcrModel     = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
    info.fYcbcrRange     = range;
    info.fXChromaOffset  = VK_CHROMA_LOCATION_MIDPOINT;
    info.fYChromaOffset  = VK_CHROMA_LOCATION_MIDPOINT;
    info.fChromaFilter   = VK_FILTER_LINEAR;

    return PrecompileShaders::VulkanYCbCrImage(info,
                                               PrecompileShaders::ImageShaderFlags::kExcludeCubic,
                                               { &ci, 1 },
                                               {});
}

} // anonymous namespace

PaintOptions ImagePremulYCbCr238Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHoAAO4AAAAAAAAA)
    paintOptions.setShaders({ vulkan_ycbcr_709_image_shader(238,
                                                            VK_SAMPLER_YCBCR_RANGE_ITU_NARROW) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions ImagePremulYCbCr240Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHIAAPAAAAAAAAAA)
    paintOptions.setShaders({ vulkan_ycbcr_709_image_shader(240,
                                                            VK_SAMPLER_YCBCR_RANGE_ITU_FULL) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    return paintOptions;
}

PaintOptions TransparentPaintImagePremulYCbCr240Srcover() {
    PaintOptions paintOptions;

    // HardwareImage(3: kHIAAPAAAAAAAAAA)
    paintOptions.setShaders({ vulkan_ycbcr_709_image_shader(240,
                                                            VK_SAMPLER_YCBCR_RANGE_ITU_FULL) });
    paintOptions.setBlendModes({ SkBlendMode::kSrcOver });
    paintOptions.setPaintColorIsOpaque(false);
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

#if defined(SK_DEBUG)
// This helper maps from the RenderPass string in the Pipeline label to the
// RenderPassProperties needed by the Precompile system
// TODO(robertphillips): converting this to a more piecemeal approach might better illuminate
// the mapping between the string and the RenderPassProperties
RenderPassProperties get_render_pass_properties(const char* str) {
    static const struct {
        RenderPassProperties fRenderPassProperties;
        const char* fStr;
    } kRenderPassPropertiesMapping[] = {
        { kR_1_D,     "RP((R8+D16 x1).a000)" },
        { kBGRA_1_D,  "RP((BGRA8+D16 x1).rgba)" },
        { kRGBA_1_D,  "RP((RGBA8+D16 x1).rgba)" },
        // These RPPs can generate two strings when Caps::loadOpAffectsMSAAPipelines.
        { kR_4_DS,    "RP((R8+D24_S8 x4->1).a000)" },
        { kR_4_DS,    "RP((R8+D24_S8 x4->1).a000 w/ msaa load)" },
        { kBGRA_4_D,  "RP((BGRA8+D16 x4->1).rgba)" },
        { kBGRA_4_D,  "RP((BGRA8+D16 x4->1).rgba w/ msaa load)" },

        { kRGBA_4_D,  "RP((RGBA8+D16 x4->1).rgba)" },
        { kRGBA_4_D,  "RP((RGBA8+D16 x4->1).rgba w/ msaa load)" },

        { kBGRA_4_DS, "RP((BGRA8+D24_S8 x4->1).rgba)" },
        { kBGRA_4_DS, "RP((BGRA8+D24_S8 x4->1).rgba w/ msaa load)" },

        { kRGBA_4_DS, "RP((RGBA8+D24_S8 x4->1).rgba)" },
        { kRGBA_4_DS, "RP((RGBA8+D24_S8 x4->1).rgba w/ msaa load)" },

        { kRGBA16F_1_D, "RP((RGBA16F+D16 x1).rgba)" },
    };

    for (const auto& rppm : kRenderPassPropertiesMapping) {
        if (strstr(str, rppm.fStr)) {
            return rppm.fRenderPassProperties;
        }
    }

    SkAssertResult(0);
    return {};
}

// This helper maps from the RenderStep's name in the Pipeline label to the DrawTypeFlag that
// resulted in its use.
DrawTypeFlags get_draw_type_flags(const char* str) {
    static const struct {
        const char* fStr;
        DrawTypeFlags fFlags;
    } kDrawTypeFlagsMapping[] = {
        { "BitmapTextRenderStep[Mask]",                  DrawTypeFlags::kBitmapText_Mask  },
        { "BitmapTextRenderStep[LCD]",                   DrawTypeFlags::kBitmapText_LCD   },
        { "BitmapTextRenderStep[Color]",                 DrawTypeFlags::kBitmapText_Color },

        { "SDFTextRenderStep",                           DrawTypeFlags::kSDFText      },
        { "SDFTextLCDRenderStep",                        DrawTypeFlags::kSDFText_LCD  },

        { "VerticesRenderStep[Tris]",                    DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisTexCoords]",           DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisColor]",               DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TrisColorTexCoords]",      DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[Tristrips]",               DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsTexCoords]",      DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsColor]",          DrawTypeFlags::kDrawVertices },
        { "VerticesRenderStep[TristripsColorTexCoords]", DrawTypeFlags::kDrawVertices },

        { "CircularArcRenderStep",                       DrawTypeFlags::kCircularArc  },

        { "AnalyticRRectRenderStep",                     DrawTypeFlags::kAnalyticRRect  },
        { "CoverBoundsRenderStep[NonAAFill]",            DrawTypeFlags::kNonAAFillRect  },
        { "PerEdgeAAQuadRenderStep",                     DrawTypeFlags::kPerEdgeAAQuad  },

        { "CoverageMaskRenderStep",                      DrawTypeFlags::kNonSimpleShape },
        { "CoverBoundsRenderStep[RegularCover]",         DrawTypeFlags::kNonSimpleShape },
        { "CoverBoundsRenderStep[InverseCover]",         DrawTypeFlags::kNonSimpleShape },
        { "MiddleOutFanRenderStep[EvenOdd]",             DrawTypeFlags::kNonSimpleShape },
        { "MiddleOutFanRenderStep[Winding]",             DrawTypeFlags::kNonSimpleShape },
        { "TessellateCurvesRenderStep[EvenOdd]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateCurvesRenderStep[Winding]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateStrokesRenderStep",                 DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[Convex]",          DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[EvenOdd]",         DrawTypeFlags::kNonSimpleShape },
        { "TessellateWedgesRenderStep[Winding]",         DrawTypeFlags::kNonSimpleShape },
    };

    for (const auto& dtfm : kDrawTypeFlagsMapping) {
        if (strstr(str, dtfm.fStr)) {
            SkAssertResult(dtfm.fFlags != DrawTypeFlags::kNone);
            return dtfm.fFlags;
        }
    }

    SkAssertResult(0);
    return DrawTypeFlags::kNone;
}

void deduce_settings_from_label(const char* testStr, PrecompileSettings* result) {
    result->fDrawTypeFlags = get_draw_type_flags(testStr);
    result->fRenderPassProps = get_render_pass_properties(testStr);
    if (strstr(testStr, "LinearGradient4 ColorSpaceTransformSRGB") ||
        strstr(testStr, "LinearGradient8 ColorSpaceTransformSRGB") ||
        strstr(testStr, "PrimitiveColor ColorSpaceTransformSRGB")) {
        result->fRenderPassProps.fDstCS = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                               SkNamedGamut::kAdobeRGB);
    } else if (strstr(testStr, "ColorSpaceTransformSRGB")) {
        result->fRenderPassProps.fDstCS = SkColorSpace::MakeSRGB();
    }
}

#endif // SK_DEBUG

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

bool PrecompileSettings::isSubsetOf(const PrecompileSettings& superSet) const {
    SkASSERT(SkPopCount(static_cast<uint32_t>(fDrawTypeFlags)) == 1);

    // 'superSet' may have a wider range of DrawTypeFlags
    return (fDrawTypeFlags & superSet.fDrawTypeFlags) &&
           fRenderPassProps == superSet.fRenderPassProps;
}

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
        SkASSERT(fOverGenerated.find(precompiledLabel) == fOverGenerated.end());
        fOverGenerated.insert({ precompiledLabel, OverGenInfo(precompileCase) });
        return -1;
    }

    // We expect each PrecompileSettings case to handle disjoint sets of labels. If this
    // assert fires some pair of PrecompileSettings are handling the same case.
    SkASSERT(result->second.fPrecompileCase == PipelineLabelInfo::kUninit);
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
               settings.fDrawTypeFlags,
               { &settings.fRenderPassProps, 1 });

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

#if defined(SK_DEBUG)
            {
                PrecompileSettings expectedSettings;

                deduce_settings_from_label(cases[matchInCases].fString, &expectedSettings);
                SkASSERT(expectedSettings.isSubsetOf(settings));
            }
#endif
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
    SkDebugf("precompile case %d handles %zu/%zu cases (%.2f utilization): ",
             precompileSettingsIndex, matchesInCases.size(), generatedLabels.size(), utilization);
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
