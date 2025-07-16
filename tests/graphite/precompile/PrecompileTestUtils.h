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

#include <map>

// Print out a final report that includes missed cases in 'kCases'
//#define FINAL_REPORT

// Print out the cases (in 'kCases') that are covered by each 'kPrecompileCases' case
// Also lists the utilization of each 'kPrecompileCases' case
//#define PRINT_COVERAGE

// Print out all the generated labels and whether they were found in 'kCases'.
// This is usually used along with the 'kChosenCase' variable.
//#define PRINT_GENERATED_LABELS

namespace skiatest {
    class Reporter;
}

namespace PrecompileTestUtils {

struct PrecompileSettings {
    PrecompileSettings(const skgpu::graphite::PaintOptions& paintOptions,
                       skgpu::graphite::DrawTypeFlags drawTypeFlags,
                       const skgpu::graphite::RenderPassProperties& renderPassProps,
                       bool analyticClipping = false)
           : fPaintOptions(paintOptions)
           , fDrawTypeFlags(drawTypeFlags)
           , fRenderPassProps({ &renderPassProps, 1 })
           , fAnalyticClipping(analyticClipping) {}

    PrecompileSettings(const skgpu::graphite::PaintOptions& paintOptions,
                       skgpu::graphite::DrawTypeFlags drawTypeFlags,
                       SkSpan<const skgpu::graphite::RenderPassProperties> renderPassProps,
                       bool analyticClipping = false)
            : fPaintOptions(paintOptions)
            , fDrawTypeFlags(drawTypeFlags)
            , fRenderPassProps(renderPassProps)
            , fAnalyticClipping(analyticClipping) {}

    skgpu::graphite::PaintOptions fPaintOptions;
    skgpu::graphite::DrawTypeFlags fDrawTypeFlags = skgpu::graphite::DrawTypeFlags::kNone;
    SkSpan<const skgpu::graphite::RenderPassProperties> fRenderPassProps;
    bool fAnalyticClipping = false;
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
             const PrecompileSettings& precompileSettings,
             int precompileSettingsIndex,
             SkSpan<const PipelineLabel> cases,
             PipelineLabelInfoCollector* collector);

skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16Passthrough();
skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16Premul();
skgpu::graphite::PaintOptions MouriMapChunk8x8Effect();
skgpu::graphite::PaintOptions MouriMapBlur();
skgpu::graphite::PaintOptions MouriMapToneMap();
skgpu::graphite::PaintOptions KawaseBlurLowSrcSrcOver();
skgpu::graphite::PaintOptions KawaseBlurHighSrc();
skgpu::graphite::PaintOptions BlurFilterMix();

skgpu::graphite::PaintOptions EdgeExtensionPassthroughSrcover();
skgpu::graphite::PaintOptions EdgeExtensionPremulSrcover();
skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPassthroughMatrixCFDitherSrcover();
skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPassthroughSrcover();
skgpu::graphite::PaintOptions TransparentPaintEdgeExtensionPremulSrcover();

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
skgpu::graphite::PaintOptions ImagePremulYCbCr238Srcover(bool narrow);
skgpu::graphite::PaintOptions TransparentPaintImagePremulYCbCr238Srcover();
skgpu::graphite::PaintOptions ImagePremulYCbCr240Srcover();
skgpu::graphite::PaintOptions TransparentPaintImagePremulYCbCr240Srcover();
skgpu::graphite::PaintOptions MouriMapCrosstalkAndChunk16x16YCbCr247();

// Prints out the VulkanYcbcrConversionInfo retrieved from a Vulkan YCbCr Pipeline label
// (e.g., base64 part of HardwareImage(3: kEwAAPcAAAAAAAAA)).
void Base642YCbCr(const char*);
#endif // SK_VULKAN

} // namespace PrecompileTestUtils

#endif // PrecompileTestUtils_DEFINED
