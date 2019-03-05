/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPipeline.h"

#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrGpu.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetOpList.h"
#include "GrXferProcessor.h"

#include "ops/GrOp.h"

GrPipeline::GrPipeline(const InitArgs& args,
                       GrProcessorSet&& processors,
                       GrAppliedClip&& appliedClip) {
    SkASSERT(processors.isFinalized());

    fFlags = args.fFlags;
    if (appliedClip.hasStencilClip()) {
        fFlags |= kHasStencilClip_Flag;
    }
    if (appliedClip.scissorState().enabled()) {
        fFlags |= kScissorEnabled_Flag;
    }

    fWindowRectsState = appliedClip.windowRectsState();
    if (!args.fUserStencil->isDisabled(fFlags & kHasStencilClip_Flag)) {
        fFlags |= kStencilEnabled_Flag;
    }

    fUserStencilSettings = args.fUserStencil;

    fXferProcessor = processors.refXferProcessor();

    if (args.fDstProxy.proxy()) {
        if (!args.fDstProxy.proxy()->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }

        fDstTextureProxy.reset(args.fDstProxy.proxy());
        fDstTextureOffset = args.fDstProxy.offset();
    }

    // Copy GrFragmentProcessors from GrProcessorSet to Pipeline
    fNumColorProcessors = processors.numColorFragmentProcessors();
    int numTotalProcessors = fNumColorProcessors +
                             processors.numCoverageFragmentProcessors() +
                             appliedClip.numClipCoverageFragmentProcessors();
    fFragmentProcessors.reset(numTotalProcessors);
    int currFPIdx = 0;
    for (int i = 0; i < processors.numColorFragmentProcessors(); ++i, ++currFPIdx) {
        fFragmentProcessors[currFPIdx] = processors.detachColorFragmentProcessor(i);
        if (!fFragmentProcessors[currFPIdx]->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }
    }
    for (int i = 0; i < processors.numCoverageFragmentProcessors(); ++i, ++currFPIdx) {
        fFragmentProcessors[currFPIdx] = processors.detachCoverageFragmentProcessor(i);
        if (!fFragmentProcessors[currFPIdx]->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }
    }
    for (int i = 0; i < appliedClip.numClipCoverageFragmentProcessors(); ++i, ++currFPIdx) {
        fFragmentProcessors[currFPIdx] = appliedClip.detachClipCoverageFragmentProcessor(i);
        if (!fFragmentProcessors[currFPIdx]->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }
    }
}

void GrPipeline::addDependenciesTo(GrOpList* opList, const GrCaps& caps) const {
    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        GrFragmentProcessor::TextureAccessIter iter(fFragmentProcessors[i].get());
        while (const GrFragmentProcessor::TextureSampler* sampler = iter.next()) {
            opList->addDependency(sampler->proxy(), caps);
        }
    }

    if (fDstTextureProxy) {
        opList->addDependency(fDstTextureProxy.get(), caps);
    }

}

GrXferBarrierType GrPipeline::xferBarrierType(GrTexture* texture, const GrCaps& caps) const {
    if (fDstTextureProxy.get() && fDstTextureProxy.get()->peekTexture() == texture) {
        return kTexture_GrXferBarrierType;
    }
    return this->getXferProcessor().xferBarrierType(caps);
}

GrPipeline::GrPipeline(GrScissorTest scissorTest, SkBlendMode blendmode, uint32_t flags,
                       const GrUserStencilSettings* userStencil)
        : fWindowRectsState()
        , fUserStencilSettings(userStencil)
        , fFlags(flags)
        , fXferProcessor(GrPorterDuffXPFactory::MakeNoCoverageXP(blendmode))
        , fFragmentProcessors()
        , fNumColorProcessors(0) {
    if (GrScissorTest::kEnabled == scissorTest) {
        fFlags |= kScissorEnabled_Flag;
    }
    if (!userStencil->isDisabled(false)) {
        fFlags |= kStencilEnabled_Flag;
    }
}

uint32_t GrPipeline::getBlendInfoKey() const {
    GrXferProcessor::BlendInfo blendInfo;
    this->getXferProcessor().getBlendInfo(&blendInfo);

    static const uint32_t kBlendWriteShift = 1;
    static const uint32_t kBlendCoeffShift = 5;
    GR_STATIC_ASSERT(kLast_GrBlendCoeff < (1 << kBlendCoeffShift));
    GR_STATIC_ASSERT(kFirstAdvancedGrBlendEquation - 1 < 4);

    uint32_t key = blendInfo.fWriteColor;
    key |= (blendInfo.fSrcBlend << kBlendWriteShift);
    key |= (blendInfo.fDstBlend << (kBlendWriteShift + kBlendCoeffShift));
    key |= (blendInfo.fEquation << (kBlendWriteShift + 2 * kBlendCoeffShift));

    return key;
}
