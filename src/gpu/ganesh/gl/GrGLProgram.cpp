/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/gl/GrGLProgram.h"

#include "include/core/SkSamplingOptions.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/gl/GrGLFunctions.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/gl/GrGLGpu.h"
#include "src/gpu/ganesh/gl/GrGLTexture.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "src/sksl/SkSLCompiler.h"

#include <array>
#include <functional>
#include <utility>

class GrTexture;

#define GL_CALL(X) GR_GL_CALL(fGpu->glInterface(), X)
#define GL_CALL_RET(R, X) GR_GL_CALL_RET(fGpu->glInterface(), R, X)

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrGLProgram> GrGLProgram::Make(
        GrGLGpu* gpu,
        const GrGLSLBuiltinUniformHandles& builtinUniforms,
        GrGLuint programID,
        const UniformInfoArray& uniforms,
        const UniformInfoArray& textureSamplers,
        std::unique_ptr<GrGeometryProcessor::ProgramImpl> gpImpl,
        std::unique_ptr<GrXferProcessor::ProgramImpl> xpImpl,
        std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls,
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
                                               std::move(gpImpl),
                                               std::move(xpImpl),
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
                         std::unique_ptr<GrGeometryProcessor::ProgramImpl> gpImpl,
                         std::unique_ptr<GrXferProcessor::ProgramImpl> xpImpl,
                         std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls,
                         std::unique_ptr<Attribute[]> attributes,
                         int vertexAttributeCnt,
                         int instanceAttributeCnt,
                         int vertexStride,
                         int instanceStride)
        : fBuiltinUniformHandles(builtinUniforms)
        , fProgramID(programID)
        , fGPImpl(std::move(gpImpl))
        , fXPImpl(std::move(xpImpl))
        , fFPImpls(std::move(fpImpls))
        , fAttributes(std::move(attributes))
        , fVertexAttributeCnt(vertexAttributeCnt)
        , fInstanceAttributeCnt(instanceAttributeCnt)
        , fVertexStride(vertexStride)
        , fInstanceStride(instanceStride)
        , fGpu(gpu)
        , fProgramDataManager(gpu, uniforms)
        , fNumTextureSamplers(textureSamplers.count()) {}

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
    fGPImpl->setData(fProgramDataManager, *fGpu->caps()->shaderCaps(), programInfo.geomProc());

    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        const auto& fp = programInfo.pipeline().getFragmentProcessor(i);
        fp.visitWithImpls([&](const GrFragmentProcessor& fp,
                              GrFragmentProcessor::ProgramImpl& impl) {
            impl.setData(fProgramDataManager, fp);
        }, *fFPImpls[i]);
    }

    programInfo.pipeline().setDstTextureUniforms(fProgramDataManager, &fBuiltinUniformHandles);
    fXPImpl->setData(fProgramDataManager, programInfo.pipeline().getXferProcessor());
}

void GrGLProgram::bindTextures(const GrGeometryProcessor& geomProc,
                               const GrSurfaceProxy* const geomProcTextures[],
                               const GrPipeline& pipeline) {
    // Bind textures from the geometry processor.
    for (int i = 0; i < geomProc.numTextureSamplers(); ++i) {
        SkASSERT(geomProcTextures[i]->asTextureProxy());
        auto* overrideTexture = static_cast<GrGLTexture*>(geomProcTextures[i]->peekTexture());
        fGpu->bindTexture(i, geomProc.textureSampler(i).samplerState(),
                          geomProc.textureSampler(i).swizzle(), overrideTexture);
    }
    int nextTexSamplerIdx = geomProc.numTextureSamplers();
    // Bind texture from the destination proxy view.
    GrTexture* dstTexture = pipeline.peekDstTexture();
    if (dstTexture) {
        fGpu->bindTexture(nextTexSamplerIdx++, GrSamplerState::Filter::kNearest,
                          pipeline.dstProxyView().swizzle(), static_cast<GrGLTexture*>(dstTexture));
    }
    // Bind textures from all of the fragment processors.
    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        GrSamplerState samplerState = te.samplerState();
        skgpu::Swizzle swizzle = te.view().swizzle();
        auto* texture = static_cast<GrGLTexture*>(te.texture());
        fGpu->bindTexture(nextTexSamplerIdx++, samplerState, swizzle, texture);
    });

    SkASSERT(nextTexSamplerIdx == fNumTextureSamplers);
}

void GrGLProgram::setRenderTargetState(const GrRenderTarget* rt,
                                       GrSurfaceOrigin origin,
                                       const GrGeometryProcessor& geomProc) {
    // Set RT adjustment and RT flip
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
        if (fBuiltinUniformHandles.fRTFlipUni.isValid()) {
            std::array<float, 2> d = SkSL::Compiler::GetRTFlipVector(dimensions.height(), flip);
            fProgramDataManager.set2fv(fBuiltinUniformHandles.fRTFlipUni, 1, d.data());
        }
    }
}
