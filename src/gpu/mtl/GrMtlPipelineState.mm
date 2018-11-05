/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlPipelineState.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrPipeline.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrTexturePriv.h"
#include "GrMtlBuffer.h"
#include "GrMtlGpu.h"
#include "GrMtlSampler.h"
#include "GrMtlTexture.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLGeometryProcessor.h"
#include "glsl/GrGLSLXferProcessor.h"

GrMtlPipelineState::SamplerBindings::SamplerBindings(const GrSamplerState& state,
                                                     GrTexture* texture,
                                                     GrMtlGpu* gpu)
        : fTexture(static_cast<GrMtlTexture*>(texture)->mtlTexture()) {
    // TODO: use resource provider to get sampler.
    std::unique_ptr<GrMtlSampler> sampler(
            GrMtlSampler::Create(gpu, state, texture->texturePriv().maxMipMapLevel()));
    fSampler = sampler->mtlSamplerState();
}

GrMtlPipelineState::GrMtlPipelineState(
        GrMtlGpu* gpu,
        id<MTLRenderPipelineState> pipelineState,
        MTLPixelFormat pixelFormat,
        const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
        const UniformInfoArray& uniforms,
        GrMtlBuffer* geometryUniformBuffer,
        GrMtlBuffer* fragmentUniformBuffer,
        uint32_t numSamplers,
        std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
        std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
        std::unique_ptr<std::unique_ptr<GrGLSLFragmentProcessor>[]> fragmentProcessors,
        int fragmentProcessorCnt)
        : fGpu(gpu)
        , fPipelineState(pipelineState)
        , fPixelFormat(pixelFormat)
        , fBuiltinUniformHandles(builtinUniformHandles)
        , fGeometryUniformBuffer(geometryUniformBuffer)
        , fFragmentUniformBuffer(fragmentUniformBuffer)
        , fNumSamplers(numSamplers)
        , fGeometryProcessor(std::move(geometryProcessor))
        , fXferProcessor(std::move(xferProcessor))
        , fFragmentProcessors(std::move(fragmentProcessors))
        , fFragmentProcessorCnt(fragmentProcessorCnt)
        , fDataManager(uniforms, geometryUniformBuffer->sizeInBytes(),
                       fragmentUniformBuffer->sizeInBytes()) {
    (void) fPixelFormat; // Suppress unused-var warning.
}

void GrMtlPipelineState::setData(const GrPrimitiveProcessor& primProc,
                                 const GrPipeline& pipeline,
                                 const GrTextureProxy* const primProcTextures[]) {
    SkASSERT(primProcTextures || !primProc.numTextureSamplers());

    this->setRenderTargetState(pipeline.proxy());
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
    if (fGeometryUniformBuffer || fFragmentUniformBuffer) {
        fDataManager.uploadUniformBuffers(fGpu, fGeometryUniformBuffer.get(),
                                          fFragmentUniformBuffer.get());
    }

    if (pipeline.isStencilEnabled()) {
        GrRenderTarget* rt = pipeline.renderTarget();
        SkASSERT(rt->renderTargetPriv().getStencilAttachment());
        fStencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(),
                       rt->renderTargetPriv().numStencilBits());
    }
}

void GrMtlPipelineState::bind(id<MTLRenderCommandEncoder> renderCmdEncoder) {
    if (fGeometryUniformBuffer) {
        [renderCmdEncoder setVertexBuffer: fGeometryUniformBuffer->mtlBuffer()
                                   offset: 0
                                  atIndex: GrMtlUniformHandler::kGeometryBinding];
    }
    if (fFragmentUniformBuffer) {
        [renderCmdEncoder setFragmentBuffer: fFragmentUniformBuffer->mtlBuffer()
                                     offset: 0
                                    atIndex: GrMtlUniformHandler::kFragBinding];
    }
    SkASSERT(fNumSamplers == fSamplerBindings.count());
    for (int index = 0; index < fNumSamplers; ++index) {
        [renderCmdEncoder setFragmentTexture: fSamplerBindings[index].fTexture
                                     atIndex: index];
        [renderCmdEncoder setFragmentSamplerState: fSamplerBindings[index].fSampler
                                          atIndex: index];
    }
}

void GrMtlPipelineState::setRenderTargetState(const GrRenderTargetProxy* proxy) {
    GrRenderTarget* rt = proxy->peekRenderTarget();

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize size;
    size.set(rt->width(), rt->height());
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != proxy->origin() ||
        fRenderTargetState.fRenderTargetSize != size) {
        fRenderTargetState.fRenderTargetSize = size;
        fRenderTargetState.fRenderTargetOrigin = proxy->origin();

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
        float floatColors[4];
        // Swizzle the blend to match what the shader will output.
        const GrSwizzle& swizzle = fGpu->caps()->shaderCaps()->configOutputSwizzle(config);
        GrColor blendConst = swizzle.applyTo(blendInfo.fBlendConstant);
        GrColorToRGBAFloat(blendConst, floatColors);

        [renderCmdEncoder setBlendColorRed: floatColors[0]
                                     green: floatColors[1]
                                      blue: floatColors[2]
                                     alpha: floatColors[3]];
    }
}

MTLStencilOperation skia_stencil_op_to_mtl(GrStencilOp op) {
    switch (op) {
        case GrStencilOp::kKeep:
            return MTLStencilOperationKeep;
        case GrStencilOp::kZero:
            return MTLStencilOperationZero;
        case GrStencilOp::kReplace:
            return MTLStencilOperationReplace;
        case GrStencilOp::kInvert:
            return MTLStencilOperationInvert;
        case GrStencilOp::kIncWrap:
            return MTLStencilOperationIncrementWrap;
        case GrStencilOp::kDecWrap:
            return MTLStencilOperationDecrementWrap;
        case GrStencilOp::kIncClamp:
            return MTLStencilOperationIncrementClamp;
        case GrStencilOp::kDecClamp:
            return MTLStencilOperationDecrementClamp;
    }
}

MTLStencilDescriptor* skia_stencil_to_mtl(GrStencilSettings::Face face) {
    MTLStencilDescriptor* result = [[MTLStencilDescriptor alloc] init];
    switch (face.fTest) {
        case GrStencilTest::kAlways:
            result.stencilCompareFunction = MTLCompareFunctionAlways;
            break;
        case GrStencilTest::kNever:
            result.stencilCompareFunction = MTLCompareFunctionNever;
            break;
        case GrStencilTest::kGreater:
            result.stencilCompareFunction = MTLCompareFunctionGreater;
            break;
        case GrStencilTest::kGEqual:
            result.stencilCompareFunction = MTLCompareFunctionGreaterEqual;
            break;
        case GrStencilTest::kLess:
            result.stencilCompareFunction = MTLCompareFunctionLess;
            break;
        case GrStencilTest::kLEqual:
            result.stencilCompareFunction = MTLCompareFunctionLessEqual;
            break;
        case GrStencilTest::kEqual:
            result.stencilCompareFunction = MTLCompareFunctionEqual;
            break;
        case GrStencilTest::kNotEqual:
            result.stencilCompareFunction = MTLCompareFunctionNotEqual;
            break;
    }
    result.readMask = face.fTestMask;
    result.writeMask = face.fWriteMask;
    result.depthStencilPassOperation = skia_stencil_op_to_mtl(face.fPassOp);
    result.stencilFailureOperation = skia_stencil_op_to_mtl(face.fFailOp);
    return result;
}

void GrMtlPipelineState::setDepthStencilState(id<MTLRenderCommandEncoder> renderCmdEncoder) {
    if (fStencil.isDisabled()) {
        MTLDepthStencilDescriptor* desc = [[MTLDepthStencilDescriptor alloc] init];
        id<MTLDepthStencilState> state = [fGpu->device() newDepthStencilStateWithDescriptor:desc];
        [renderCmdEncoder setDepthStencilState:state];
    }
    else {
        MTLDepthStencilDescriptor* desc = [[MTLDepthStencilDescriptor alloc] init];
        desc.frontFaceStencil = skia_stencil_to_mtl(fStencil.front());
        if (fStencil.isTwoSided()) {
            desc.backFaceStencil = skia_stencil_to_mtl(fStencil.back());
            [renderCmdEncoder setStencilFrontReferenceValue:fStencil.front().fRef
                              backReferenceValue:fStencil.back().fRef];
        }
        else {
            desc.backFaceStencil = desc.frontFaceStencil;
            [renderCmdEncoder setStencilReferenceValue:fStencil.front().fRef];
        }
        id<MTLDepthStencilState> state = [fGpu->device() newDepthStencilStateWithDescriptor:desc];
        [renderCmdEncoder setDepthStencilState:state];
    }
}
