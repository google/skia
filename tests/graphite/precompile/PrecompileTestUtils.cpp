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
             const PrecompileSettings& settings,
             int precompileSettingsIndex,
             SkSpan<const PipelineLabel> cases,
             PipelineLabelInfoCollector* collector) {
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
