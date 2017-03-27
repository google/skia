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

GrPipelineOptimizations GrPipeline::init(const InitArgs& args) {
    SkASSERT(args.fAnalysis);
    SkASSERT(args.fRenderTarget);

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

    bool isHWAA = kHWAntialias_Flag & args.fFlags;

    // Create XferProcessor from DS's XPFactory
    bool hasMixedSamples = args.fRenderTarget->isMixedSampled() && (isHWAA || isStencilEnabled());
    const GrXPFactory* xpFactory = args.fProcessors->xpFactory();
    sk_sp<GrXferProcessor> xferProcessor;
    if (xpFactory) {
        xferProcessor.reset(xpFactory->createXferProcessor(*args.fAnalysis, hasMixedSamples,
                                                           &args.fDstTexture, *args.fCaps));
        SkASSERT(xferProcessor);
    } else {
        // This may return nullptr in the common case of src-over implemented using hw blending.
        xferProcessor.reset(GrPorterDuffXPFactory::CreateSrcOverXferProcessor(
                *args.fCaps, *args.fAnalysis, hasMixedSamples, &args.fDstTexture));
    }
    GrColor overrideColor = GrColor_ILLEGAL;
    int colorFPsToEliminate =
            args.fAnalysis->getInputColorOverrideAndColorProcessorEliminationCount(&overrideColor);
    colorFPsToEliminate = SkTMax(colorFPsToEliminate, 0);

    GrXferProcessor::OptFlags optFlags = GrXferProcessor::kNone_OptFlags;

    const GrXferProcessor* xpForOpts = xferProcessor ? xferProcessor.get() :
                                                       &GrPorterDuffXPFactory::SimpleSrcOverXP();
    optFlags = xpForOpts->getOptimizations(*args.fAnalysis);

    // No need to have an override color if it isn't even going to be used.
    if (SkToBool(GrXferProcessor::kIgnoreColor_OptFlag & optFlags)) {
        overrideColor = GrColor_ILLEGAL;
    }

    fXferProcessor.reset(xferProcessor.get());

    if ((optFlags & GrXferProcessor::kIgnoreColor_OptFlag)) {
        colorFPsToEliminate = args.fProcessors->numColorFragmentProcessors();
    }

    // Copy GrFragmentProcessors from GrPipelineBuilder to Pipeline, possibly removing some of the
    // color fragment processors.
    fNumColorProcessors = args.fProcessors->numColorFragmentProcessors() - colorFPsToEliminate;
    int numTotalProcessors =
            fNumColorProcessors + args.fProcessors->numCoverageFragmentProcessors();
    if (args.fAppliedClip && args.fAppliedClip->clipCoverageFragmentProcessor()) {
        ++numTotalProcessors;
    }
    fFragmentProcessors.reset(numTotalProcessors);
    int currFPIdx = 0;
    for (int i = colorFPsToEliminate; i < args.fProcessors->numColorFragmentProcessors();
         ++i, ++currFPIdx) {
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

    // Setup info we need to pass to GrPrimitiveProcessors that are used with this GrPipeline.
    GrPipelineOptimizations optimizations;
    optimizations.fFlags = 0;
    if (GrColor_ILLEGAL != overrideColor) {
        optimizations.fFlags |= GrPipelineOptimizations::kUseOverrideColor_Flag;
        optimizations.fOverrideColor = overrideColor;
    }
    if (args.fAnalysis->usesLocalCoords()) {
        optimizations.fFlags |= GrPipelineOptimizations::kReadsLocalCoords_Flag;
    }
    if (SkToBool(optFlags & GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag)) {
        optimizations.fFlags |= GrPipelineOptimizations::kCanTweakAlphaForCoverage_Flag;
    }
    return optimizations;
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
