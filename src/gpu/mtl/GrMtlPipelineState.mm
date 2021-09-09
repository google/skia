/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineState.h"

#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlFramebuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlRenderCommandEncoder.h"
#include "src/gpu/mtl/GrMtlTexture.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlPipelineState::SamplerBindings::SamplerBindings(GrSamplerState state,
                                                     GrTexture* texture,
                                                     GrMtlGpu* gpu)
        : fTexture(static_cast<GrMtlTexture*>(texture)->mtlTexture()) {
    fSampler = gpu->resourceProvider().findOrCreateCompatibleSampler(state);
    gpu->commandBuffer()->addResource(sk_ref_sp<GrManagedResource>(fSampler));
    gpu->commandBuffer()->addGrSurface(
            sk_ref_sp<GrSurface>(static_cast<GrMtlTexture*>(texture)->attachment()));
}

GrMtlPipelineState::GrMtlPipelineState(
        GrMtlGpu* gpu,
        sk_sp<GrMtlRenderPipeline> pipeline,
        MTLPixelFormat pixelFormat,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        uint32_t uniformBufferSize,
        uint32_t numSamplers,
        std::unique_ptr<GrGeometryProcessor::ProgramImpl> gpImpl,
        std::unique_ptr<GrXferProcessor::ProgramImpl> xpImpl,
        std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls)
        : fGpu(gpu)
        , fPipeline(std::move(pipeline))
        , fPixelFormat(pixelFormat)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fNumSamplers(numSamplers)
        , fGPImpl(std::move(gpImpl))
        , fXPImpl(std::move(xpImpl))
        , fFPImpls(std::move(fpImpls))
        , fDataManager(uniforms, uniformBufferSize) {
    (void) fPixelFormat; // Suppress unused-var warning.
}

void GrMtlPipelineState::setData(GrMtlFramebuffer* framebuffer,
                                 const GrProgramInfo& programInfo) {
    SkISize colorAttachmentDimensions = framebuffer->colorAttachment()->dimensions();

    this->setRenderTargetState(colorAttachmentDimensions, programInfo.origin());
    fGPImpl->setData(fDataManager, *fGpu->caps()->shaderCaps(), programInfo.geomProc());

    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        const auto& fp = programInfo.pipeline().getFragmentProcessor(i);
        fp.visitWithImpls([&](const GrFragmentProcessor& fp,
                              GrFragmentProcessor::ProgramImpl& impl) {
            impl.setData(fDataManager, fp);
        }, *fFPImpls[i]);
    }

    programInfo.pipeline().setDstTextureUniforms(fDataManager, &fBuiltinUniformHandles);
    fXPImpl->setData(fDataManager, programInfo.pipeline().getXferProcessor());

    fDataManager.resetDirtyBits();

#ifdef SK_DEBUG
    if (programInfo.isStencilEnabled()) {
        SkDEBUGCODE(const GrAttachment* stencil = framebuffer->stencilAttachment());
        SkASSERT(stencil);
        SkASSERT(GrBackendFormatStencilBits(stencil->backendFormat()) == 8);
    }
#endif

    fStencil = programInfo.nonGLStencilSettings();
    fGpu->commandBuffer()->addResource(fPipeline);
}

void GrMtlPipelineState::setTextures(const GrGeometryProcessor& geomProc,
                                     const GrPipeline& pipeline,
                                     const GrSurfaceProxy* const geomProcTextures[]) {
    fSamplerBindings.reset();
    for (int i = 0; i < geomProc.numTextureSamplers(); ++i) {
        SkASSERT(geomProcTextures[i]->asTextureProxy());
        const auto& sampler = geomProc.textureSampler(i);
        auto texture = static_cast<GrMtlTexture*>(geomProcTextures[i]->peekTexture());
        fSamplerBindings.emplace_back(sampler.samplerState(), texture, fGpu);
    }

    if (GrTextureProxy* dstTextureProxy = pipeline.dstProxyView().asTextureProxy()) {
        fSamplerBindings.emplace_back(
                GrSamplerState::Filter::kNearest, dstTextureProxy->peekTexture(), fGpu);
    }

    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        fSamplerBindings.emplace_back(te.samplerState(), te.texture(), fGpu);
    });

    SkASSERT(fNumSamplers == fSamplerBindings.count());
}

void GrMtlPipelineState::setDrawState(GrMtlRenderCommandEncoder* renderCmdEncoder,
                                      const GrSwizzle& writeSwizzle,
                                      const GrXferProcessor& xferProcessor) {
    this->bindUniforms(renderCmdEncoder);
    this->setBlendConstants(renderCmdEncoder, writeSwizzle, xferProcessor);
    this->setDepthStencilState(renderCmdEncoder);
}

void GrMtlPipelineState::bindUniforms(GrMtlRenderCommandEncoder* renderCmdEncoder) {
    fDataManager.uploadAndBindUniformBuffers(fGpu, renderCmdEncoder);
}

void GrMtlPipelineState::bindTextures(GrMtlRenderCommandEncoder* renderCmdEncoder) {
    SkASSERT(fNumSamplers == fSamplerBindings.count());
    for (int index = 0; index < fNumSamplers; ++index) {
        renderCmdEncoder->setFragmentTexture(fSamplerBindings[index].fTexture, index);
        renderCmdEncoder->setFragmentSamplerState(fSamplerBindings[index].fSampler, index);
    }
}

void GrMtlPipelineState::setRenderTargetState(SkISize colorAttachmentDimensions,
                                              GrSurfaceOrigin origin) {
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != colorAttachmentDimensions) {
        fRenderTargetState.fRenderTargetSize = colorAttachmentDimensions;
        fRenderTargetState.fRenderTargetOrigin = origin;

        // The client will mark a swap buffer as kTopLeft when making a SkSurface because
        // Metal's framebuffer space has (0, 0) at the top left. This agrees with Skia's device
        // coords. However, in NDC (-1, -1) is the bottom left. So we flip when origin is kTopLeft.
        bool flip = (origin == kTopLeft_GrSurfaceOrigin);
        std::array<float, 4> v = SkSL::Compiler::GetRTAdjustVector(colorAttachmentDimensions, flip);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, v.data());
        if (fBuiltinUniformHandles.fRTFlipUni.isValid()) {
            // Note above that framebuffer space has origin top left. So we need !flip here.
            std::array<float, 2> d =
                    SkSL::Compiler::GetRTFlipVector(colorAttachmentDimensions.height(), !flip);
            fDataManager.set2fv(fBuiltinUniformHandles.fRTFlipUni, 1, d.data());
        }
    }
}

void GrMtlPipelineState::setBlendConstants(GrMtlRenderCommandEncoder* renderCmdEncoder,
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

        renderCmdEncoder->setBlendColor(blendConst);
    }
}

void GrMtlPipelineState::setDepthStencilState(GrMtlRenderCommandEncoder* renderCmdEncoder) {
    const GrSurfaceOrigin& origin = fRenderTargetState.fRenderTargetOrigin;
    GrMtlDepthStencil* state =
            fGpu->resourceProvider().findOrCreateCompatibleDepthStencilState(fStencil, origin);
    if (!fStencil.isDisabled()) {
        if (fStencil.isTwoSided()) {
            if (@available(macOS 10.11, iOS 9.0, *)) {
                renderCmdEncoder->setStencilFrontBackReferenceValues(
                        fStencil.postOriginCCWFace(origin).fRef,
                        fStencil.postOriginCWFace(origin).fRef);
            } else {
                // Two-sided stencil not supported on older versions of iOS
                // TODO: Find a way to recover from this
                SkASSERT(false);
            }
        } else {
            renderCmdEncoder->setStencilReferenceValue(fStencil.singleSidedFace().fRef);
        }
    }
    renderCmdEncoder->setDepthStencilState(state->mtlDepthStencil());
    fGpu->commandBuffer()->addResource(sk_ref_sp<GrManagedResource>(state));
}

void GrMtlPipelineState::SetDynamicScissorRectState(GrMtlRenderCommandEncoder* renderCmdEncoder,
                                                    SkISize colorAttachmentDimensions,
                                                    GrSurfaceOrigin rtOrigin,
                                                    SkIRect scissorRect) {
    if (!scissorRect.intersect(SkIRect::MakeWH(colorAttachmentDimensions.width(),
                                               colorAttachmentDimensions.height()))) {
        scissorRect.setEmpty();
    }

    MTLScissorRect scissor;
    scissor.x = scissorRect.fLeft;
    scissor.width = scissorRect.width();
    if (kTopLeft_GrSurfaceOrigin == rtOrigin) {
        scissor.y = scissorRect.fTop;
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == rtOrigin);
        scissor.y = colorAttachmentDimensions.height() - scissorRect.fBottom;
    }
    scissor.height = scissorRect.height();

    SkASSERT(scissor.x >= 0);
    SkASSERT(scissor.y >= 0);

    renderCmdEncoder->setScissorRect(scissor);
}

bool GrMtlPipelineState::doesntSampleAttachment(
        const MTLRenderPassAttachmentDescriptor* attachment) const {
    for (int i = 0; i < fSamplerBindings.count(); ++i) {
        if (attachment.texture == fSamplerBindings[i].fTexture ||
            attachment.resolveTexture == fSamplerBindings[i].fTexture) {
            return false;
        }
    }
    return true;
}

GR_NORETAIN_END
