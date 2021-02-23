/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineState.h"

#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLGeometryProcessor.h"
#include "src/gpu/glsl/GrGLSLXferProcessor.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlTexture.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlPipelineState::SamplerBindings::SamplerBindings(GrSamplerState state,
                                                     GrTexture* texture,
                                                     GrMtlGpu* gpu)
        : fTexture(static_cast<GrMtlTexture*>(texture)->mtlTexture()) {
    fSampler = gpu->resourceProvider().findOrCreateCompatibleSampler(state);
}

GrMtlPipelineState::GrMtlPipelineState(
        GrMtlGpu* gpu,
        id<MTLRenderPipelineState> pipelineState,
        MTLPixelFormat pixelFormat,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        uint32_t uniformBufferSize,
        uint32_t numSamplers,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fpImpls)
        : fGpu(gpu)
        , fPipelineState(pipelineState)
        , fPixelFormat(pixelFormat)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fNumSamplers(numSamplers)
        , fGeometryProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFPImpls(std::move(fpImpls))
        , fDataManager(uniforms, uniformBufferSize) {
    (void) fPixelFormat; // Suppress unused-var warning.
}

void GrMtlPipelineState::setData(const GrRenderTarget* renderTarget,
                                 const GrProgramInfo& programInfo) {
    this->setRenderTargetState(renderTarget, programInfo.origin());
    fGeometryProcessor->setData(fDataManager, programInfo.primProc());

    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        auto& fp = programInfo.pipeline().getFragmentProcessor(i);
        for (auto [fp, impl] : GrGLSLFragmentProcessor::ParallelRange(fp, *fFPImpls[i])) {
            impl.setData(fDataManager, fp);
        }
    }

    {
        SkIPoint offset;
        GrTexture* dstTexture = programInfo.pipeline().peekDstTexture(&offset);
        fXferProcessor->setData(fDataManager, programInfo.pipeline().getXferProcessor(), dstTexture,
                                offset);
    }

    fDataManager.resetDirtyBits();

#ifdef SK_DEBUG
    if (programInfo.isStencilEnabled()) {
        SkASSERT(renderTarget->getStencilAttachment());
        SkASSERT(renderTarget->numStencilBits() == 8);
    }
#endif

    fStencil = programInfo.nonGLStencilSettings();
}

void GrMtlPipelineState::setTextures(const GrPrimitiveProcessor& primProc,
                                     const GrPipeline& pipeline,
                                     const GrSurfaceProxy* const primProcTextures[]) {
    fSamplerBindings.reset();
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        SkASSERT(primProcTextures[i]->asTextureProxy());
        const auto& sampler = primProc.textureSampler(i);
        auto texture = static_cast<GrMtlTexture*>(primProcTextures[i]->peekTexture());
        fSamplerBindings.emplace_back(sampler.samplerState(), texture, fGpu);
    }

    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        fSamplerBindings.emplace_back(te.samplerState(), te.texture(), fGpu);
    });

    if (GrTextureProxy* dstTextureProxy = pipeline.dstProxyView().asTextureProxy()) {
        fSamplerBindings.emplace_back(
                GrSamplerState::Filter::kNearest, dstTextureProxy->peekTexture(), fGpu);
    }

    SkASSERT(fNumSamplers == fSamplerBindings.count());
}

void GrMtlPipelineState::setDrawState(id<MTLRenderCommandEncoder> renderCmdEncoder,
                                      const GrSwizzle& writeSwizzle,
                                      const GrXferProcessor& xferProcessor) {
    [renderCmdEncoder pushDebugGroup:@"setDrawState"];
    this->bindUniforms(renderCmdEncoder);
    this->setBlendConstants(renderCmdEncoder, writeSwizzle, xferProcessor);
    this->setDepthStencilState(renderCmdEncoder);
    [renderCmdEncoder popDebugGroup];
}

void GrMtlPipelineState::bindUniforms(id<MTLRenderCommandEncoder> renderCmdEncoder) {
    fDataManager.uploadAndBindUniformBuffers(fGpu, renderCmdEncoder);
}

void GrMtlPipelineState::bindTextures(id<MTLRenderCommandEncoder> renderCmdEncoder) {
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

void GrMtlPipelineState::setBlendConstants(id<MTLRenderCommandEncoder> renderCmdEncoder,
                                           const GrSwizzle& swizzle,
                                           const GrXferProcessor& xferProcessor) {
    if (!renderCmdEncoder) {
        return;
    }

    const GrXferProcessor::BlendInfo& blendInfo = xferProcessor.getBlendInfo();
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    if (GrBlendCoeffRefsConstant(srcCoeff) || GrBlendCoeffRefsConstant(dstCoeff)) {
        // Swizzle the blend to match what the shader will output.
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
            if (@available(macOS 10.11, iOS 9.0, *)) {
                [renderCmdEncoder
                        setStencilFrontReferenceValue:fStencil.postOriginCCWFace(origin).fRef
                        backReferenceValue:fStencil.postOriginCWFace(origin).fRef];
            } else {
                // Two-sided stencil not supported on older versions of iOS
                // TODO: Find a way to recover from this
                SkASSERT(false);
            }
        } else {
            [renderCmdEncoder setStencilReferenceValue:fStencil.singleSidedFace().fRef];
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
