/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/gl/GrGLProgram.h"

#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/gl/GrGLBuffer.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"
#include "src/sksl/SkSLCompiler.h"

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrGLProgram> GrGLProgram::Make(
        GrGLGpu* gpu,
        const GrGLSLBuiltinUniformHandles& builtinUniforms,
        GrGLuint programID,
        const UniformInfoArray& uniforms,
        const UniformInfoArray& textureSamplers,
        std::unique_ptr<GrGLSLGeometryProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fpImpls,
        std::unique_ptr<Attribute[]> attributes,
        int vertexAttributeCnt,
        int instanceAttributeCnt,
        int vertexStride,
        int instanceStride) {
    sk_sp<GrGLProgram> program(new GrGLProgram(gpu,
                                               builtinUniforms,
                                               programID,
                                               uniforms,
                                               textureSamplers,
                                               std::move(geometryProcessor),
                                               std::move(xferProcessor),
                                               std::move(fpImpls),
                                               std::move(attributes),
                                               vertexAttributeCnt,
                                               instanceAttributeCnt,
                                               vertexStride,
                                               instanceStride));
    // Assign texture units to sampler uniforms one time up front.
    gpu->flushProgram(program);
    program->fProgramDataManager.setSamplerUniforms(textureSamplers, 0);
    return program;
}

GrGLProgram::GrGLProgram(GrGLGpu* gpu,
                         const GrGLSLBuiltinUniformHandles& builtinUniforms,
                         GrGLuint programID,
                         const UniformInfoArray& uniforms,
                         const UniformInfoArray& textureSamplers,
                         std::unique_ptr<GrGLSLGeometryProcessor> geometryProcessor,
                         std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
                         std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fpImpls,
                         std::unique_ptr<Attribute[]> attributes,
                         int vertexAttributeCnt,
                         int instanceAttributeCnt,
                         int vertexStride,
                         int instanceStride)
        : fBuiltinUniformHandles(builtinUniforms)
        , fProgramID(programID)
        , fGeometryProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFPImpls(std::move(fpImpls))
        , fAttributes(std::move(attributes))
        , fVertexAttributeCnt(vertexAttributeCnt)
        , fInstanceAttributeCnt(instanceAttributeCnt)
        , fVertexStride(vertexStride)
        , fInstanceStride(instanceStride)
        , fGpu(gpu)
        , fProgramDataManager(gpu, uniforms)
        , fNumTextureSamplers(textureSamplers.count()) {
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
    this->setRenderTargetState(renderTarget, programInfo.origin(), programInfo.geomProc());

    // we set the uniforms for installed processors in a generic way, but subclasses of GLProgram
    // determine how to set coord transforms

    // We must bind to texture units in the same order in which we set the uniforms in
    // GrGLProgramDataManager. That is, we bind textures for processors in this order:
    // primProc, fragProcs, XP.
    fGeometryProcessor->setData(fProgramDataManager,
                                *fGpu->caps()->shaderCaps(),
                                programInfo.geomProc());

    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        auto& fp = programInfo.pipeline().getFragmentProcessor(i);
        for (auto [fp, impl] : GrGLSLFragmentProcessor::ParallelRange(fp, *fFPImpls[i])) {
            impl.setData(fProgramDataManager, fp);
        }
    }

    const GrXferProcessor& xp = programInfo.pipeline().getXferProcessor();
    SkIPoint offset;
    GrTexture* dstTexture = programInfo.pipeline().peekDstTexture(&offset);

    fXferProcessor->setData(fProgramDataManager, xp, dstTexture, offset);
}

void GrGLProgram::bindTextures(const GrGeometryProcessor& geomProc,
                               const GrSurfaceProxy* const geomProcTextures[],
                               const GrPipeline& pipeline) {
    for (int i = 0; i < geomProc.numTextureSamplers(); ++i) {
        SkASSERT(geomProcTextures[i]->asTextureProxy());
        auto* overrideTexture = static_cast<GrGLTexture*>(geomProcTextures[i]->peekTexture());
        fGpu->bindTexture(i, geomProc.textureSampler(i).samplerState(),
                          geomProc.textureSampler(i).swizzle(), overrideTexture);
    }
    int nextTexSamplerIdx = geomProc.numTextureSamplers();

    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        GrSamplerState samplerState = te.samplerState();
        GrSwizzle swizzle = te.view().swizzle();
        auto* texture = static_cast<GrGLTexture*>(te.texture());
        fGpu->bindTexture(nextTexSamplerIdx++, samplerState, swizzle, texture);
    });

    SkIPoint offset;
    GrTexture* dstTexture = pipeline.peekDstTexture(&offset);
    if (dstTexture) {
        fGpu->bindTexture(nextTexSamplerIdx++, GrSamplerState::Filter::kNearest,
                          pipeline.dstProxyView().swizzle(), static_cast<GrGLTexture*>(dstTexture));
    }
    SkASSERT(nextTexSamplerIdx == fNumTextureSamplers);
}

void GrGLProgram::setRenderTargetState(const GrRenderTarget* rt,
                                       GrSurfaceOrigin origin,
                                       const GrGeometryProcessor& geomProc) {
    // Load the RT height uniform if it is needed
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fProgramDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize dimensions = rt->dimensions();
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != dimensions) {
        fRenderTargetState.fRenderTargetSize = dimensions;
        fRenderTargetState.fRenderTargetOrigin = origin;

        // The client will mark a swap buffer as kBottomLeft when making a SkSurface because
        // GL's framebuffer space has (0, 0) at the bottom left. In NDC (-1, -1) is also the
        // bottom left. However, Skia's device coords has (0, 0) at the top left, so a flip is
        // required when the origin is kBottomLeft.
        bool flip = (origin == kBottomLeft_GrSurfaceOrigin);
        std::array<float, 4> v = SkSL::Compiler::GetRTAdjustVector(dimensions, flip);
        fProgramDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, v.data());
    }
}
