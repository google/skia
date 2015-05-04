/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPipelineBuilder.h"

#include "GrBatch.h"
#include "GrBlend.h"
#include "GrPaint.h"
#include "GrPipeline.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"
#include "effects/GrPorterDuffXferProcessor.h"

GrPipelineBuilder::GrPipelineBuilder()
    : fFlags(0x0)
    , fDrawFace(kBoth_DrawFace)
    , fColorProcInfoValid(false)
    , fCoverageProcInfoValid(false)
    , fColorCache(GrColor_ILLEGAL)
    , fCoverageCache(GrColor_ILLEGAL) {
    SkDEBUGCODE(fBlockEffectRemovalCnt = 0;)
}

GrPipelineBuilder& GrPipelineBuilder::operator=(const GrPipelineBuilder& that) {
    fRenderTarget.reset(SkSafeRef(that.fRenderTarget.get()));
    fFlags = that.fFlags;
    fStencilSettings = that.fStencilSettings;
    fDrawFace = that.fDrawFace;
    fXPFactory.reset(SkRef(that.getXPFactory()));
    fColorStages = that.fColorStages;
    fCoverageStages = that.fCoverageStages;
    fClip = that.fClip;

    fColorProcInfoValid = that.fColorProcInfoValid;
    fCoverageProcInfoValid = that.fCoverageProcInfoValid;
    fColorCache = that.fColorCache;
    fCoverageCache = that.fCoverageCache;
    if (fColorProcInfoValid) {
        fColorProcInfo = that.fColorProcInfo;
    }
    if (fCoverageProcInfoValid) {
        fCoverageProcInfo = that.fCoverageProcInfo;
    }
    return *this;
}

void GrPipelineBuilder::setFromPaint(const GrPaint& paint, GrRenderTarget* rt, const GrClip& clip) {
    SkASSERT(0 == fBlockEffectRemovalCnt || 0 == this->numFragmentStages());

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
                   rt->isMultisampled() && paint.isAntiAlias());

    fColorProcInfoValid = false;
    fCoverageProcInfoValid = false;

    fColorCache = GrColor_ILLEGAL;
    fCoverageCache = GrColor_ILLEGAL;
}

//////////////////////////////////////////////////////////////////////////////s

bool GrPipelineBuilder::willXPNeedDstCopy(const GrDrawTargetCaps& caps,
                                          const GrProcOptInfo& colorPOI,
                                          const GrProcOptInfo& coveragePOI) const {
    return this->getXPFactory()->willNeedDstCopy(caps, colorPOI, coveragePOI);
}

void GrPipelineBuilder::AutoRestoreFragmentProcessors::set(GrPipelineBuilder* pipelineBuilder) {
    if (fPipelineBuilder) {
        int m = fPipelineBuilder->numColorFragmentStages() - fColorEffectCnt;
        SkASSERT(m >= 0);
        fPipelineBuilder->fColorStages.pop_back_n(m);

        int n = fPipelineBuilder->numCoverageFragmentStages() - fCoverageEffectCnt;
        SkASSERT(n >= 0);
        fPipelineBuilder->fCoverageStages.pop_back_n(n);
        if (m + n > 0) {
            fPipelineBuilder->fColorProcInfoValid = false;
            fPipelineBuilder->fCoverageProcInfoValid = false;
        }
        SkDEBUGCODE(--fPipelineBuilder->fBlockEffectRemovalCnt;)
    }
    fPipelineBuilder = pipelineBuilder;
    if (NULL != pipelineBuilder) {
        fColorEffectCnt = pipelineBuilder->numColorFragmentStages();
        fCoverageEffectCnt = pipelineBuilder->numCoverageFragmentStages();
        SkDEBUGCODE(++pipelineBuilder->fBlockEffectRemovalCnt;)
    }
}

////////////////////////////////////////////////////////////////////////////////

GrPipelineBuilder::~GrPipelineBuilder() {
    SkASSERT(0 == fBlockEffectRemovalCnt);
}

////////////////////////////////////////////////////////////////////////////////

bool GrPipelineBuilder::willBlendWithDst(const GrPrimitiveProcessor* pp) const {
    this->calcColorInvariantOutput(pp);
    this->calcCoverageInvariantOutput(pp);
    
    GrXPFactory::InvariantOutput output;
    fXPFactory->getInvariantOutput(fColorProcInfo, fCoverageProcInfo, &output);
    return output.fWillBlendWithDst;
}

void GrPipelineBuilder::calcColorInvariantOutput(const GrPrimitiveProcessor* pp) const {
    fColorProcInfo.calcColorWithPrimProc(pp, fColorStages.begin(), this->numColorFragmentStages());
    fColorProcInfoValid = false;

}

void GrPipelineBuilder::calcCoverageInvariantOutput(const GrPrimitiveProcessor* pp) const {
    fCoverageProcInfo.calcCoverageWithPrimProc(pp, fCoverageStages.begin(),
                                               this->numCoverageFragmentStages());
    fCoverageProcInfoValid = false;
}

void GrPipelineBuilder::calcColorInvariantOutput(const GrBatch* batch) const {
    fColorProcInfo.calcColorWithBatch(batch, fColorStages.begin(), this->numColorFragmentStages());
    fColorProcInfoValid = false;
}

void GrPipelineBuilder::calcCoverageInvariantOutput(const GrBatch* batch) const {
    fCoverageProcInfo.calcCoverageWithBatch(batch, fCoverageStages.begin(),
                                            this->numCoverageFragmentStages());
    fCoverageProcInfoValid = false;
}


void GrPipelineBuilder::calcColorInvariantOutput(GrColor color) const {
    if (!fColorProcInfoValid || color != fColorCache) {
        GrColorComponentFlags flags = kRGBA_GrColorComponentFlags;
        fColorProcInfo.calcWithInitialValues(fColorStages.begin(),this->numColorFragmentStages(),
                                             color, flags, false);
        fColorProcInfoValid = true;
        fColorCache = color;
    }
}

void GrPipelineBuilder::calcCoverageInvariantOutput(GrColor coverage) const {
    if (!fCoverageProcInfoValid || coverage != fCoverageCache) {
        GrColorComponentFlags flags = kRGBA_GrColorComponentFlags;
        fCoverageProcInfo.calcWithInitialValues(fCoverageStages.begin(),
                                                this->numCoverageFragmentStages(), coverage, flags,
                                                true);
        fCoverageProcInfoValid = true;
        fCoverageCache = coverage;
    }
}
