/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPipelineBuilder.h"

#include "GrBlend.h"
#include "GrPaint.h"
#include "GrPipeline.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "ops/GrOp.h"

GrPipelineBuilder::GrPipelineBuilder(const GrPaint& paint, GrAAType aaType)
        : fFlags(0x0)
        , fUserStencilSettings(&GrUserStencilSettings::kUnused)
        , fDrawFace(GrDrawFace::kBoth) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)

    for (int i = 0; i < paint.numColorFragmentProcessors(); ++i) {
        fColorFragmentProcessors.emplace_back(SkRef(paint.getColorFragmentProcessor(i)));
    }

    for (int i = 0; i < paint.numCoverageFragmentProcessors(); ++i) {
        fCoverageFragmentProcessors.emplace_back(SkRef(paint.getCoverageFragmentProcessor(i)));
    }

    fXPFactory.reset(SkSafeRef(paint.getXPFactory()));

    this->setState(GrPipelineBuilder::kHWAntialias_Flag, GrAATypeIsHW(aaType));
    this->setState(GrPipelineBuilder::kDisableOutputConversionToSRGB_Flag,
                   paint.getDisableOutputConversionToSRGB());
    this->setState(GrPipelineBuilder::kAllowSRGBInputs_Flag,
                   paint.getAllowSRGBInputs());
    this->setState(GrPipelineBuilder::kUsesDistanceVectorField_Flag,
                   paint.usesDistanceVectorField());
}

//////////////////////////////////////////////////////////////////////////////s

bool GrPipelineBuilder::willXPNeedDstTexture(const GrCaps& caps,
                                             const GrPipelineAnalysis& analysis) const {
    if (this->getXPFactory()) {
        return this->getXPFactory()->willNeedDstTexture(caps, analysis);
    }
    return GrPorterDuffXPFactory::SrcOverWillNeedDstTexture(caps, analysis);
}

void GrPipelineBuilder::AutoRestoreFragmentProcessorState::set(
                                                         const GrPipelineBuilder* pipelineBuilder) {
    if (fPipelineBuilder) {
        int m = fPipelineBuilder->numColorFragmentProcessors() - fColorEffectCnt;
        SkASSERT(m >= 0);
        fPipelineBuilder->fColorFragmentProcessors.pop_back_n(m);

        int n = fPipelineBuilder->numCoverageFragmentProcessors() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        fPipelineBuilder->fCoverageFragmentProcessors.pop_back_n(n);

        SkDEBUGCODE(--fPipelineBuilder->fBlockEffectRemovalCnt;)
    }
    fPipelineBuilder = const_cast<GrPipelineBuilder*>(pipelineBuilder);
    if (nullptr != pipelineBuilder) {
        fColorEffectCnt = pipelineBuilder->numColorFragmentProcessors();
        fCoverageEffectCnt = pipelineBuilder->numCoverageFragmentProcessors();
        SkDEBUGCODE(++pipelineBuilder->fBlockEffectRemovalCnt;)
    }
}

////////////////////////////////////////////////////////////////////////////////

GrPipelineBuilder::~GrPipelineBuilder() {
    SkASSERT(0 == fBlockEffectRemovalCnt);
}
