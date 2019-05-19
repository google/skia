/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineState.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/GrRenderTarget.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlTexture.h"

GrMtlPipelineState::SamplerBindings::SamplerBindings(const GrSamplerState& state,
                                                     GrTexture* texture,
                                                     GrMtlGpu* gpu)
        : fTexture(static_cast<GrMtlTexture*>(texture)->mtlTexture()) {
    uint32_t maxMipMapLevel = texture->texturePriv().maxMipMapLevel();
    fSampler = gpu->resourceProvider().findOrCreateCompatibleSampler(state, maxMipMapLevel);
}

GrMtlPipelineState::GrMtlPipelineState(
        GrMtlGpu* gpu,
        id<MTLRenderPipelineState> pipelineState,
        MTLPixelFormat pixelFormat,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        uint32_t geometryUniformBufferSize,
        uint32_t fragmentUniformBufferSize,
        uint32_t numSamplers,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
        int fragmentProcessorCnt)
        : fGpu(gpu)
        , fPipelineState(pipelineState)
        , fPixelFormat(pixelFormat)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fNumSamplers(numSamplers)
        , fGeometryProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFragmentProcessors(std::move(fragmentProcessors))
        , fFragmentProcessorCnt(fragmentProcessorCnt)
        , fDataManager(uniforms, geometryUniformBufferSize, fragmentUniformBufferSize) {
    (void) fPixelFormat; // Suppress unused-var warning.
}

void GrMtlPipelineState::setData(const GrRenderTarget* renderTarget,
                                 GrSurfaceOrigin origin,
                                 const GrPrimitiveProcessor& primProc,
                                 const GrPipeline& pipeline,
                                 const GrTextureProxy* const primProcTextures[]) {
    SkASSERT(primProcTextures || !primProc.numTextureSamplers());

    this->setRenderTargetState(renderTarget, origin);
    fGeometryProcessor->setData(fDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));
    fSamplerBindings.reset();
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        const auto& sampler = primProc.textureSampler(i);
        auto texture = static_cast<GrMtlTexture*>(primProcTextures[i]->peekTexture());
        fSamplerBindings.emplace_back(sampler.samplerState(), texture, fGpu);
    }

    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fDataManager, *fp);
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            const auto& sampler = fp->textureSampler(i);
            fSamplerBindings.emplace_back(sampler.samplerState(), sampler.peekTexture(), fGpu);
        }
        fp = iter.next();
        glslFP = glslIter.next();
    }
    SkASSERT(!fp && !glslFP);

    {
        SkIPoint offset;
        GrTexture* dstTexture = pipeline.peekDstTexture(&offset);

        fXferProcessor->setData(fDataManager, pipeline.getXferProcessor(), dstTexture, offset);
    }

    if (GrTextureProxy* dstTextureProxy = pipeline.dstTextureProxy()) {
        fSamplerBindings.emplace_back(GrSamplerState::ClampNearest(),
                                      dstTextureProxy->peekTexture(),
                                      fGpu);
    }

    SkASSERT(fNumSamplers == fSamplerBindings.count());
    fDataManager.resetDirtyBits();

    if (pipeline.isStencilEnabled()) {
        SkASSERT(renderTarget->renderTargetPriv().getStencilAttachment());
        fStencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(),
                       renderTarget->renderTargetPriv().numStencilBits());
    }
}

void GrMtlPipelineState::setDrawState(id<MTLRenderCommandEncoder> renderCmdEncoder,
                                      GrPixelConfig config, const GrXferProcessor& xferProcessor) {
    this->bind(renderCmdEncoder);
    this->setBlendConstants(renderCmdEncoder, config, xferProcessor);
    this->setDepthStencilState(renderCmdEncoder);
}

void GrMtlPipelineState::bind(id<MTLRenderCommandEncoder> renderCmdEncoder) {
    fDataManager.uploadAndBindUniformBuffers(fGpu, renderCmdEncoder);

    SkASSERT(fNumSamplers == fSamplerBindings.count());
    for (int index = 0; index < fNumSamplers; ++index) {
        [renderCmdEncoder setFragmentTexture: fSamplerBindings[index].fTexture
                                     atIndex: index];
        [renderCmdEncoder setFragmentSamplerState: fSamplerBindings[index].fSampler->mtlSampler()
                                          atIndex: index];
    }
}

void GrMtlPipelineState::setRenderTargetState(const GrRenderTarget* rt, GrSurfaceOrigin origin) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize size;
    size.set(rt->width(), rt->height());
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != size) {
        fRenderTargetState.fRenderTargetSize = size;
        fRenderTargetState.fRenderTargetOrigin = origin;

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

static bool blend_coeff_refs_constant(GrBlendCoeff coeff) {
    switch (coeff) {
        case kConstC_GrBlendCoeff:
        case kIConstC_GrBlendCoeff:
        case kConstA_GrBlendCoeff:
        case kIConstA_GrBlendCoeff:
            return true;
        default:
            return false;
    }
}

void GrMtlPipelineState::setBlendConstants(id<MTLRenderCommandEncoder> renderCmdEncoder,
                                           GrPixelConfig config,
                                           const GrXferProcessor& xferProcessor) {
    if (!renderCmdEncoder) {
        return;
    }

    GrXferProcessor::BlendInfo blendInfo;
    xferProcessor.getBlendInfo(&blendInfo);
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    if (blend_coeff_refs_constant(srcCoeff) || blend_coeff_refs_constant(dstCoeff)) {
        // Swizzle the blend to match what the shader will output.
        const GrSwizzle& swizzle = fGpu->caps()->shaderCaps()->configOutputSwizzle(config);
        SkPMColor4f blendConst = swizzle.applyTo(blendInfo.fBlendConstant);

        [renderCmdEncoder setBlendColorRed: blendConst.fR
                                     green: blendConst.fG
                                      blue: blendConst.fB
                                     alpha: blendConst.fA];
    }
}

void GrMtlPipelineState::setDepthStencilState(id<MTLRenderCommandEncoder> renderCmdEncoder) {
    const GrSurfaceOrigin& origin = fRenderTargetState.fRenderTargetOrigin;
    GrMtlDepthStencil* state =
            fGpu->resourceProvider().findOrCreateCompatibleDepthStencilState(fStencil, origin);
    if (!fStencil.isDisabled()) {
        if (fStencil.isTwoSided()) {
            [renderCmdEncoder setStencilFrontReferenceValue:fStencil.front(origin).fRef
                              backReferenceValue:fStencil.back(origin).fRef];
        }
        else {
            [renderCmdEncoder setStencilReferenceValue:fStencil.frontAndBack().fRef];
        }
    }
    [renderCmdEncoder setDepthStencilState:state->mtlDepthStencil()];
}

void GrMtlPipelineState::SetDynamicScissorRectState(id<MTLRenderCommandEncoder> renderCmdEncoder,
                                                    const GrRenderTarget* renderTarget,
                                                    GrSurfaceOrigin rtOrigin,
                                                    SkIRect scissorRect) {
    if (!scissorRect.intersect(SkIRect::MakeWH(renderTarget->width(), renderTarget->height()))) {
        scissorRect.setEmpty();
    }

    MTLScissorRect scissor;
    scissor.x = scissorRect.fLeft;
    scissor.width = scissorRect.width();
    if (kTopLeft_GrSurfaceOrigin == rtOrigin) {
        scissor.y = scissorRect.fTop;
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == rtOrigin);
        scissor.y = renderTarget->height() - scissorRect.fBottom;
    }
    scissor.height = scissorRect.height();

    SkASSERT(scissor.x >= 0);
    SkASSERT(scissor.y >= 0);

    [renderCmdEncoder setScissorRect: scissor];
}

bool GrMtlPipelineState::doesntSampleAttachment(
        const MTLRenderPassAttachmentDescriptor* attachment) const {
    for (int i = 0; i < fSamplerBindings.count(); ++i) {
        if (attachment.texture == fSamplerBindings[i].fTexture) {
            return false;
        }
    }
    return true;
}
