/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGLProgram.h"
#include "GrAllocator.h"
#include "GrCoordTransform.h"
#include "GrGLBuffer.h"
#include "GrGLGpu.h"
#include "GrGLPathRendering.h"
#include "GrPathProcessor.h"
#include "GrPipeline.h"
#include "GrProcessor.h"
#include "GrTexturePriv.h"
#include "GrXferProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLXferProcessor.h"

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

void GrGLProgram::setData(const GrPrimitiveProcessor& primProc, const GrPipeline& pipeline) {
    this->setRenderTargetState(primProc, pipeline.proxy());

    // we set the textures, and uniforms for installed processors in a generic way, but subclasses
    // of GLProgram determine how to set coord transforms

    // We must bind to texture units in the same order in which we set the uniforms in
    // GrGLProgramDataManager. That is, we bind textures for processors in this order:
    // primProc, fragProcs, XP.
    int nextTexSamplerIdx = 0;
    fPrimitiveProcessor->setData(fProgramDataManager, primProc,
                                 GrFragmentProcessor::CoordTransformIter(pipeline));
    this->bindTextures(primProc, &nextTexSamplerIdx);

    this->setFragmentData(primProc, pipeline, &nextTexSamplerIdx);

    const GrXferProcessor& xp = pipeline.getXferProcessor();
    SkIPoint offset;
    GrTexture* dstTexture = pipeline.peekDstTexture(&offset);

    fXferProcessor->setData(fProgramDataManager, xp, dstTexture, offset);
    if (dstTexture) {
        fGpu->bindTexture(nextTexSamplerIdx++, GrSamplerState::ClampNearest(),
                          static_cast<GrGLTexture*>(dstTexture));
    }
    SkASSERT(nextTexSamplerIdx == fNumTextureSamplers);
}

void GrGLProgram::generateMipmaps(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline) {
    this->generateMipmaps(primProc);

    GrFragmentProcessor::Iter iter(pipeline);
    while (const GrFragmentProcessor* fp  = iter.next()) {
        this->generateMipmaps(*fp);
    }
}

void GrGLProgram::setFragmentData(const GrPrimitiveProcessor& primProc,
                                  const GrPipeline& pipeline,
                                  int* nextTexSamplerIdx) {
    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fProgramDataManager, *fp);
        this->bindTextures(*fp, nextTexSamplerIdx);
        fp = iter.next();
        glslFP = glslIter.next();
    }
    SkASSERT(!fp && !glslFP);
}


void GrGLProgram::setRenderTargetState(const GrPrimitiveProcessor& primProc,
                                       const GrRenderTargetProxy* proxy) {
    GrRenderTarget* rt = proxy->priv().peekRenderTarget();
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize size;
    size.set(rt->width(), rt->height());
    if (!primProc.isPathRendering()) {
        if (fRenderTargetState.fRenderTargetOrigin != proxy->origin() ||
            fRenderTargetState.fRenderTargetSize != size) {
            fRenderTargetState.fRenderTargetSize = size;
            fRenderTargetState.fRenderTargetOrigin = proxy->origin();

            float rtAdjustmentVec[4];
            fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
            fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
        }
    } else {
        SkASSERT(fGpu->glCaps().shaderCaps()->pathRenderingSupport());
        const GrPathProcessor& pathProc = primProc.cast<GrPathProcessor>();
        fGpu->glPathRendering()->setProjectionMatrix(pathProc.viewMatrix(),
                                                     size, proxy->origin());
    }
}

void GrGLProgram::bindTextures(const GrResourceIOProcessor& processor,
                               int* nextTexSamplerIdx) {
    for (int i = 0; i < processor.numTextureSamplers(); ++i) {
        const GrResourceIOProcessor::TextureSampler& sampler = processor.textureSampler(i);
        fGpu->bindTexture((*nextTexSamplerIdx)++, sampler.samplerState(),
                          static_cast<GrGLTexture*>(sampler.peekTexture()));
    }
}

void GrGLProgram::generateMipmaps(const GrResourceIOProcessor& processor) {
    for (int i = 0; i < processor.numTextureSamplers(); ++i) {
        const GrResourceIOProcessor::TextureSampler& sampler = processor.textureSampler(i);
        auto* tex = sampler.peekTexture();
        if (sampler.samplerState().filter() == GrSamplerState::Filter::kMipMap &&
            tex->texturePriv().mipMapped() == GrMipMapped::kYes &&
            tex->texturePriv().mipMapsAreDirty()) {
            SkASSERT(fGpu->caps()->mipMapSupport());
            fGpu->regenerateMipMapLevels(static_cast<GrGLTexture*>(sampler.peekTexture()));
        }
    }
}
