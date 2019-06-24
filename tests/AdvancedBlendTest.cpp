/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/gpu/GrBlend.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkColorData.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPaint.h"
#include "src/gpu/GrProcessorAnalysis.h"
#include "src/gpu/GrProcessorSet.h"
#include "src/gpu/GrUserStencilSettings.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/effects/GrCustomXfermode.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

#include <utility>

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(AdvancedBlendTest, reporter, ctxInfo) {
    static constexpr auto opaque = GrProcessorAnalysisColor::Opaque::kYes;
    static constexpr auto coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    const GrCaps& caps = *ctxInfo.grContext()->priv().caps();

    for (int mode = (int)SkBlendMode::kLastMode; mode > (int)SkBlendMode::kLastCoeffMode; --mode) {
        const SkBlendMode blendMode = (SkBlendMode)mode;
        const GrBlendEquation blendEquation =
                (GrBlendEquation)(mode + (kOverlay_GrBlendEquation - (int)SkBlendMode::kOverlay));
        const GrXPFactory* xpf = GrCustomXfermode::Get(blendMode);

        GrXPFactory::AnalysisProperties xpfAnalysis =
                GrXPFactory::GetAnalysisProperties(xpf, opaque, coverage, caps, GrClampType::kAuto);

        GrPaint paint;
        paint.setXPFactory(xpf);
        GrProcessorSet procs(std::move(paint));
        bool hasMixedSampledCoverage = false;
        SkPMColor4f overrideColor;
        GrProcessorSet::Analysis processorAnalysis = procs.finalize(
                opaque, coverage, nullptr, &GrUserStencilSettings::kUnused, hasMixedSampledCoverage,
                caps, GrClampType::kAuto, &overrideColor);

        if (caps.advancedBlendEquationSupport() &&
                !caps.isAdvancedBlendEquationBlacklisted(blendEquation)) {
            REPORTER_ASSERT(reporter,
                            !(xpfAnalysis & GrXPFactory::AnalysisProperties::kReadsDstInShader));
            if (GrCaps::kAdvancedCoherent_BlendEquationSupport == caps.blendEquationSupport()) {
                REPORTER_ASSERT(reporter, !processorAnalysis.requiresNonOverlappingDraws());
            } else {
                REPORTER_ASSERT(reporter,
                                GrCaps::kAdvanced_BlendEquationSupport
                                        == caps.blendEquationSupport());
                REPORTER_ASSERT(reporter, processorAnalysis.requiresNonOverlappingDraws());
            }
        } else {
            REPORTER_ASSERT(reporter,
                            (xpfAnalysis & GrXPFactory::AnalysisProperties::kReadsDstInShader));
            if (xpfAnalysis & GrXPFactory::AnalysisProperties::kRequiresDstTexture) {
                REPORTER_ASSERT(reporter, processorAnalysis.requiresNonOverlappingDraws());
            } else {
                REPORTER_ASSERT(reporter, !processorAnalysis.requiresNonOverlappingDraws());
            }
        }
    }
}
