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
#include "GrPipelineBuilder.h"
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
    if (args.fProcessors->usesDistanceVectorField()) {
        fFlags |= kUsesDistanceVectorField_Flag;
    }
    if (args.fProcessors->disableOutputConversionToSRGB()) {
        fFlags |= kDisableOutputConversionToSRGB_Flag;
    }
    if (args.fProcessors->allowSRGBInputs()) {
        fFlags |= kAllowSRGBInputs_Flag;
    }
    if (!args.fUserStencil->isDisabled(fFlags & kHasStencilClip_Flag)) {
        fFlags |= kStencilEnabled_Flag;
    }

    fUserStencilSettings = args.fUserStencil;

    fDrawFace = static_cast<int16_t>(args.fDrawFace);

    fXferProcessor = args.fProcessors->refXferProcessor();

    if (args.fDstTexture.texture()) {
        fDstTexture.reset(args.fDstTexture.texture());
        fDstTextureOffset = args.fDstTexture.offset();
    }

    // Copy GrFragmentProcessors from GrPipelineBuilder to Pipeline, possibly removing some of the
    // color fragment processors.
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
    }

    for (int i = 0; i < args.fProcessors->numCoverageFragmentProcessors(); ++i, ++currFPIdx) {
        const GrFragmentProcessor* fp = args.fProcessors->coverageFragmentProcessor(i);
        fFragmentProcessors[currFPIdx].reset(fp);
    }
    if (args.fAppliedClip) {
        if (const GrFragmentProcessor* fp = args.fAppliedClip->clipCoverageFragmentProcessor()) {
            fFragmentProcessors[currFPIdx].reset(fp);
        }
    }
}

static void add_dependencies_for_processor(const GrFragmentProcessor* proc, GrRenderTarget* rt) {
    GrFragmentProcessor::TextureAccessIter iter(proc);
    while (const GrResourceIOProcessor::TextureSampler* sampler = iter.next()) {
        SkASSERT(rt->getLastOpList());
        rt->getLastOpList()->addDependency(sampler->texture());
    }
}

void GrPipeline::addDependenciesTo(GrRenderTarget* rt) const {
    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        add_dependencies_for_processor(fFragmentProcessors[i].get(), rt);
    }

    if (fDstTexture) {
        SkASSERT(rt->getLastOpList());
        rt->getLastOpList()->addDependency(fDstTexture.get());
    }
}

GrPipeline::GrPipeline(GrRenderTarget* rt, SkBlendMode blendmode)
        : fRenderTarget(rt)
        , fScissorState()
        , fWindowRectsState()
        , fUserStencilSettings(&GrUserStencilSettings::kUnused)
        , fDrawFace(static_cast<uint16_t>(GrDrawFace::kBoth))
        , fFlags()
        , fXferProcessor(GrPorterDuffXPFactory::MakeNoCoverageXP(blendmode))
        , fFragmentProcessors()
        , fNumColorProcessors(0) {}

////////////////////////////////////////////////////////////////////////////////

bool GrPipeline::AreEqual(const GrPipeline& a, const GrPipeline& b) {
    SkASSERT(&a != &b);

    if (a.getRenderTarget() != b.getRenderTarget() ||
        a.fFragmentProcessors.count() != b.fFragmentProcessors.count() ||
        a.fNumColorProcessors != b.fNumColorProcessors ||
        a.fScissorState != b.fScissorState ||
        a.fWindowRectsState != b.fWindowRectsState ||
        a.fFlags != b.fFlags ||
        a.fUserStencilSettings != b.fUserStencilSettings ||
        a.fDrawFace != b.fDrawFace) {
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
