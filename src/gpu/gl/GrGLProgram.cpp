/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrPathProcessor.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrTAllocator.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/gl/GrGLBuffer.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLPathRendering.h"
#include "src/gpu/gl/GrGLProgram.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

///////////////////////////////////////////////////////////////////////////////////////////////////

GrGLProgram::GrGLProgram(
        GrGLGpu* gpu,
        const GrGLSLBuiltinUniformHandles& builtinUniforms,
        GrGLuint programID,
        const UniformInfoArray& uniforms,
        const UniformInfoArray& textureSamplers,
        const VaryingInfoArray& pathProcVaryings,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
        int fragmentProcessorCnt,
        std::unique_ptr<Attribute[]> attributes,
        int vertexAttributeCnt,
        int instanceAttributeCnt,
        int vertexStride,
        int instanceStride)
        : fBuiltinUniformHandles(builtinUniforms)
        , fProgramID(programID)
        , fPrimitiveProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFragmentProcessors(std::move(fragmentProcessors))
        , fFragmentProcessorCnt(fragmentProcessorCnt)
        , fAttributes(std::move(attributes))
        , fVertexAttributeCnt(vertexAttributeCnt)
        , fInstanceAttributeCnt(instanceAttributeCnt)
        , fVertexStride(vertexStride)
        , fInstanceStride(instanceStride)
        , fGpu(gpu)
        , fProgramDataManager(gpu, programID, uniforms, pathProcVaryings)
        , fNumTextureSamplers(textureSamplers.count()) {
    // Assign texture units to sampler uniforms one time up front.
    GL_CALL(UseProgram(fProgramID));
    fProgramDataManager.setSamplerUniforms(textureSamplers, 0);
}

GrGLProgram::~GrGLProgram() {
    if (fProgramID) {
        GL_CALL(DeleteProgram(fProgramID));
    }
}

void GrGLProgram::abandon() {
    fProgramID = 0;
}

///////////////////////////////////////////////////////////////////////////////

void GrGLProgram::updateUniforms(const GrRenderTarget* renderTarget,
                                 const GrProgramInfo& programInfo) {
    this->setRenderTargetState(renderTarget, programInfo.origin(), programInfo.primProc());

    // we set the uniforms for installed processors in a generic way, but subclasses of GLProgram
    // determine how to set coord transforms

    // We must bind to texture units in the same order in which we set the uniforms in
    // GrGLProgramDataManager. That is, we bind textures for processors in this order:
    // primProc, fragProcs, XP.
    GrFragmentProcessor::PipelineCoordTransformRange range(programInfo.pipeline());
    fPrimitiveProcessor->setData(fProgramDataManager, programInfo.primProc(), range);

    GrFragmentProcessor::CIter fpIter(programInfo.pipeline());
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    for (; fpIter && glslIter; ++fpIter, ++glslIter) {
        glslIter->setData(fProgramDataManager, *fpIter);
    }
    SkASSERT(!fpIter && !glslIter);

    const GrXferProcessor& xp = programInfo.pipeline().getXferProcessor();
    SkIPoint offset;
    GrTexture* dstTexture = programInfo.pipeline().peekDstTexture(&offset);

    fXferProcessor->setData(fProgramDataManager, xp, dstTexture, offset);
}

void GrGLProgram::bindTextures(const GrPrimitiveProcessor& primProc,
                               const GrSurfaceProxy* const primProcTextures[],
                               const GrPipeline& pipeline) {
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        SkASSERT(primProcTextures[i]->asTextureProxy());
        auto* overrideTexture = static_cast<GrGLTexture*>(primProcTextures[i]->peekTexture());
        fGpu->bindTexture(i, primProc.textureSampler(i).samplerState(),
                          primProc.textureSampler(i).swizzle(), overrideTexture);
    }
    int nextTexSamplerIdx = primProc.numTextureSamplers();

    GrFragmentProcessor::CIter fpIter(pipeline);
    for (; fpIter; ++fpIter) {
        for (int i = 0; i < fpIter->numTextureSamplers(); ++i) {
            const GrFragmentProcessor::TextureSampler& sampler = fpIter->textureSampler(i);
            fGpu->bindTexture(nextTexSamplerIdx++, sampler.samplerState(), sampler.view().swizzle(),
                              static_cast<GrGLTexture*>(sampler.peekTexture()));
        }
    }

    SkIPoint offset;
    GrTexture* dstTexture = pipeline.peekDstTexture(&offset);
    if (dstTexture) {
        fGpu->bindTexture(nextTexSamplerIdx++, GrSamplerState::Filter::kNearest,
                          pipeline.dstProxyView().swizzle(), static_cast<GrGLTexture*>(dstTexture));
    }
    SkASSERT(nextTexSamplerIdx == fNumTextureSamplers);
}

void GrGLProgram::setRenderTargetState(const GrRenderTarget* rt, GrSurfaceOrigin origin,
                                       const GrPrimitiveProcessor& primProc) {
    // Load the RT size uniforms if they are needed
    if (fBuiltinUniformHandles.fRTWidthUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fWidth != rt->width()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTWidthUni, SkIntToScalar(rt->width()));
    }
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize dimensions = rt->dimensions();
    if (!primProc.isPathRendering()) {
        if (fRenderTargetState.fRenderTargetOrigin != origin ||
            fRenderTargetState.fRenderTargetSize != dimensions) {
            fRenderTargetState.fRenderTargetSize = dimensions;
            fRenderTargetState.fRenderTargetOrigin = origin;

            float rtAdjustmentVec[4];
            fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
            fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
        }
    } else {
        SkASSERT(fGpu->glCaps().shaderCaps()->pathRenderingSupport());
        const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
        fGpu->glPathRendering()->setProjectionMatrix(pathProc.viewMatrix(), dimensions, origin);
    }
}
