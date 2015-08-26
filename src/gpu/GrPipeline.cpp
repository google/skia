/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPipeline.h"

#include "GrCaps.h"
#include "GrGpu.h"
#include "GrPipelineBuilder.h"
#include "GrProcOptInfo.h"
#include "GrXferProcessor.h"

#include "batches/GrBatch.h"

GrPipeline* GrPipeline::CreateAt(void* memory, const CreateArgs& args,
                                 GrPipelineOptimizations* opts) {
    const GrPipelineBuilder& builder = *args.fPipelineBuilder;

    // Create XferProcessor from DS's XPFactory
    SkAutoTUnref<GrXferProcessor> xferProcessor(
        builder.getXPFactory()->createXferProcessor(args.fColorPOI, args.fCoveragePOI,
                                                    builder.hasMixedSamples(), &args.fDstTexture,
                                                    *args.fCaps));
    if (!xferProcessor) {
        return nullptr;
    }

    GrColor overrideColor = GrColor_ILLEGAL;
    if (args.fColorPOI.firstEffectiveProcessorIndex() != 0) {
        overrideColor = args.fColorPOI.inputColorToFirstEffectiveProccesor();
    }

    GrXferProcessor::OptFlags optFlags = GrXferProcessor::kNone_OptFlags;

    optFlags = xferProcessor->getOptimizations(args.fColorPOI,
                                                args.fCoveragePOI,
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

    GrPipeline* pipeline = SkNEW_PLACEMENT(memory, GrPipeline);
    pipeline->fXferProcessor.reset(xferProcessor.get());

    pipeline->fRenderTarget.reset(builder.fRenderTarget.get());
    SkASSERT(pipeline->fRenderTarget);
    pipeline->fScissorState = *args.fScissor;
    pipeline->fStencilSettings = builder.getStencil();
    pipeline->fDrawFace = builder.getDrawFace();

    pipeline->fFlags = 0;
    if (builder.isHWAntialias()) {
        pipeline->fFlags |= kHWAA_Flag;
    }
    if (builder.isDither()) {
        pipeline->fFlags |= kDither_Flag;
    }
    if (builder.snapVerticesToPixelCenters()) {
        pipeline->fFlags |= kSnapVertices_Flag;
    }

    int firstColorProcessorIdx = args.fColorPOI.firstEffectiveProcessorIndex();

    // TODO: Once we can handle single or four channel input into coverage GrFragmentProcessors
    // then we can use GrPipelineBuilder's coverageProcInfo (like color above) to set this initial
    // information.
    int firstCoverageProcessorIdx = 0;

    pipeline->adjustProgramFromOptimizations(builder, optFlags, args.fColorPOI, args.fCoveragePOI,
                                             &firstColorProcessorIdx, &firstCoverageProcessorIdx);

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
        fp->gatherCoordTransforms(&pipeline->fCoordTransforms);
    }

    for (int i = firstCoverageProcessorIdx; i < builder.numCoverageFragmentProcessors();
         ++i, ++currFPIdx) {
        const GrFragmentProcessor* fp = builder.getCoverageFragmentProcessor(i);
        pipeline->fFragmentProcessors[currFPIdx].reset(fp);
        usesLocalCoords = usesLocalCoords || fp->usesLocalCoords();
        fp->gatherCoordTransforms(&pipeline->fCoordTransforms);
    }

    // Setup info we need to pass to GrPrimitiveProcessors that are used with this GrPipeline.
    opts->fFlags = 0;
    if (!SkToBool(optFlags & GrXferProcessor::kIgnoreColor_OptFlag)) {
        opts->fFlags |= GrPipelineOptimizations::kReadsColor_Flag;
    }
    if (GrColor_ILLEGAL != overrideColor) {
        opts->fFlags |= GrPipelineOptimizations::kUseOverrideColor_Flag;
        opts->fOverrideColor = overrideColor;
    }
    if (!SkToBool(optFlags & GrXferProcessor::kIgnoreCoverage_OptFlag)) {
        opts->fFlags |= GrPipelineOptimizations::kReadsCoverage_Flag;
    }
    if (usesLocalCoords) {
        opts->fFlags |= GrPipelineOptimizations::kReadsLocalCoords_Flag;
    }
    if (SkToBool(optFlags & GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag)) {
        opts->fFlags |= GrPipelineOptimizations::kCanTweakAlphaForCoverage_Flag; 
    }

    GrXPFactory::InvariantBlendedColor blendedColor;
    builder.fXPFactory->getInvariantBlendedColor(args.fColorPOI, &blendedColor);
    if (blendedColor.fWillBlendWithDst) {
        opts->fFlags |= GrPipelineOptimizations::kWillColorBlendWithDst_Flag;
    }

    return pipeline;
}

void GrPipeline::adjustProgramFromOptimizations(const GrPipelineBuilder& pipelineBuilder,
                                                GrXferProcessor::OptFlags flags,
                                                const GrProcOptInfo& colorPOI,
                                                const GrProcOptInfo& coveragePOI,
                                                int* firstColorProcessorIdx,
                                                int* firstCoverageProcessorIdx) {
    fReadsFragPosition = fXferProcessor->willReadFragmentPosition();

    if ((flags & GrXferProcessor::kIgnoreColor_OptFlag) ||
        (flags & GrXferProcessor::kOverrideColor_OptFlag)) {
        *firstColorProcessorIdx = pipelineBuilder.numColorFragmentProcessors();
    } else {
        if (coveragePOI.readsFragPosition()) {
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

    if (!a.getXferProcessor()->isEqual(*b.getXferProcessor())) {
        return false;
    }

    for (int i = 0; i < a.numFragmentProcessors(); i++) {
        if (!a.getFragmentProcessor(i).isEqual(b.getFragmentProcessor(i), ignoreCoordTransforms)) {
            return false;
        }
    }
    return true;
}

