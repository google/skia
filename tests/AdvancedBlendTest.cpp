/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "GrContextPriv.h"
#include "GrPaint.h"
#include "GrProcessorSet.h"
#include "effects/GrCustomXfermode.h"

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
        SkPMColor4f overrideColor;
        GrProcessorSet::Analysis processorAnalysis =
                procs.finalize(
                        opaque, coverage, nullptr, &GrUserStencilSettings::kUnused,
                        GrFSAAType::kNone, caps, GrClampType::kAuto, &overrideColor);

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
