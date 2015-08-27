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

GrPipelineBuilder::GrPipelineBuilder(const GrPaint& paint, GrRenderTarget* rt, const GrClip& clip) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)

    // TODO keep this logically const using an AutoReset
    fProcDataManager.reset(
         const_cast<GrProcessorDataManager*>(SkRef(paint.processorDataManager())));

    for (int i = 0; i < paint.numColorFragmentProcessors(); ++i) {
        fColorFragmentProcessors.push_back(SkRef(paint.getColorFragmentProcessor(i)));
    }

    for (int i = 0; i < paint.numCoverageFragmentProcessors(); ++i) {
        fCoverageFragmentProcessors.push_back(SkRef(paint.getCoverageFragmentProcessor(i)));
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
        int m = fPipelineBuilder->numColorFragmentProcessors() - fColorEffectCnt;
        SkASSERT(m >= 0);
        for (int i = 0; i < m; ++i) {
            fPipelineBuilder->fColorFragmentProcessors.fromBack(i)->unref();
        }
        fPipelineBuilder->fColorFragmentProcessors.pop_back_n(m);

        int n = fPipelineBuilder->numCoverageFragmentProcessors() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        for (int i = 0; i < n; ++i) {
            fPipelineBuilder->fCoverageFragmentProcessors.fromBack(i)->unref();
        }
        fPipelineBuilder->fCoverageFragmentProcessors.pop_back_n(n);
        SkDEBUGCODE(--fPipelineBuilder->fBlockEffectRemovalCnt;)
        fPipelineBuilder->getProcessorDataManager()->restoreToSaveMarker(/*fSaveMarker*/);
    }
    fPipelineBuilder = const_cast<GrPipelineBuilder*>(pipelineBuilder);
    if (nullptr != pipelineBuilder) {
        fColorEffectCnt = pipelineBuilder->numColorFragmentProcessors();
        fCoverageEffectCnt = pipelineBuilder->numCoverageFragmentProcessors();
        SkDEBUGCODE(++pipelineBuilder->fBlockEffectRemovalCnt;)
        fSaveMarker = pipelineBuilder->processorDataManager()->currentSaveMarker();
    }
}

////////////////////////////////////////////////////////////////////////////////

GrPipelineBuilder::~GrPipelineBuilder() {
    SkASSERT(0 == fBlockEffectRemovalCnt);
    for (int i = 0; i < fColorFragmentProcessors.count(); ++i) {
        fColorFragmentProcessors[i]->unref();
    }
    for (int i = 0; i < fCoverageFragmentProcessors.count(); ++i) {
        fCoverageFragmentProcessors[i]->unref();
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrPipelineBuilder::calcColorInvariantOutput(const GrPrimitiveProcessor* pp) const {
    fColorProcInfo.calcColorWithPrimProc(pp, fColorFragmentProcessors.begin(),
                                         this->numColorFragmentProcessors());

}

void GrPipelineBuilder::calcCoverageInvariantOutput(const GrPrimitiveProcessor* pp) const {
    fCoverageProcInfo.calcCoverageWithPrimProc(pp, fCoverageFragmentProcessors.begin(),
                                               this->numCoverageFragmentProcessors());
}

void GrPipelineBuilder::calcColorInvariantOutput(const GrDrawBatch* batch) const {
    fColorProcInfo.calcColorWithBatch(batch, fColorFragmentProcessors.begin(),
                                      this->numColorFragmentProcessors());
}

void GrPipelineBuilder::calcCoverageInvariantOutput(const GrDrawBatch* batch) const {
    fCoverageProcInfo.calcCoverageWithBatch(batch, fCoverageFragmentProcessors.begin(),
                                            this->numCoverageFragmentProcessors());
}
