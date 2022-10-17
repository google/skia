/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkColorData.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/Blend.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrProcessorAnalysis.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/effects/GrCustomXfermode.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <utility>

struct GrContextOptions;

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(AdvancedBlendTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kApiLevel_T) {
    static constexpr auto opaque = GrProcessorAnalysisColor::Opaque::kYes;
    static constexpr auto coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    const GrCaps& caps = *ctxInfo.directContext()->priv().caps();

    for (int mode = (int)SkBlendMode::kLastMode; mode > (int)SkBlendMode::kLastCoeffMode; --mode) {
        const SkBlendMode blendMode = (SkBlendMode)mode;
        const skgpu::BlendEquation blendEquation = (skgpu::BlendEquation)(mode +
                ((int)skgpu::BlendEquation::kOverlay - (int)SkBlendMode::kOverlay));
        const GrXPFactory* xpf = GrCustomXfermode::Get(blendMode);

        GrXPFactory::AnalysisProperties xpfAnalysis =
                GrXPFactory::GetAnalysisProperties(xpf, opaque, coverage, caps, GrClampType::kAuto);

        GrPaint paint;
        paint.setXPFactory(xpf);
        GrProcessorSet procs(std::move(paint));
        SkPMColor4f overrideColor;
        GrProcessorSet::Analysis processorAnalysis = procs.finalize(
                opaque, coverage, nullptr, &GrUserStencilSettings::kUnused, caps,
                GrClampType::kAuto, &overrideColor);

        if (caps.advancedBlendEquationSupport() &&
                !caps.isAdvancedBlendEquationDisabled(blendEquation)) {
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
