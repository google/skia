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
#include "GrRenderTargetPriv.h"
#include "GrXferProcessor.h"

#include "ops/GrOp.h"

GrPipeline::GrPipeline(const InitArgs& args, GrProcessorSet processors, GrAppliedClip appliedClip) {
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


    fHeadColorProcessor = processors.detachColorFragmentProcessors();
    fHeadCoverageProcessor = processors.detachCoverageFragmentProcessors();
    auto clipFP = appliedClip.detachClipCoverageFragmentProcessor();
    if (clipFP) {
        clipFP->setNext(std::move(fHeadCoverageProcessor));
        fHeadCoverageProcessor = std::move(clipFP);
    }
    for (auto fp : GrFragmentProcessor::Series(fHeadColorProcessor.get())) {
        if (!fp->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }
    }
    for (auto fp : GrFragmentProcessor::Series(fHeadCoverageProcessor.get())) {
        if (!fp->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }
    }
}

void GrPipeline::addDependenciesTo(GrOpList* opList, const GrCaps& caps) const {
    for (auto fp : GrFragmentProcessor::Series(fHeadColorProcessor.get())) {
        GrFragmentProcessor::TextureAccessIter iter(fp);
        while (const GrResourceIOProcessor::TextureSampler* sampler = iter.next()) {
            opList->addDependency(sampler->proxy(), caps);
        }
    }
    for (auto fp : GrFragmentProcessor::Series(fHeadCoverageProcessor.get())) {
        GrFragmentProcessor::TextureAccessIter iter(fp);
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
        , fXferProcessor(GrPorterDuffXPFactory::MakeNoCoverageXP(blendmode)) {
    SkASSERT(proxy);
    if (ScissorState::kEnabled == scissorState) {
        fScissorState.set({0, 0, 0, 0}); // caller will use the DynamicState struct.
    }
}
