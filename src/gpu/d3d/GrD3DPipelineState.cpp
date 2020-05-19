/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DPipelineState.h"

#include "include/private/SkTemplates.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"

GrD3DPipelineState::GrD3DPipelineState(
        gr_cp<ID3D12PipelineState> pipelineState,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms, uint32_t uniformSize,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
        int fragmentProcessorCnt)
    : fPipelineState(std::move(pipelineState))
    , fBuiltinUniformHandles(builtinUniformHandles)
    , fGeometryProcessor(std::move(geometryProcessor))
    , fXferProcessor(std::move(xferProcessor))
    , fFragmentProcessors(std::move(fragmentProcessors))
    , fFragmentProcessorCnt(fragmentProcessorCnt)
    , fDataManager(uniforms, uniformSize) {}

void GrD3DPipelineState::setData(const GrRenderTarget* renderTarget,
                                 const GrProgramInfo& programInfo) {
    this->setRenderTargetState(renderTarget, programInfo.origin());

    GrFragmentProcessor::PipelineCoordTransformRange transformRange(programInfo.pipeline());
    fGeometryProcessor->setData(fDataManager, programInfo.primProc(), transformRange);
    GrFragmentProcessor::CIter fpIter(programInfo.pipeline());
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    for (; fpIter && glslIter; ++fpIter, ++glslIter) {
        glslIter->setData(fDataManager, *fpIter);
    }
    SkASSERT(!fpIter && !glslIter);

    {
        SkIPoint offset;
        GrTexture* dstTexture = programInfo.pipeline().peekDstTexture(&offset);

        fXferProcessor->setData(fDataManager, programInfo.pipeline().getXferProcessor(),
                                dstTexture, offset);
    }
}

void GrD3DPipelineState::setRenderTargetState(const GrRenderTarget* rt, GrSurfaceOrigin origin) {

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize dimensions = rt->dimensions();
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != dimensions) {
        fRenderTargetState.fRenderTargetSize = dimensions;
        fRenderTargetState.fRenderTargetOrigin = origin;

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}
