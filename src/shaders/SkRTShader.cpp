/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "src/shaders/SkRTShader.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include "src/sksl/SkSLByteCode.h"
#include "src/sksl/SkSLCompiler.h"

SkRTShader::SkRTShader(SkString sksl, sk_sp<SkData> inputs, const SkMatrix* localMatrix,
                       bool isOpaque)
    : SkShaderBase(localMatrix)
    , fSkSL(std::move(sksl))
    , fInputs(std::move(inputs))
    , fIsOpaque(isOpaque)
{}

bool SkRTShader::onAppendStages(const SkStageRec& rec) const {
    SkMatrix inverse;
    if (!this->computeTotalInverse(rec.fCTM, rec.fLocalM, &inverse)) {
        return false;
    }

    auto ctx = rec.fAlloc->make<SkRasterPipeline_InterpreterCtx>();
    ctx->inputs = fInputs->data();
    ctx->ninputs = fInputs->size() / 4;
    ctx->shader_convention = true;

    SkAutoMutexExclusive ama(fByteCodeMutex);
    if (!fByteCode) {
        SkSL::Compiler c;
        auto prog = c.convertProgram(SkSL::Program::kGeneric_Kind,
                                     SkSL::String(fSkSL.c_str()),
                                     SkSL::Program::Settings());
        if (c.errorCount()) {
            SkDebugf("%s\n", c.errorText().c_str());
            return false;
        }
        fByteCode = c.toByteCode(*prog);
        if (c.errorCount()) {
            SkDebugf("%s\n", c.errorText().c_str());
            return false;
        }
        SkASSERT(fByteCode);
    }
    ctx->byteCode = fByteCode.get();
    ctx->fn = ctx->byteCode->fFunctions[0].get();

    rec.fPipeline->append(SkRasterPipeline::seed_shader);
    rec.fPipeline->append_matrix(rec.fAlloc, inverse);
    rec.fPipeline->append(SkRasterPipeline::interpreter, ctx);
    return true;
}

enum Flags {
    kIsOpaque_Flag          = 1 << 0,
    kHasLocalMatrix_Flag    = 1 << 1,
};

void SkRTShader::flatten(SkWriteBuffer& buffer) const {
    uint32_t flags = 0;
    if (fIsOpaque) {
        flags |= kIsOpaque_Flag;
    }
    if (!this->getLocalMatrix().isIdentity()) {
        flags |= kHasLocalMatrix_Flag;
    }

    buffer.writeString(fSkSL.c_str());
    if (fInputs) {
        buffer.writeDataAsByteArray(fInputs.get());
    } else {
        buffer.writeByteArray(nullptr, 0);
    }
    buffer.write32(flags);
    if (flags & kHasLocalMatrix_Flag) {
        buffer.writeMatrix(this->getLocalMatrix());
    }
}

sk_sp<SkFlattenable> SkRTShader::CreateProc(SkReadBuffer& buffer) {
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> inputs = buffer.readByteArrayAsData();
    uint32_t flags = buffer.read32();

    bool isOpaque = SkToBool(flags & kIsOpaque_Flag);
    SkMatrix localM, *localMPtr = nullptr;
    if (flags & kHasLocalMatrix_Flag) {
        buffer.readMatrix(&localM);
        localMPtr = &localM;
    }

    return sk_sp<SkFlattenable>(new SkRTShader(std::move(sksl), std::move(inputs),
                                               localMPtr, isOpaque));
}

sk_sp<SkShader> SkRuntimeShaderMaker(SkString sksl, sk_sp<SkData> inputs,
                                     const SkMatrix* localMatrix, bool isOpaque) {
    return sk_sp<SkShader>(new SkRTShader(std::move(sksl), std::move(inputs),
                                          localMatrix, isOpaque));
}

#if SK_SUPPORT_GPU

#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColorSpaceInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/SkGr.h"

#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/generated/GrMixerEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"

std::unique_ptr<GrFragmentProcessor> SkRTShader::asFragmentProcessor(const GrFPArgs& args) const {

    static int rtIndex = GrSkSLFP::NewIndex();
    return GrSkSLFP::Make(args.fContext, rtIndex, "runtime-shader", fSkSL, nullptr, 0);

#if 0
    const auto lm = this->totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix);
    SkMatrix lmInverse;
    if (!lm->invert(&lmInverse)) {
        return nullptr;
    }

    GrSamplerState::WrapMode wrapModes[] = {tile_mode_to_wrap_mode(fTileModeX),
        tile_mode_to_wrap_mode(fTileModeY)};

    // If either domainX or domainY are un-ignored, a texture domain effect has to be used to
    // implement the decal mode (while leaving non-decal axes alone). The wrap mode originally
    // clamp-to-border is reset to clamp since the hw cannot implement it directly.
    GrTextureDomain::Mode domainX = GrTextureDomain::kIgnore_Mode;
    GrTextureDomain::Mode domainY = GrTextureDomain::kIgnore_Mode;
    if (!args.fContext->priv().caps()->clampToBorderSupport()) {
        if (wrapModes[0] == GrSamplerState::WrapMode::kClampToBorder) {
            domainX = GrTextureDomain::kDecal_Mode;
            wrapModes[0] = GrSamplerState::WrapMode::kClamp;
        }
        if (wrapModes[1] == GrSamplerState::WrapMode::kClampToBorder) {
            domainY = GrTextureDomain::kDecal_Mode;
            wrapModes[1] = GrSamplerState::WrapMode::kClamp;
        }
    }

    // Must set wrap and filter on the sampler before requesting a texture. In two places below
    // we check the matrix scale factors to determine how to interpret the filter quality setting.
    // This completely ignores the complexity of the drawVertices case where explicit local coords
    // are provided by the caller.
    bool doBicubic;
    GrSamplerState::Filter textureFilterMode = GrSkFilterQualityToGrFilterMode(
                                                                               args.fFilterQuality, *args.fViewMatrix, *lm,
                                                                               args.fContext->priv().options().fSharpenMipmappedTextures, &doBicubic);
    GrSamplerState samplerState(wrapModes, textureFilterMode);
    SkScalar scaleAdjust[2] = { 1.0f, 1.0f };
    sk_sp<GrTextureProxy> proxy(as_IB(fImage)->asTextureProxyRef(args.fContext, samplerState,
                                                                 scaleAdjust));
    if (!proxy) {
        return nullptr;
    }

    GrPixelConfig config = proxy->config();
    bool isAlphaOnly = GrPixelConfigIsAlphaOnly(config);

    lmInverse.postScale(scaleAdjust[0], scaleAdjust[1]);

    std::unique_ptr<GrFragmentProcessor> inner;
    if (doBicubic) {
        // domainX and domainY will properly apply the decal effect with the texture domain used in
        // the bicubic filter if clamp to border was unsupported in hardware
        static constexpr auto kDir = GrBicubicEffect::Direction::kXY;
        inner = GrBicubicEffect::Make(std::move(proxy), lmInverse, wrapModes, domainX, domainY,
                                      kDir, fImage->alphaType());
    } else {
        if (domainX != GrTextureDomain::kIgnore_Mode || domainY != GrTextureDomain::kIgnore_Mode) {
            SkRect domain = GrTextureDomain::MakeTexelDomain(
                                                             SkIRect::MakeWH(proxy->width(), proxy->height()),
                                                             domainX, domainY);
            inner = GrTextureDomainEffect::Make(std::move(proxy), lmInverse, domain,
                                                domainX, domainY, samplerState);
        } else {
            inner = GrSimpleTextureEffect::Make(std::move(proxy), lmInverse, samplerState);
        }
    }
    inner = GrColorSpaceXformEffect::Make(std::move(inner), fImage->colorSpace(),
                                          fImage->alphaType(),
                                          args.fDstColorSpaceInfo->colorSpace());
    if (isAlphaOnly) {
        return inner;
    } else if (args.fInputColorIsOpaque) {
        return GrFragmentProcessor::OverrideInput(std::move(inner), SK_PMColor4fWHITE, false);
    }
    return GrFragmentProcessor::MulChildByInputAlpha(std::move(inner));
#endif
}

#endif

