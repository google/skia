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
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/sksl/SkSLUtil.h"
#include "tests/graphite/precompile/AndroidRuntimeEffectManager.h"
#include "tests/graphite/precompile/PrecompileTestUtils.h"
#include "tools/graphite/UniqueKeyUtils.h"

#if defined (SK_VULKAN)
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

// Used in lieu of SkEnumBitMask
static constexpr DrawTypeFlags operator|(DrawTypeFlags a, DrawTypeFlags b) {
    return static_cast<DrawTypeFlags>(static_cast<std::underlying_type<DrawTypeFlags>::type>(a) |
                                      static_cast<std::underlying_type<DrawTypeFlags>::type>(b));
}

namespace PrecompileTestUtils {

#if defined(SK_VULKAN)

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
             "xOff: %d yOff: %d filter: %d explicit: %u components: %d %d %d %d\n",
             info.format(),
             (unsigned long long) info.externalFormat(),
             info.model(),
             info.range(),
             info.xChromaOffset(),
             info.yChromaOffset(),
             info.chromaFilter(),
             info.forceExplicitReconstruction(),
             info.components().r,
             info.components().g,
             info.components().b,
             info.components().a);
}

#endif // SK_VULKAN

namespace {

// Find any duplicate Pipeline labels
[[maybe_unused]] void find_duplicates(SkSpan<const PipelineLabel> labels) {
    for (size_t i = 0; i < labels.size(); ++i) {
        for (size_t j = i+1; j < labels.size(); ++j) {
            if (!strcmp(labels[j].fString, labels[i].fString)) {
                SkDebugf("%zu is a duplicate of %zu\n", i, j);
            }
        }
    }
}

std::string rm_whitespace(const std::string& s) {
    auto start = s.find_first_not_of(' ');
    auto end = s.find_last_not_of(' ');
    return s.substr(start, (end - start) + 1);
}

bool skip(const char* str) {
#if !defined(SK_VULKAN)
    if (strstr(str, "HardwareImage(3:")) {
        return true;
    }
#endif // SK_VULKAN
    if (strstr(str, "RE_GainmapEffect")) {
        return true;
    }
    return false;
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
              "cases %d and %d cover the same label (%d)",
              result->second.fPrecompileCase, precompileCase, result->second.fCasesIndex);
    result->second.fPrecompileCase = precompileCase;
    return result->second.fCasesIndex;
}

int PipelineLabelInfoCollector::numNotCovered() const {
    int numNotCovered = 0;

    for (const auto& iter : fMap) {
        if (iter.second.fPrecompileCase < 0) {
            ++numNotCovered;
        }
    }

    return numNotCovered;
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
             const PrecompileSettings& settings,
             int precompileSettingsIndex,
             SkSpan<const PipelineLabel> cases,
             PipelineLabelInfoCollector* collector,
             bool checkPaintOptionCoverage) {
    using namespace skgpu::graphite;

    precompileContext->priv().globalCache()->resetGraphicsPipelines();

    Precompile(precompileContext,
               settings.fPaintOptions,
               settings.fDrawTypeFlags,
               settings.fRenderPassProps);

    if (settings.fAnalyticClipping) {
        Precompile(precompileContext,
                   settings.fPaintOptions,
                   settings.fDrawTypeFlags | DrawTypeFlags::kAnalyticClip,
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
            std::string tmp = GetPipelineLabel(precompileContext->priv().caps(),
                                               dict, renderPassDesc, renderStep,
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

    float utilization = ((float) matchesInCases.size())/generatedLabels.size();

    if (checkPaintOptionCoverage) {
        REPORTER_ASSERT(reporter, matchesInCases.size() >= 1,   // This tests requirement 1, above
                        "%d: num matches: %zu", precompileSettingsIndex, matchesInCases.size());
        REPORTER_ASSERT(reporter, utilization >= 0.4f,         // This tests requirement 2, above
                        "%d: utilization: %f", precompileSettingsIndex, utilization);
    }

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

void PrecompileTest(skiatest::Reporter* reporter,
                    skgpu::graphite::Context* context,
                    SkSpan<const PipelineLabel> labels,
                    VisitSettingsFunc visitSettings,
                    bool checkPaintOptionCoverage,
                    bool checkPipelineLabelCoverage) {
    using namespace skgpu::graphite;

//    find_duplicates(labels);

#if defined(SK_VULKAN)
    // Use this call to map back from a HardwareImage sub-string to a VulkanYcbcrConversionInfo
    //Base642YCbCr("kAwAEPcAAAAAAAAA");
#endif

    std::unique_ptr<PrecompileContext> precompileContext = context->makePrecompileContext();
    const skgpu::graphite::Caps* caps = precompileContext->priv().caps();

    TextureInfo textureInfo = caps->getDefaultSampledTextureInfo(kBGRA_8888_SkColorType,
                                                                 skgpu::Mipmapped::kNo,
                                                                 skgpu::Protected::kNo,
                                                                 skgpu::Renderable::kYes);

    const bool msaaSupported = caps->getCompatibleMSAASampleCount(textureInfo) > SampleCount::k1;

    if (!msaaSupported) {
        // The following pipelines rely on having MSAA
        return;
    }

#ifdef SK_ENABLE_VELLO_SHADERS
    if (caps->computeSupport()) {
        // The following pipelines rely on not utilizing Vello
        return;
    }
#endif

    PipelineLabelInfoCollector collector(labels, skip);
    RuntimeEffectManager effectManager;
    bool skipped = false;

    (*visitSettings)(
         precompileContext.get(),
         effectManager,
         [&](skgpu::graphite::PrecompileContext* precompileContext,
             const PrecompileSettings& precompileCase,
             int index) {
            const skgpu::graphite::Caps* caps = precompileContext->priv().caps();

            static const int kChosenCase = -1; // only test this entry in 'kPrecompileCases'
            if (kChosenCase != -1 && kChosenCase != index) {
                skipped = true;
                return;
            }

            if (caps->getDepthStencilFormat(DepthStencilFlags::kDepth) != TextureFormat::kD16) {
                // The Pipeline labels in 'kOldLabels' have "D16" for this case (i.e., "D32F" is a
                // fine Depth buffer type but won't match the strings).
                bool skip = false;
                for (const RenderPassProperties& rpp : precompileCase.fRenderPassProps) {
                    if (rpp.fDSFlags == DepthStencilFlags::kDepth) {
                        skip = true;
                    }
                }

                if (skip) {
                    skipped = true;
                    return;
                }
            }

            SkSpan<const SkBlendMode> blendModes = precompileCase.fPaintOptions.getBlendModes();
            bool skip = false;
            for (SkBlendMode bm : blendModes) {
                if (bm == SkBlendMode::kSrc && !caps->shaderCaps()->fDualSourceBlendingSupport) {
                    // The Pipeline labels were gathered on a device w/ dual source blending.
                    // kSrc blend mode w/o dual source blending can result in a dst read and, thus,
                    // break the string matching.
                    skip = true;
                    break;
                }
            }

            if (skip) {
                skipped = true;
                return;
            }

            RunTest(precompileContext, reporter, precompileCase, index, labels, &collector,
                    checkPaintOptionCoverage);
        });

    if (checkPipelineLabelCoverage && !skipped) {
        REPORTER_ASSERT(reporter, collector.numNotCovered() == 0,
                        "%d Pipeline labels are not covered", collector.numNotCovered());
    }

#if defined(FINAL_REPORT)
    // This block prints out a final report. This includes a list of the cases in 'labels' that
    // were not covered by the PaintOptions.

    collector.finalReport();
#endif
}

} // namespace PrecompileTestUtils

#endif // SK_GRAPHITE
