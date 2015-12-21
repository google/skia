/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPipeline.h"

#include "GrCaps.h"
#include "GrDrawTarget.h"
#include "GrGpu.h"
#include "GrPipelineBuilder.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"

#include "batches/GrBatch.h"

GrPipeline* GrPipeline::CreateAt(void* memory, const CreateArgs& args,
                                 GrXPOverridesForBatch* overrides) {
    const GrPipelineBuilder& builder = *args.fPipelineBuilder;

    // Create XferProcessor from DS's XPFactory
    const GrXPFactory* xpFactory = builder.getXPFactory();
    SkAutoTUnref<GrXferProcessor> xferProcessor;
    if (xpFactory) {
        xferProcessor.reset(xpFactory->createXferProcessor(args.fOpts,
                                                           builder.hasMixedSamples(),
                                                           &args.fDstTexture,
                                                           *args.fCaps));
        if (!xferProcessor) {
            return nullptr;
        }
    } else {
        // This may return nullptr in the common case of src-over implemented using hw blending.
        xferProcessor.reset(GrPorterDuffXPFactory::CreateSrcOverXferProcessor(
                                                                        *args.fCaps,
                                                                        args.fOpts,
                                                                        builder.hasMixedSamples(),
                                                                        &args.fDstTexture));
    }
   GrColor overrideColor = GrColor_ILLEGAL;
    if (args.fOpts.fColorPOI.firstEffectiveProcessorIndex() != 0) {
        overrideColor = args.fOpts.fColorPOI.inputColorToFirstEffectiveProccesor();
    }

    GrXferProcessor::OptFlags optFlags = GrXferProcessor::kNone_OptFlags;

    const GrXferProcessor* xpForOpts = xferProcessor ? xferProcessor.get() : 
                                                       &GrPorterDuffXPFactory::SimpleSrcOverXP();
    optFlags = xpForOpts->getOptimizations(args.fOpts,
                                           builder.getStencil().doesWrite(),
                                           &overrideColor,
                                           *args.fCaps);

    // When path rendering the stencil settings are not always set on the GrPipelineBuilder
    // so we must check the draw type. In cases where we will skip drawing we simply return a
    // null GrPipeline.
    if (GrXferProcessor::kSkipDraw_OptFlag & optFlags) {
        return nullptr;
    }

    // No need to have an override color if it isn't even going to be used.
    if (SkToBool(GrXferProcessor::kIgnoreColor_OptFlag & optFlags)) {
        overrideColor = GrColor_ILLEGAL;
    }

    GrPipeline* pipeline = new (memory) GrPipeline;
    pipeline->fXferProcessor.reset(xferProcessor);

    pipeline->fRenderTarget.reset(builder.fRenderTarget.get());
    SkASSERT(pipeline->fRenderTarget);
    pipeline->fScissorState = *args.fScissor;
    pipeline->fStencilSettings = builder.getStencil();
    pipeline->fDrawFace = builder.getDrawFace();

    pipeline->fFlags = 0;
    if (builder.isHWAntialias()) {
        pipeline->fFlags |= kHWAA_Flag;
    }
    if (builder.snapVerticesToPixelCenters()) {
        pipeline->fFlags |= kSnapVertices_Flag;
    }

    int firstColorProcessorIdx = args.fOpts.fColorPOI.firstEffectiveProcessorIndex();

    // TODO: Once we can handle single or four channel input into coverage GrFragmentProcessors
    // then we can use GrPipelineBuilder's coverageProcInfo (like color above) to set this initial
    // information.
    int firstCoverageProcessorIdx = 0;

    pipeline->adjustProgramFromOptimizations(builder, optFlags, args.fOpts.fColorPOI, 
                                             args.fOpts.fCoveragePOI, &firstColorProcessorIdx, 
                                             &firstCoverageProcessorIdx);

    bool usesLocalCoords = false;

    // Copy GrFragmentProcessors from GrPipelineBuilder to Pipeline
    pipeline->fNumColorProcessors = builder.numColorFragmentProcessors() - firstColorProcessorIdx;
    int numTotalProcessors = pipeline->fNumColorProcessors +
                             builder.numCoverageFragmentProcessors() - firstCoverageProcessorIdx;
    pipeline->fFragmentProcessors.reset(numTotalProcessors);
    int currFPIdx = 0;
    for (int i = firstColorProcessorIdx; i < builder.numColorFragmentProcessors();
         ++i, ++currFPIdx) {
        const GrFragmentProcessor* fp = builder.getColorFragmentProcessor(i);
        pipeline->fFragmentProcessors[currFPIdx].reset(fp);
        usesLocalCoords = usesLocalCoords || fp->usesLocalCoords();
    }

    for (int i = firstCoverageProcessorIdx; i < builder.numCoverageFragmentProcessors();
         ++i, ++currFPIdx) {
        const GrFragmentProcessor* fp = builder.getCoverageFragmentProcessor(i);
        pipeline->fFragmentProcessors[currFPIdx].reset(fp);
        usesLocalCoords = usesLocalCoords || fp->usesLocalCoords();
    }

    // Setup info we need to pass to GrPrimitiveProcessors that are used with this GrPipeline.
    overrides->fFlags = 0;
    if (!SkToBool(optFlags & GrXferProcessor::kIgnoreColor_OptFlag)) {
        overrides->fFlags |= GrXPOverridesForBatch::kReadsColor_Flag;
    }
    if (GrColor_ILLEGAL != overrideColor) {
        overrides->fFlags |= GrXPOverridesForBatch::kUseOverrideColor_Flag;
        overrides->fOverrideColor = overrideColor;
    }
    if (!SkToBool(optFlags & GrXferProcessor::kIgnoreCoverage_OptFlag)) {
        overrides->fFlags |= GrXPOverridesForBatch::kReadsCoverage_Flag;
    }
    if (usesLocalCoords) {
        overrides->fFlags |= GrXPOverridesForBatch::kReadsLocalCoords_Flag;
    }
    if (SkToBool(optFlags & GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag)) {
        overrides->fFlags |= GrXPOverridesForBatch::kCanTweakAlphaForCoverage_Flag; 
    }

    GrXPFactory::InvariantBlendedColor blendedColor;
    if (xpFactory) {
        xpFactory->getInvariantBlendedColor(args.fOpts.fColorPOI, &blendedColor);
    } else {
        GrPorterDuffXPFactory::SrcOverInvariantBlendedColor(args.fOpts.fColorPOI.color(),
                                                            args.fOpts.fColorPOI.validFlags(),
                                                            args.fOpts.fColorPOI.isOpaque(),
                                                            &blendedColor); 
    }
    if (blendedColor.fWillBlendWithDst) {
        overrides->fFlags |= GrXPOverridesForBatch::kWillColorBlendWithDst_Flag;
    }

    return pipeline;
}

static void add_dependencies_for_processor(const GrFragmentProcessor* proc, GrRenderTarget* rt) {
    for (int i = 0; i < proc->numChildProcessors(); ++i) {
        // need to recurse
        add_dependencies_for_processor(&proc->childProcessor(i), rt);
    }

    for (int i = 0; i < proc->numTextures(); ++i) {
        GrTexture* texture = proc->textureAccess(i).getTexture();
        SkASSERT(rt->getLastDrawTarget());
        rt->getLastDrawTarget()->addDependency(texture);
    }
}

void GrPipeline::addDependenciesTo(GrRenderTarget* rt) const {
    for (int i = 0; i < fFragmentProcessors.count(); ++i) {
        add_dependencies_for_processor(fFragmentProcessors[i].get(), rt);
    }

    const GrXferProcessor& xfer = this->getXferProcessor();

    for (int i = 0; i < xfer.numTextures(); ++i) {
        GrTexture* texture = xfer.textureAccess(i).getTexture();   
        SkASSERT(rt->getLastDrawTarget());
        rt->getLastDrawTarget()->addDependency(texture);
    }
}

void GrPipeline::adjustProgramFromOptimizations(const GrPipelineBuilder& pipelineBuilder,
                                                GrXferProcessor::OptFlags flags,
                                                const GrProcOptInfo& colorPOI,
                                                const GrProcOptInfo& coveragePOI,
                                                int* firstColorProcessorIdx,
                                                int* firstCoverageProcessorIdx) {
    fIgnoresCoverage = SkToBool(flags & GrXferProcessor::kIgnoreCoverage_OptFlag);
    fReadsFragPosition = this->getXferProcessor().willReadFragmentPosition();

    if ((flags & GrXferProcessor::kIgnoreColor_OptFlag) ||
        (flags & GrXferProcessor::kOverrideColor_OptFlag)) {
        *firstColorProcessorIdx = pipelineBuilder.numColorFragmentProcessors();
    } else {
        if (colorPOI.readsFragPosition()) {
            fReadsFragPosition = true;
        }
    }

    if (flags & GrXferProcessor::kIgnoreCoverage_OptFlag) {
        *firstCoverageProcessorIdx = pipelineBuilder.numCoverageFragmentProcessors();
    } else {
        if (coveragePOI.readsFragPosition()) {
            fReadsFragPosition = true;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrPipeline::AreEqual(const GrPipeline& a, const GrPipeline& b,
                          bool ignoreCoordTransforms) {
    SkASSERT(&a != &b);

    if (a.getRenderTarget() != b.getRenderTarget() ||
        a.fFragmentProcessors.count() != b.fFragmentProcessors.count() ||
        a.fNumColorProcessors != b.fNumColorProcessors ||
        a.fScissorState != b.fScissorState ||
        a.fFlags != b.fFlags ||
        a.fStencilSettings != b.fStencilSettings ||
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
        if (!a.getFragmentProcessor(i).isEqual(b.getFragmentProcessor(i), ignoreCoordTransforms)) {
            return false;
        }
    }
    return true;
}

