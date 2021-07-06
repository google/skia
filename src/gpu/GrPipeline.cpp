/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrPipeline.h"

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrXferProcessor.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

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
                       const GrSwizzle& writeSwizzle,
                       InputFlags inputFlags)
        : fWindowRectsState()
        , fFlags((Flags)inputFlags)
        , fXferProcessor(std::move(xp))
        , fWriteSwizzle(writeSwizzle) {
    if (GrScissorTest::kEnabled == scissorTest) {
        fFlags |= Flags::kScissorTestEnabled;
    }
}

void GrPipeline::genKey(GrProcessorKeyBuilder* b, const GrCaps& caps) const {
    // kSnapVerticesToPixelCenters is implemented in a shader.
    InputFlags ignoredFlags = InputFlags::kSnapVerticesToPixelCenters;
    if (!caps.multisampleDisableSupport()) {
        // Ganesh will omit kHWAntialias regardless of multisampleDisableSupport.
        ignoredFlags |= InputFlags::kHWAntialias;
    }
    b->add32((uint32_t)fFlags & ~(uint32_t)ignoredFlags, "flags");

    const GrXferProcessor::BlendInfo& blendInfo = this->getXferProcessor().getBlendInfo();

    static constexpr uint32_t kBlendCoeffSize = 5;
    static constexpr uint32_t kBlendEquationSize = 5;
    static_assert(kLast_GrBlendCoeff < (1 << kBlendCoeffSize));
    static_assert(kLast_GrBlendEquation < (1 << kBlendEquationSize));

    b->addBool(blendInfo.fWriteColor, "writeColor");
    b->addBits(kBlendCoeffSize, blendInfo.fSrcBlend, "srcBlend");
    b->addBits(kBlendCoeffSize, blendInfo.fDstBlend, "dstBlend");
    b->addBits(kBlendEquationSize, blendInfo.fEquation, "equation");
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
        func(this->dstProxyView().proxy(), GrMipmapped::kNo);
    }
}

void GrPipeline::setDstTextureUniforms(const GrGLSLProgramDataManager& pdm,
                                       GrGLSLBuiltinUniformHandles* fBuiltinUniformHandles) const {
    GrTexture* dstTexture = this->peekDstTexture();

    if (dstTexture) {
        if (fBuiltinUniformHandles->fDstTextureCoordsUni.isValid()) {
            pdm.set4f(fBuiltinUniformHandles->fDstTextureCoordsUni,
                      static_cast<float>(this->dstTextureOffset().fX),
                      static_cast<float>(this->dstTextureOffset().fY),
                      1.f / dstTexture->width(),
                      1.f / dstTexture->height());
        }
    } else {
        SkASSERT(!fBuiltinUniformHandles->fDstTextureCoordsUni.isValid());
    }
}
