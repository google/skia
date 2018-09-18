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

GrPipeline::GrPipeline(const InitArgs& args, GrProcessorSet&& processors,
                       GrAppliedClip&& appliedClip) {
    SkASSERT(args.fProxy);
    SkASSERT(processors.isFinalized());

    fProxy.reset(args.fProxy);

    fFlags = args.fFlags;
    fScissorState = appliedClip.scissorState();
    if (appliedClip.hasStencilClip()) {
        fFlags |= kHasStencilClip_Flag;
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
        while (const GrResourceIOProcessor::TextureSampler* sampler = iter.next()) {
            opList->addDependency(sampler->proxy(), caps);
        }
    }

    if (fDstTextureProxy) {
        opList->addDependency(fDstTextureProxy.get(), caps);
    }

}

GrXferBarrierType GrPipeline::xferBarrierType(const GrCaps& caps) const {
    if (fDstTextureProxy.get() &&
        fDstTextureProxy.get()->priv().peekTexture() == fProxy.get()->priv().peekTexture()) {
        return kTexture_GrXferBarrierType;
    }
    return this->getXferProcessor().xferBarrierType(caps);
}

GrPipeline::GrPipeline(GrRenderTargetProxy* proxy, ScissorState scissorState, SkBlendMode blendmode)
        : fProxy(proxy)
        , fScissorState()
        , fWindowRectsState()
        , fUserStencilSettings(&GrUserStencilSettings::kUnused)
        , fFlags()
        , fXferProcessor(GrPorterDuffXPFactory::MakeNoCoverageXP(blendmode))
        , fFragmentProcessors()
        , fNumColorProcessors(0) {
    SkASSERT(proxy);
    if (ScissorState::kEnabled == scissorState) {
        fScissorState.set({0, 0, 0, 0}); // caller will use the DynamicState struct.
    }
}
