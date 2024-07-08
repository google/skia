/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/GrPipeline.h"

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/Blend.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrAppliedClip.h"
#include "src/gpu/ganesh/GrProcessorSet.h"
#include "src/gpu/ganesh/GrScissorState.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

#include <utility>

GrPipeline::GrPipeline(const InitArgs& args,
                       sk_sp<const GrXferProcessor> xferProcessor,
                       const GrAppliedHardClip& hardClip)
        : fDstProxy(args.fDstProxyView)
        , fWindowRectsState(hardClip.windowRectsState())
        , fXferProcessor(std::move(xferProcessor))
        , fWriteSwizzle(args.fWriteSwizzle) {
    fFlags = (Flags)args.fInputFlags;
    if (hardClip.hasStencilClip()) {
        fFlags |= Flags::kHasStencilClip;
    }
    if (hardClip.scissorState().enabled()) {
        fFlags |= Flags::kScissorTestEnabled;
    }
    // If we have any special dst sample flags we better also have a dst proxy
    SkASSERT(this->dstSampleFlags() == GrDstSampleFlags::kNone || this->dstProxyView());
}

GrPipeline::GrPipeline(const InitArgs& args, GrProcessorSet&& processors,
                       GrAppliedClip&& appliedClip)
        : GrPipeline(args, processors.refXferProcessor(), appliedClip.hardClip()) {
    SkASSERT(processors.isFinalized());
    // Copy GrFragmentProcessors from GrProcessorSet to Pipeline
    fNumColorProcessors = processors.hasColorFragmentProcessor() ? 1 : 0;
    int numTotalProcessors = fNumColorProcessors +
                             (processors.hasCoverageFragmentProcessor() ? 1 : 0) +
                             (appliedClip.hasCoverageFragmentProcessor() ? 1 : 0);
    fFragmentProcessors.reset(numTotalProcessors);

    int currFPIdx = 0;
    if (processors.hasColorFragmentProcessor()) {
        fFragmentProcessors[currFPIdx++] = processors.detachColorFragmentProcessor();
    }
    if (processors.hasCoverageFragmentProcessor()) {
        fFragmentProcessors[currFPIdx++] = processors.detachCoverageFragmentProcessor();
    }
    if (appliedClip.hasCoverageFragmentProcessor()) {
        fFragmentProcessors[currFPIdx++] = appliedClip.detachCoverageFragmentProcessor();
    }
}

GrXferBarrierType GrPipeline::xferBarrierType(const GrCaps& caps) const {
    if (this->dstSampleFlags() & GrDstSampleFlags::kRequiresTextureBarrier) {
        return kTexture_GrXferBarrierType;
    }
    return this->getXferProcessor().xferBarrierType(caps);
}

GrPipeline::GrPipeline(GrScissorTest scissorTest,
                       sk_sp<const GrXferProcessor> xp,
                       const skgpu::Swizzle& writeSwizzle,
                       InputFlags inputFlags)
        : fWindowRectsState()
        , fFlags((Flags)inputFlags)
        , fXferProcessor(std::move(xp))
        , fWriteSwizzle(writeSwizzle) {
    if (GrScissorTest::kEnabled == scissorTest) {
        fFlags |= Flags::kScissorTestEnabled;
    }
}

void GrPipeline::genKey(skgpu::KeyBuilder* b, const GrCaps& caps) const {
    // kSnapVerticesToPixelCenters is implemented in a shader.
    InputFlags ignoredFlags = InputFlags::kSnapVerticesToPixelCenters;
    b->add32((uint32_t)fFlags & ~(uint32_t)ignoredFlags, "flags");

    const skgpu::BlendInfo& blendInfo = this->getXferProcessor().getBlendInfo();

    static constexpr uint32_t kBlendCoeffSize = 5;
    static constexpr uint32_t kBlendEquationSize = 5;
    static_assert(static_cast<int>(skgpu::BlendCoeff::kLast) < (1 << kBlendCoeffSize));
    static_assert(static_cast<int>(skgpu::BlendEquation::kLast) < (1 << kBlendEquationSize));

    b->addBool(blendInfo.fWritesColor, "writesColor");
    b->addBits(kBlendCoeffSize, static_cast<int>(blendInfo.fSrcBlend), "srcBlend");
    b->addBits(kBlendCoeffSize, static_cast<int>(blendInfo.fDstBlend), "dstBlend");
    b->addBits(kBlendEquationSize, static_cast<int>(blendInfo.fEquation), "equation");
    b->addBool(this->usesDstInputAttachment(), "inputAttach");
}

void GrPipeline::visitTextureEffects(
        const std::function<void(const GrTextureEffect&)>& func) const {
    for (auto& fp : fFragmentProcessors) {
        fp->visitTextureEffects(func);
    }
}

void GrPipeline::visitProxies(const GrVisitProxyFunc& func) const {
    // This iteration includes any clip coverage FPs
    for (auto& fp : fFragmentProcessors) {
        fp->visitProxies(func);
    }
    if (this->usesDstTexture()) {
        func(this->dstProxyView().proxy(), skgpu::Mipmapped::kNo);
    }
}

void GrPipeline::setDstTextureUniforms(const GrGLSLProgramDataManager& pdm,
                                       GrGLSLBuiltinUniformHandles* fBuiltinUniformHandles) const {
    GrTexture* dstTexture = this->peekDstTexture();

    if (dstTexture) {
        if (fBuiltinUniformHandles->fDstTextureCoordsUni.isValid()) {
            float scaleX = 1.f;
            float scaleY = 1.f;
            if (dstTexture->textureType() == GrTextureType::kRectangle) {
                // When we have a rectangle texture, we use the scaleX component to store the height
                // in case we need to flip the coords when using a bottom left origin.
                scaleX = dstTexture->height();
            } else {
                scaleX /= dstTexture->width();
                scaleY /= dstTexture->height();
            }
            pdm.set4f(fBuiltinUniformHandles->fDstTextureCoordsUni,
                      static_cast<float>(this->dstTextureOffset().fX),
                      static_cast<float>(this->dstTextureOffset().fY),
                      scaleX,
                      scaleY);
        }
    } else {
        SkASSERT(!fBuiltinUniformHandles->fDstTextureCoordsUni.isValid());
    }
}
