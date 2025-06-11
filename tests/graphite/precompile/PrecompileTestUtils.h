/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PrecompileTestUtils_DEFINED
#define PrecompileTestUtils_DEFINED

#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/gpu/graphite/precompile/Precompile.h"

// Print out a final report that includes missed cases in 'kCases'
//#define FINAL_REPORT

// Print out the cases (in 'kCases') that are covered by each 'kPrecompileCases' case
// Also lists the utilization of each 'kPrecompileCases' case
//#define PRINT_COVERAGE

// Print out all the generated labels and whether they were found in 'kCases'.
// This is usually used along with the 'kChosenCase' variable.
//#define PRINT_GENERATED_LABELS

namespace PrecompileTestUtils {

struct PrecompileSettings {
    skgpu::graphite::PaintOptions fPaintOptions;
    skgpu::graphite::DrawTypeFlags fDrawTypeFlags = skgpu::graphite::DrawTypeFlags::kNone;
    skgpu::graphite::RenderPassProperties fRenderPassProps;

    bool isSubsetOf(const PrecompileSettings& superSet) const;
};

struct PipelineLabel {
    int fNumHits;         // the number of uses in 9 of the 14 most visited web sites
    const char* fString;
};

class PipelineLabelInfoCollector {
public:
    typedef bool (*SkipFunc)(const char*);

    explicit PipelineLabelInfoCollector(SkSpan<const PipelineLabel> cases, SkipFunc);

    int processLabel(const std::string& precompiledLabel, int precompileCase);

    void finalReport();

private:
    // PipelineLabelInfo captures the information for a single Pipeline label. It stores which
    // entry in 'kCases' it represents and which entry in 'kPrecompileCases' fulfilled it.
    class PipelineLabelInfo {
    public:
        PipelineLabelInfo(int casesIndex, int val = kUninit)
                : fCasesIndex(casesIndex)
                , fPrecompileCase(val) {}

        // Index of this Pipeline label in 'kCases'.
        const int fCasesIndex;

        static constexpr int kSkipped = -2;
        static constexpr int kUninit  = -1;
        // >= 0 -> covered by the 'fPrecompileCase' case in 'kPrecompileCases'
        int fPrecompileCase = kUninit;
    };

    struct comparator {
        bool operator()(const char* a, const char* b) const {
            return strcmp(a, b) < 0;
        }
    };

    int fNumLabelsProcessed = 0;
    std::map<const char*, PipelineLabelInfo, comparator> fMap;

    struct OverGenInfo {
        OverGenInfo(int originatingSetting) : fOriginatingSetting(originatingSetting) {}

        int fOriginatingSetting;
    };

    std::map<std::string, OverGenInfo> fOverGenerated;
};

void RunTest(skgpu::graphite::PrecompileContext* precompileContext,
             skiatest::Reporter* reporter,
             SkSpan<const PrecompileSettings> precompileSettings,
             int precompileSettingsIndex,
             SkSpan<const PipelineLabel> cases,
             PipelineLabelInfoCollector* collector);

skgpu::graphite::PaintOptions SolidSrcover();
skgpu::graphite::PaintOptions SolidMatrixCFSrcover();
skgpu::graphite::PaintOptions LinearGradSmSrcover();
skgpu::graphite::PaintOptions LinearGradSRGBSmMedDitherSrcover();
skgpu::graphite::PaintOptions TransparentPaintImagePremulHWAndClampSrcover();
skgpu::graphite::PaintOptions TransparentPaintImagePremulHWOnlyMatrixCFSrcover();
skgpu::graphite::PaintOptions TransparentPaintImagePremulHWOnlyMatrixCFDitherSrcover();
skgpu::graphite::PaintOptions TransparentPaintImageSRGBHWOnlyMatrixCFDitherSrcover();
skgpu::graphite::PaintOptions TransparentPaintImagePremulHWOnlySrcover();
skgpu::graphite::PaintOptions TransparentPaintImageSRGBHWOnlySrcover();
skgpu::graphite::PaintOptions TransparentPaintSrcover();
skgpu::graphite::PaintOptions SolidClearSrcSrcover();
skgpu::graphite::PaintOptions SolidSrcSrcover();
skgpu::graphite::PaintOptions ImagePremulNoCubicSrcover();
skgpu::graphite::PaintOptions ImagePremulHWOnlySrc();
skgpu::graphite::PaintOptions ImagePremulHWOnlySrcover();
skgpu::graphite::PaintOptions ImagePremulClampNoCubicDstin();
skgpu::graphite::PaintOptions ImagePremulHWOnlyDstin();
skgpu::graphite::PaintOptions YUVImageSRGBNoCubicSrcover();
skgpu::graphite::PaintOptions YUVImageSRGBSrcover2();
skgpu::graphite::PaintOptions ImagePremulNoCubicSrcSrcover();
skgpu::graphite::PaintOptions ImageSRGBNoCubicSrc();
skgpu::graphite::PaintOptions ImageAlphaHWOnlySrcover();
skgpu::graphite::PaintOptions ImageAlphaPremulHWOnlyMatrixCFSrcover();
skgpu::graphite::PaintOptions ImageAlphaSRGBHWOnlyMatrixCFSrcover();
skgpu::graphite::PaintOptions ImageAlphaNoCubicSrc();
skgpu::graphite::PaintOptions ImagePremulHWOnlyPorterDuffCFSrcover();
skgpu::graphite::PaintOptions ImagePremulHWOnlyMatrixCFSrcover();
skgpu::graphite::PaintOptions ImagePremulHWOnlyMatrixCFDitherSrcover();
skgpu::graphite::PaintOptions ImageSRGBHWOnlyMatrixCFDitherSrcover();
skgpu::graphite::PaintOptions ImageHWOnlySRGBSrcover();

skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16();
skgpu::graphite::PaintOptions MouriMapChunk8x8Effect();
skgpu::graphite::PaintOptions MouriMapBlur();
skgpu::graphite::PaintOptions MouriMapToneMap();
skgpu::graphite::PaintOptions KawaseBlurLowSrcSrcOver();
skgpu::graphite::PaintOptions KawaseBlurHighSrc();
skgpu::graphite::PaintOptions BlurFilterMix();

// Specifies the child shader to be created for a RE_LinearEffect
enum class ChildType {
    kSolidColor,
    kHWTexture,
#if defined(SK_VULKAN)
    kHWTextureYCbCr247,
#endif
};

skgpu::graphite::PaintOptions LinearEffect(const char* parameterStr,
                                           ChildType childType,
                                           SkBlendMode blendMode,
                                           bool paintColorIsOpaque = true,
                                           bool matrixColorFilter = false,
                                           bool dither = false);

#if defined(SK_VULKAN)
skgpu::graphite::PaintOptions ImagePremulYCbCr238Srcover();
skgpu::graphite::PaintOptions TransparentPaintImagePremulYCbCr238Srcover();
skgpu::graphite::PaintOptions ImagePremulYCbCr240Srcover();
skgpu::graphite::PaintOptions TransparentPaintImagePremulYCbCr240Srcover();
skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16YCbCr247();

// Prints out the VulkanYcbcrConversionInfo retrieved from a Vulkan YCbCr Pipeline label
// (e.g., base64 part of HardwareImage(3: kEwAAPcAAAAAAAAA)).
void Base642YCbCr(const char*);
#endif // SK_VULKAN

// Single sampled R w/ just depth
const skgpu::graphite::RenderPassProperties kR_1_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kAlpha_8_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ false
};

// MSAA R w/ depth and stencil
const skgpu::graphite::RenderPassProperties kR_4_DS {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kAlpha_8_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ true
};

// Single sampled BGRA w/ just depth
const skgpu::graphite::RenderPassProperties kBGRA_1_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kBGRA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ false
};

// RGBA version of the above
const skgpu::graphite::RenderPassProperties kRGBA_1_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ false
};

// MSAA BGRA w/ just depth
const skgpu::graphite::RenderPassProperties kBGRA_4_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kBGRA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ true
};

// RGBA version of the above
const skgpu::graphite::RenderPassProperties kRGBA_4_D {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ true
};

// MSAA BGRA w/ depth and stencil
const skgpu::graphite::RenderPassProperties kBGRA_4_DS {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kBGRA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ true
};

// RGBA version of the above
const skgpu::graphite::RenderPassProperties kRGBA_4_DS {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kRGBA_8888_SkColorType,
    /* fDstCS= */ nullptr,
    /* fRequiresMSAA= */ true
};

// The same as kBGRA_1_D but w/ an SRGB colorSpace
const skgpu::graphite::RenderPassProperties kBGRA_1_D_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kBGRA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ false
};

// RGBA version of the above
const skgpu::graphite::RenderPassProperties kRGBA_1_D_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ false
};

// The same as kBGRA_1_D but w/ an Adobe RGB colorSpace
const skgpu::graphite::RenderPassProperties kBGRA_1_D_Adobe {
    skgpu::graphite::DepthStencilFlags::kDepth,
    kBGRA_8888_SkColorType,
    SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                          SkNamedGamut::kAdobeRGB),
    /* fRequiresMSAA= */ false
};

// The same as kBGRA_4_DS but w/ an SRGB colorSpace
const skgpu::graphite::RenderPassProperties kBGRA_4_DS_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kBGRA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ true
};

// RGBA version of the above
const skgpu::graphite::RenderPassProperties kRGBA_4_DS_SRGB {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kRGBA_8888_SkColorType,
    SkColorSpace::MakeSRGB(),
    /* fRequiresMSAA= */ true
};

// The same as kBGRA_4_DS but w/ an Adobe RGB colorSpace
const skgpu::graphite::RenderPassProperties kBGRA_4_DS_Adobe {
    skgpu::graphite::DepthStencilFlags::kDepthStencil,
    kBGRA_8888_SkColorType,
    SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                          SkNamedGamut::kAdobeRGB),
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

constexpr skgpu::graphite::DrawTypeFlags kRRectAndNonAARect =
        static_cast<skgpu::graphite::DrawTypeFlags>(skgpu::graphite::DrawTypeFlags::kAnalyticRRect |
                                                    skgpu::graphite::DrawTypeFlags::kNonAAFillRect);
constexpr skgpu::graphite::DrawTypeFlags kQuadAndNonAARect =
        static_cast<skgpu::graphite::DrawTypeFlags>(skgpu::graphite::DrawTypeFlags::kPerEdgeAAQuad |
                                                    skgpu::graphite::DrawTypeFlags::kNonAAFillRect);

} // namespace PrecompileTestUtils

#endif // PrecompileTestUtils_DEFINED
