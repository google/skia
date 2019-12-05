/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrPipeline.h"

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrXferProcessor.h"

#include "src/gpu/ops/GrOp.h"

GrPipeline::GrPipeline(const InitArgs& args,
                       GrProcessorSet&& processors,
                       GrAppliedClip&& appliedClip)
        : fOutputSwizzle(args.fOutputSwizzle) {
    SkASSERT(processors.isFinalized());

    fFlags = (Flags)args.fInputFlags;
    if (appliedClip.hasStencilClip()) {
        fFlags |= Flags::kHasStencilClip;
    }
    if (appliedClip.scissorState().enabled()) {
        fFlags |= Flags::kScissorEnabled;
    }

    fWindowRectsState = appliedClip.windowRectsState();
    if (!args.fUserStencil->isDisabled(fFlags & Flags::kHasStencilClip)) {
        fFlags |= Flags::kStencilEnabled;
    }

    fUserStencilSettings = args.fUserStencil;

    fXferProcessor = processors.refXferProcessor();

    if (args.fDstProxyView.proxy()) {
        fDstProxyView = args.fDstProxyView.proxyView();
        fDstTextureOffset = args.fDstProxyView.offset();
    }

    // Copy GrFragmentProcessors from GrProcessorSet to Pipeline
    fNumColorProcessors = processors.numColorFragmentProcessors();
    int numTotalProcessors = fNumColorProcessors +
                             processors.numCoverageFragmentProcessors() +
                             appliedClip.numClipCoverageFragmentProcessors();
    fFragmentProcessors.reset(numTotalProcessors);

    int currFPIdx = 0;
    for (int i = 0; i < processors.numColorFragmentProcessors(); ++i, ++currFPIdx) {
        fFragmentProcessors[currFPIdx] = processors.detachColorFragmentProcessor(i);
    }
    for (int i = 0; i < processors.numCoverageFragmentProcessors(); ++i, ++currFPIdx) {
        fFragmentProcessors[currFPIdx] = processors.detachCoverageFragmentProcessor(i);
    }
    for (int i = 0; i < appliedClip.numClipCoverageFragmentProcessors(); ++i, ++currFPIdx) {
        fFragmentProcessors[currFPIdx] = appliedClip.detachClipCoverageFragmentProcessor(i);
    }
}

GrXferBarrierType GrPipeline::xferBarrierType(GrTexture* texture, const GrCaps& caps) const {
    auto proxy = fDstProxyView.proxy();
    if (proxy && proxy->peekTexture() == texture) {
        return kTexture_GrXferBarrierType;
    }
    return this->getXferProcessor().xferBarrierType(caps);
}

GrPipeline::GrPipeline(GrScissorTest scissorTest, sk_sp<const GrXferProcessor> xp,
                       const GrSwizzle& outputSwizzle, InputFlags inputFlags,
                       const GrUserStencilSettings* userStencil)
        : fWindowRectsState()
        , fUserStencilSettings(userStencil)
        , fFlags((Flags)inputFlags)
        , fXferProcessor(std::move(xp))
        , fFragmentProcessors()
        , fNumColorProcessors(0)
        , fOutputSwizzle(outputSwizzle) {
    if (GrScissorTest::kEnabled == scissorTest) {
        fFlags |= Flags::kScissorEnabled;
    }
    if (!userStencil->isDisabled(false)) {
        fFlags |= Flags::kStencilEnabled;
    }
}

void GrPipeline::genKey(GrProcessorKeyBuilder* b, const GrCaps& caps) const {
    // kSnapVerticesToPixelCenters is implemented in a shader.
    InputFlags ignoredFlags = InputFlags::kSnapVerticesToPixelCenters;
    if (!caps.multisampleDisableSupport()) {
        // Ganesh will omit kHWAntialias regardless multisampleDisableSupport.
        ignoredFlags |= InputFlags::kHWAntialias;
    }
    b->add32((uint32_t)fFlags & ~(uint32_t)ignoredFlags);

    const GrXferProcessor::BlendInfo& blendInfo = this->getXferProcessor().getBlendInfo();

    static const uint32_t kBlendWriteShift = 1;
    static const uint32_t kBlendCoeffShift = 5;
    GR_STATIC_ASSERT(kLast_GrBlendCoeff < (1 << kBlendCoeffShift));
    GR_STATIC_ASSERT(kFirstAdvancedGrBlendEquation - 1 < 4);

    uint32_t blendKey = blendInfo.fWriteColor;
    blendKey |= (blendInfo.fSrcBlend << kBlendWriteShift);
    blendKey |= (blendInfo.fDstBlend << (kBlendWriteShift + kBlendCoeffShift));
    blendKey |= (blendInfo.fEquation << (kBlendWriteShift + 2 * kBlendCoeffShift));

    b->add32(blendKey);
}

void GrPipeline::visitProxies(const GrOp::VisitProxyFunc& func) const {
    // This iteration includes any clip coverage FPs
    for (auto [sampler, fp] : GrFragmentProcessor::PipelineTextureSamplerRange(*this)) {
        bool mipped = (GrSamplerState::Filter::kMipMap == sampler.samplerState().filter());
        func(sampler.proxy(), GrMipMapped(mipped));
    }
    if (fDstProxyView.asTextureProxy()) {
        func(fDstProxyView.asTextureProxy(), GrMipMapped::kNo);
    }
}
