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
#include "GrProcOptInfo.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetOpList.h"
#include "GrRenderTargetPriv.h"
#include "GrXferProcessor.h"

#include "ops/GrOp.h"

GrPipeline* GrPipeline::CreateAt(void* memory, const CreateArgs& args,
                                 GrPipelineOptimizations* optimizations) {
    SkASSERT(args.fAnalysis);
    GrRenderTarget* rt = args.fRenderTargetContext->accessRenderTarget();
    if (!rt) {
        return nullptr;
    }

    GrPipeline* pipeline = new (memory) GrPipeline;
    pipeline->fRenderTarget.reset(rt);
    SkASSERT(pipeline->fRenderTarget);
    pipeline->fScissorState = args.fAppliedClip->scissorState();
    pipeline->fWindowRectsState = args.fAppliedClip->windowRectsState();
    pipeline->fUserStencilSettings = args.fUserStencil;
    pipeline->fDrawFace = static_cast<int16_t>(args.fDrawFace);

    pipeline->fFlags = args.fFlags;
    if (args.fProcessors->usesDistanceVectorField()) {
        pipeline->fFlags |= kUsesDistanceVectorField_Flag;
    }
    if (args.fAppliedClip->hasStencilClip()) {
        pipeline->fFlags |= kHasStencilClip_Flag;
    }
    if (!args.fUserStencil->isDisabled(args.fAppliedClip->hasStencilClip())) {
        pipeline->fFlags |= kStencilEnabled_Flag;
    }
    if (args.fProcessors->disableOutputConversionToSRGB()) {
        pipeline->fFlags |= kDisableOutputConversionToSRGB_Flag;
    }
    if (args.fProcessors->allowSRGBInputs()) {
        pipeline->fFlags |= kAllowSRGBInputs_Flag;
    }

    bool isHWAA = kHWAntialias_Flag & args.fFlags;

    // Create XferProcessor from DS's XPFactory
    bool hasMixedSamples = args.fRenderTargetContext->hasMixedSamples() &&
                           (isHWAA || pipeline->isStencilEnabled());
    const GrXPFactory* xpFactory = args.fProcessors->xpFactory();
    sk_sp<GrXferProcessor> xferProcessor;
    if (xpFactory) {
        xferProcessor.reset(xpFactory->createXferProcessor(*args.fAnalysis, hasMixedSamples,
                                                           &args.fDstTexture, *args.fCaps));
        if (!xferProcessor) {
            pipeline->~GrPipeline();
            return nullptr;
        }
    } else {
        // This may return nullptr in the common case of src-over implemented using hw blending.
        xferProcessor.reset(GrPorterDuffXPFactory::CreateSrcOverXferProcessor(
                *args.fCaps, *args.fAnalysis, hasMixedSamples, &args.fDstTexture));
    }
    GrColor overrideColor = GrColor_ILLEGAL;
    int colorFPsToEliminate = args.fAnalysis->initialColorProcessorsToEliminate(&overrideColor);

    GrXferProcessor::OptFlags optFlags = GrXferProcessor::kNone_OptFlags;

    const GrXferProcessor* xpForOpts = xferProcessor ? xferProcessor.get() :
                                                       &GrPorterDuffXPFactory::SimpleSrcOverXP();
    optFlags = xpForOpts->getOptimizations(
            *args.fAnalysis, args.fUserStencil->doesWrite(args.fAppliedClip->hasStencilClip()),
            &overrideColor, *args.fCaps);

    // When path rendering the stencil settings are not always set on the GrPipelineBuilder
    // so we must check the draw type. In cases where we will skip drawing we simply return a
    // null GrPipeline.
    if (GrXferProcessor::kSkipDraw_OptFlag & optFlags) {
        pipeline->~GrPipeline();
        return nullptr;
    }

    // No need to have an override color if it isn't even going to be used.
    if (SkToBool(GrXferProcessor::kIgnoreColor_OptFlag & optFlags)) {
        overrideColor = GrColor_ILLEGAL;
    }

    pipeline->fXferProcessor.reset(xferProcessor.get());

    if ((optFlags & GrXferProcessor::kIgnoreColor_OptFlag) ||
        (optFlags & GrXferProcessor::kOverrideColor_OptFlag)) {
        colorFPsToEliminate = args.fProcessors->numColorFragmentProcessors();
    }

    bool usesLocalCoords = false;

    // Copy GrFragmentProcessors from GrPipelineBuilder to Pipeline, possibly removing some of the
    // color fragment processors.
    pipeline->fNumColorProcessors =
            args.fProcessors->numColorFragmentProcessors() - colorFPsToEliminate;
    int numTotalProcessors =
            pipeline->fNumColorProcessors + args.fProcessors->numCoverageFragmentProcessors();
    if (args.fAppliedClip->clipCoverageFragmentProcessor()) {
        ++numTotalProcessors;
    }
    pipeline->fFragmentProcessors.reset(numTotalProcessors);
    int currFPIdx = 0;
    for (int i = colorFPsToEliminate; i < args.fProcessors->numColorFragmentProcessors();
         ++i, ++currFPIdx) {
        const GrFragmentProcessor* fp = args.fProcessors->colorFragmentProcessor(i);
        pipeline->fFragmentProcessors[currFPIdx].reset(fp);
        usesLocalCoords = usesLocalCoords || fp->usesLocalCoords();
    }

    for (int i = 0; i < args.fProcessors->numCoverageFragmentProcessors(); ++i, ++currFPIdx) {
        const GrFragmentProcessor* fp = args.fProcessors->coverageFragmentProcessor(i);
        pipeline->fFragmentProcessors[currFPIdx].reset(fp);
        usesLocalCoords = usesLocalCoords || fp->usesLocalCoords();
    }
    if (const GrFragmentProcessor* fp = args.fAppliedClip->clipCoverageFragmentProcessor()) {
        pipeline->fFragmentProcessors[currFPIdx].reset(fp);
        usesLocalCoords = usesLocalCoords || fp->usesLocalCoords();
    }

    // Setup info we need to pass to GrPrimitiveProcessors that are used with this GrPipeline.
    optimizations->fFlags = 0;
    if (GrColor_ILLEGAL != overrideColor) {
        optimizations->fFlags |= GrPipelineOptimizations::kUseOverrideColor_Flag;
        optimizations->fOverrideColor = overrideColor;
    }
    if (usesLocalCoords) {
        optimizations->fFlags |= GrPipelineOptimizations::kReadsLocalCoords_Flag;
    }
    if (SkToBool(optFlags & GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag)) {
        optimizations->fFlags |= GrPipelineOptimizations::kCanTweakAlphaForCoverage_Flag;
    }

    if (GrXPFactory::WillReadDst(xpFactory, *args.fAnalysis)) {
        optimizations->fFlags |= GrPipelineOptimizations::kXPReadsDst_Flag;
    }

    return pipeline;
}

static void add_dependencies_for_processor(const GrFragmentProcessor* proc, GrRenderTarget* rt) {
    GrFragmentProcessor::TextureAccessIter iter(proc);
    while (const GrProcessor::TextureSampler* sampler = iter.next()) {
        SkASSERT(rt->getLastOpList());
        rt->getLastOpList()->addDependency(sampler->texture());
    }
}

void GrPipeline::addDependenciesTo(GrRenderTarget* rt) const {
    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        add_dependencies_for_processor(fFragmentProcessors[i].get(), rt);
    }

    const GrXferProcessor& xfer = this->getXferProcessor();

    for (int i = 0; i < xfer.numTextureSamplers(); ++i) {
        GrTexture* texture = xfer.textureSampler(i).texture();
        SkASSERT(rt->getLastOpList());
        rt->getLastOpList()->addDependency(texture);
    }
}

GrPipeline::GrPipeline(GrRenderTarget* rt, SkBlendMode blendmode)
    : fRenderTarget(rt)
    , fScissorState()
    , fWindowRectsState()
    , fUserStencilSettings(&GrUserStencilSettings::kUnused)
    , fDrawFace(static_cast<uint16_t>(GrDrawFace::kBoth))
    , fFlags()
    , fXferProcessor(GrPorterDuffXPFactory::CreateNoCoverageXP(blendmode).get())
    , fFragmentProcessors()
    , fNumColorProcessors(0) {
}

////////////////////////////////////////////////////////////////////////////////

bool GrPipeline::AreEqual(const GrPipeline& a, const GrPipeline& b) {
    SkASSERT(&a != &b);

    if (a.getRenderTarget() != b.getRenderTarget() ||
        a.fFragmentProcessors.count() != b.fFragmentProcessors.count() ||
        a.fNumColorProcessors != b.fNumColorProcessors ||
        a.fScissorState != b.fScissorState ||
        !a.fWindowRectsState.cheapEqualTo(b.fWindowRectsState) ||
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
