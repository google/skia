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

void GrPipeline::init(const InitArgs& args) {
    SkASSERT(args.fRenderTarget);
    SkASSERT(args.fProcessors);
    SkASSERT(args.fProcessors->isFinalized());

    fRenderTarget.reset(args.fRenderTarget);

    fFlags = args.fFlags;
    if (args.fAppliedClip) {
        fScissorState = args.fAppliedClip->scissorState();
        if (args.fAppliedClip->hasStencilClip()) {
            fFlags |= kHasStencilClip_Flag;
        }
        fWindowRectsState = args.fAppliedClip->windowRectsState();
    }
    if (!args.fUserStencil->isDisabled(fFlags & kHasStencilClip_Flag)) {
        fFlags |= kStencilEnabled_Flag;
    }

    fUserStencilSettings = args.fUserStencil;

    fXferProcessor = args.fProcessors->refXferProcessor();

    if (args.fDstProxy.proxy()) {
        if (!args.fDstProxy.proxy()->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }

        fDstTextureProxy.reset(args.fDstProxy.proxy());
        fDstTextureOffset = args.fDstProxy.offset();
    }

    // Copy GrFragmentProcessors from GrProcessorSet to Pipeline
    fNumColorProcessors = args.fProcessors->numColorFragmentProcessors();
    int numTotalProcessors =
            fNumColorProcessors + args.fProcessors->numCoverageFragmentProcessors();
    if (args.fAppliedClip && args.fAppliedClip->clipCoverageFragmentProcessor()) {
        ++numTotalProcessors;
    }
    fFragmentProcessors.reset(numTotalProcessors);
    int currFPIdx = 0;
    for (int i = 0; i < args.fProcessors->numColorFragmentProcessors(); ++i, ++currFPIdx) {
        const GrFragmentProcessor* fp = args.fProcessors->colorFragmentProcessor(i);
        fFragmentProcessors[currFPIdx].reset(fp);
        if (!fp->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }
    }

    for (int i = 0; i < args.fProcessors->numCoverageFragmentProcessors(); ++i, ++currFPIdx) {
        const GrFragmentProcessor* fp = args.fProcessors->coverageFragmentProcessor(i);
        fFragmentProcessors[currFPIdx].reset(fp);
        if (!fp->instantiate(args.fResourceProvider)) {
            this->markAsBad();
        }
    }
    if (args.fAppliedClip) {
        if (const GrFragmentProcessor* fp = args.fAppliedClip->clipCoverageFragmentProcessor()) {
            fFragmentProcessors[currFPIdx].reset(fp);
            if (!fp->instantiate(args.fResourceProvider)) {
                this->markAsBad();
            }
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
        fDstTextureProxy.get()->priv().peekTexture() == fRenderTarget.get()->asTexture()) {
        return kTexture_GrXferBarrierType;
    }
    return this->getXferProcessor().xferBarrierType(caps);
}

GrPipeline::GrPipeline(GrRenderTarget* rt, ScissorState scissorState, SkBlendMode blendmode)
    : fRenderTarget(rt)
    , fScissorState()
    , fWindowRectsState()
    , fUserStencilSettings(&GrUserStencilSettings::kUnused)
    , fFlags()
    , fXferProcessor(GrPorterDuffXPFactory::MakeNoCoverageXP(blendmode))
    , fFragmentProcessors()
    , fNumColorProcessors(0) {
    if (ScissorState::kEnabled == scissorState) {
        fScissorState.set({0, 0, 0, 0}); // caller will use the DynamicState struct.
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrPipeline::AreEqual(const GrPipeline& a, const GrPipeline& b) {
    SkASSERT(&a != &b);

    if (a.getRenderTarget() != b.getRenderTarget() ||
        a.fFragmentProcessors.count() != b.fFragmentProcessors.count() ||
        a.fNumColorProcessors != b.fNumColorProcessors ||
        a.fScissorState != b.fScissorState ||
        a.fWindowRectsState != b.fWindowRectsState ||
        a.fFlags != b.fFlags ||
        a.fUserStencilSettings != b.fUserStencilSettings) {
        return false;
    }

    // Most of the time both are nullptr
    if (a.fXferProcessor.get() || b.fXferProcessor.get()) {
        if (!a.getXferProcessor().isEqual(b.getXferProcessor())) {
            return false;
        }
    }

    for (int i = 0; i < a.numFragmentProcessors(); i++) {
        if (!a.getFragmentProcessor(i).isEqual(b.getFragmentProcessor(i))) {
            return false;
        }
    }
    return true;
}
