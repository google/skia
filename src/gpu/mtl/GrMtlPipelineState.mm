/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineState.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"
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
        , fDataManager(uniforms, uniformBufferSize) {
    (void) fPixelFormat; // Suppress unused-var warning.
}

void GrMtlPipelineState::setData(const GrRenderTarget* renderTarget,
                                 const GrProgramInfo& programInfo) {
    this->setRenderTargetState(renderTarget, programInfo.origin());
    GrFragmentProcessor::PipelineCoordTransformRange transformRange(programInfo.pipeline());
    fGeometryProcessor->setData(fDataManager, programInfo.primProc(), transformRange);

    if (!programInfo.hasDynamicPrimProcTextures()) {
        auto proxies = programInfo.hasFixedPrimProcTextures() ? programInfo.fixedPrimProcTextures()
                                                              : nullptr;
        this->setTextures(programInfo, proxies);
    }
    fDataManager.resetDirtyBits();

#ifdef SK_DEBUG
    if (programInfo.pipeline().isStencilEnabled()) {
        SkASSERT(renderTarget->renderTargetPriv().getStencilAttachment());
        SkASSERT(renderTarget->renderTargetPriv().numStencilBits() == 8);
    }
#endif

    fStencil = programInfo.nonGLStencilSettings();
}

void GrMtlPipelineState::setTextures(const GrProgramInfo& programInfo,
                                     const GrSurfaceProxy* const primProcTextures[]) {
    fSamplerBindings.reset();
    for (int i = 0; i < programInfo.primProc().numTextureSamplers(); ++i) {
        SkASSERT(primProcTextures[i]->asTextureProxy());
        const auto& sampler = programInfo.primProc().textureSampler(i);
        auto texture = static_cast<GrMtlTexture*>(primProcTextures[i]->peekTexture());
        fSamplerBindings.emplace_back(sampler.samplerState(), texture, fGpu);
    }

    GrFragmentProcessor::CIter fpIter(programInfo.pipeline());
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    for (; fpIter && glslIter; ++fpIter, ++glslIter) {
        glslIter->setData(fDataManager, *fpIter);
        for (int i = 0; i < fpIter->numTextureSamplers(); ++i) {
            const auto& sampler = fpIter->textureSampler(i);
            fSamplerBindings.emplace_back(sampler.samplerState(), sampler.peekTexture(), fGpu);
        }
    }
    SkASSERT(!fpIter && !glslIter);

    {
        SkIPoint offset;
        GrTexture* dstTexture = programInfo.pipeline().peekDstTexture(&offset);

        fXferProcessor->setData(fDataManager, programInfo.pipeline().getXferProcessor(),
                                dstTexture, offset);
    }

    if (GrTextureProxy* dstTextureProxy = programInfo.pipeline().dstProxyView().asTextureProxy()) {
        fSamplerBindings.emplace_back(
                GrSamplerState::Filter::kNearest, dstTextureProxy->peekTexture(), fGpu);
    }

    SkASSERT(fNumSamplers == fSamplerBindings.count());
}

void GrMtlPipelineState::setDrawState(id<MTLRenderCommandEncoder> renderCmdEncoder,
                                      const GrSwizzle& outputSwizzle,
                                      const GrXferProcessor& xferProcessor) {
    [renderCmdEncoder pushDebugGroup:@"setDrawState"];
    this->bindUniforms(renderCmdEncoder);
    this->setBlendConstants(renderCmdEncoder, outputSwizzle, xferProcessor);
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
                                           const GrSwizzle& swizzle,
                                           const GrXferProcessor& xferProcessor) {
    if (!renderCmdEncoder) {
        return;
    }

    const GrXferProcessor::BlendInfo& blendInfo = xferProcessor.getBlendInfo();
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    if (blend_coeff_refs_constant(srcCoeff) || blend_coeff_refs_constant(dstCoeff)) {
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
