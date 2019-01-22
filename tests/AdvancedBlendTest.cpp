/*
 * Copyright 2015 Google Inc.
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

    for (int mode = (int)SkBlendMode::kLastMode; mode > (int)SkBlendMode::kLastCoeffMode; --mode) {
        const GrCaps& caps = *ctxInfo.grContext()->contextPriv().caps();
        const GrXPFactory* xpf = GrCustomXfermode::Get((SkBlendMode)mode);

        GrXPFactory::AnalysisProperties xpfAnalysis =
                GrXPFactory::GetAnalysisProperties(xpf, opaque, coverage, caps);

        GrPaint paint;
        paint.setXPFactory(xpf);
        GrProcessorSet procs(std::move(paint));
        SkPMColor4f overrideColor;
        GrProcessorSet::Analysis processorAnalysis =
            procs.finalize(opaque, coverage, nullptr, false, caps, &overrideColor);

        if (caps.advancedBlendEquationSupport()) {
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
                            GrCaps::kBasic_BlendEquationSupport == caps.blendEquationSupport());
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
