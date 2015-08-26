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
#include "batches/GrBatch.h"
#include "effects/GrPorterDuffXferProcessor.h"

GrPipelineBuilder::GrPipelineBuilder()
    : fProcDataManager(new GrProcessorDataManager), fFlags(0x0), fDrawFace(kBoth_DrawFace) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
}

GrPipelineBuilder& GrPipelineBuilder::operator=(const GrPipelineBuilder& that) {
    fProcDataManager.reset(new GrProcessorDataManager(*that.processorDataManager()));
    fRenderTarget.reset(SkSafeRef(that.fRenderTarget.get()));
    fFlags = that.fFlags;
    fStencilSettings = that.fStencilSettings;
    fDrawFace = that.fDrawFace;
    fXPFactory.reset(SkRef(that.getXPFactory()));
    fColorStages = that.fColorStages;
    fCoverageStages = that.fCoverageStages;
    fClip = that.fClip;

    return *this;
}

GrPipelineBuilder::GrPipelineBuilder(const GrPaint& paint, GrRenderTarget* rt, const GrClip& clip) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)

    // TODO keep this logically const using an AutoReset
    fProcDataManager.reset(
         const_cast<GrProcessorDataManager*>(SkRef(paint.processorDataManager())));

    fColorStages.reset();
    fCoverageStages.reset();

    for (int i = 0; i < paint.numColorStages(); ++i) {
        fColorStages.push_back(paint.getColorStage(i));
    }

    for (int i = 0; i < paint.numCoverageStages(); ++i) {
        fCoverageStages.push_back(paint.getCoverageStage(i));
    }

    fXPFactory.reset(SkRef(paint.getXPFactory()));

    this->setRenderTarget(rt);

    // These have no equivalent in GrPaint, set them to defaults
    fDrawFace = kBoth_DrawFace;
    fStencilSettings.setDisabled();
    fFlags = 0;

    fClip = clip;

    this->setState(GrPipelineBuilder::kDither_Flag, paint.isDither());
    this->setState(GrPipelineBuilder::kHWAntialias_Flag,
                   rt->isUnifiedMultisampled() && paint.isAntiAlias());
}

//////////////////////////////////////////////////////////////////////////////s

bool GrPipelineBuilder::willXPNeedDstTexture(const GrCaps& caps,
                                             const GrProcOptInfo& colorPOI,
                                             const GrProcOptInfo& coveragePOI) const {
    return this->getXPFactory()->willNeedDstTexture(caps, colorPOI, coveragePOI,
                                                    this->hasMixedSamples());
}

void GrPipelineBuilder::AutoRestoreFragmentProcessorState::set(
                                                         const GrPipelineBuilder* pipelineBuilder) {
    if (fPipelineBuilder) {
        int m = fPipelineBuilder->numColorFragmentStages() - fColorEffectCnt;
        SkASSERT(m >= 0);
        fPipelineBuilder->fColorStages.pop_back_n(m);

        int n = fPipelineBuilder->numCoverageFragmentStages() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        fPipelineBuilder->fCoverageStages.pop_back_n(n);

        SkDEBUGCODE(--fPipelineBuilder->fBlockEffectRemovalCnt;)
        fPipelineBuilder->getProcessorDataManager()->restoreToSaveMarker(/*fSaveMarker*/);
    }
    fPipelineBuilder = const_cast<GrPipelineBuilder*>(pipelineBuilder);
    if (NULL != pipelineBuilder) {
        fColorEffectCnt = pipelineBuilder->numColorFragmentStages();
        fCoverageEffectCnt = pipelineBuilder->numCoverageFragmentStages();
        SkDEBUGCODE(++pipelineBuilder->fBlockEffectRemovalCnt;)
        fSaveMarker = pipelineBuilder->processorDataManager()->currentSaveMarker();
    }
}

////////////////////////////////////////////////////////////////////////////////

GrPipelineBuilder::~GrPipelineBuilder() {
    SkASSERT(0 == fBlockEffectRemovalCnt);
}

////////////////////////////////////////////////////////////////////////////////

void GrPipelineBuilder::calcColorInvariantOutput(const GrPrimitiveProcessor* pp) const {
    fColorProcInfo.calcColorWithPrimProc(pp, fColorStages.begin(), this->numColorFragmentStages());
}

void GrPipelineBuilder::calcCoverageInvariantOutput(const GrPrimitiveProcessor* pp) const {
    fCoverageProcInfo.calcCoverageWithPrimProc(pp, fCoverageStages.begin(),
                                            this->numCoverageFragmentStages());
}

void GrPipelineBuilder::calcColorInvariantOutput(const GrDrawBatch* batch) const {
    fColorProcInfo.calcColorWithBatch(batch, fColorStages.begin(), this->numColorFragmentStages());
}

void GrPipelineBuilder::calcCoverageInvariantOutput(const GrDrawBatch* batch) const {
    fCoverageProcInfo.calcCoverageWithBatch(batch, fCoverageStages.begin(),
                                            this->numCoverageFragmentStages());
}

