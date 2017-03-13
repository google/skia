/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkComposeShader.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkColorShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkPairShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShaderA.get());
    buffer.writeFlattenable(fShaderB.get());
}

void SkPairShader::PairContext::shadeSpan(int x, int y, SkPMColor result[], int count) {
    SkShader::Context* contextA = fContextA;
    SkShader::Context* contextB = fContextB;

    const int kTmpCount = 64;
    SkPMColor tmp[kTmpCount];

    do {
        int n = count;
        if (n > kTmpCount) {
            n = kTmpCount;
        }

        contextA->shadeSpan(x, y, result, n);
        contextB->shadeSpan(x, y, tmp, n);

        ((SkPairShader*)&fShader)->mixSpans(result, tmp, n, this->getPaintAlpha());

        result += n;
        x += n;
        count -= n;
    } while (count > 0);
}

SkShader::Context* SkPairShader::onMakeContext(const ContextRec& rec, SkArenaAlloc* alloc) const {
    // we preconcat our localMatrix (if any) with the device matrix
    // before calling our sub-shaders
    SkMatrix tmpM;
    tmpM.setConcat(*rec.fMatrix, this->getLocalMatrix());

    // Our sub-shaders need to see opaque, so by combining them we don't double-alphatize the
    // result. The subclass itself will respect the alpha, and post-apply it after calling the
    // sub-shaders.
    SkPaint opaquePaint(*rec.fPaint);
    opaquePaint.setAlpha(0xFF);

    ContextRec newRec(rec);
    newRec.fMatrix = &tmpM;
    newRec.fPaint = &opaquePaint;

    SkShader::Context* contextA = fShaderA->makeContext(newRec, alloc);
    SkShader::Context* contextB = fShaderB->makeContext(newRec, alloc);
    if (!contextA || !contextB) {
        return nullptr;
    }

    return alloc->make<PairContext>(*this, rec, contextA, contextB);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkShader::MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                            SkBlendMode mode) {
    if (!src || !dst) {
        return nullptr;
    }
    if (SkBlendMode::kSrc == mode) {
        return src;
    }
    if (SkBlendMode::kDst == mode) {
        return dst;
    }
    return sk_sp<SkShader>(new SkComposeShader(std::move(dst), std::move(src), mode));
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkComposeShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shaderA(buffer.readShader());
    sk_sp<SkShader> shaderB(buffer.readShader());
    SkBlendMode mode;
    if (buffer.isVersionLT(SkReadBuffer::kXfermodeToBlendMode2_Version)) {
        sk_sp<SkXfermode> xfer = buffer.readXfermode();
        mode = xfer ? xfer->blend() : SkBlendMode::kSrcOver;
    } else {
        mode = (SkBlendMode)buffer.read32();
    }
    if (!shaderA || !shaderB) {
        return nullptr;
    }
    return sk_make_sp<SkComposeShader>(std::move(shaderA), std::move(shaderB), mode);
}

void SkComposeShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.write32((int)fMode);
}

void SkComposeShader::mixSpans(SkPMColor result[], const SkPMColor spanB[], int count,
                               uint8_t alpha) const {
    unsigned scale = SkAlpha255To256(alpha);

    SkXfermode* xfer = SkXfermode::Peek(fMode);
    if (nullptr == xfer) {   // implied SRC_OVER
        if (256 == scale) {
            for (int i = 0; i < count; i++) {
                result[i] = SkPMSrcOver(spanB[i], result[i]);
            }
        } else {
            for (int i = 0; i < count; i++) {
                result[i] = SkAlphaMulQ(SkPMSrcOver(spanB[i], result[i]), scale);
            }
        }
    } else {
        xfer->xfer32(result, spanB, count, nullptr);
        if (256 != scale) {
            for (int i = 0; i < count; i++) {
                result[i] = SkAlphaMulQ(result[i], scale);
            }
        }
    }
}

bool SkComposeShader::asACompose(ComposeRec* rec) const {
    if (rec) {
        rec->fShaderA   = fShaderA.get();
        rec->fShaderB   = fShaderB.get();
        rec->fBlendMode = fMode;
    }
    return true;
}


#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

sk_sp<GrFragmentProcessor> SkComposeShader::asFragmentProcessor(const AsFPArgs& args) const {
    switch (fMode) {
        case SkBlendMode::kClear:
            return GrConstColorProcessor::Make(GrColor4f::TransparentBlack(),
                                               GrConstColorProcessor::kIgnore_InputMode);
            break;
        case SkBlendMode::kSrc:
            return fShaderB->asFragmentProcessor(args);
            break;
        case SkBlendMode::kDst:
            return fShaderA->asFragmentProcessor(args);
            break;
        default:
            sk_sp<GrFragmentProcessor> fpA(fShaderA->asFragmentProcessor(args));
            if (!fpA) {
                return nullptr;
            }
            sk_sp<GrFragmentProcessor> fpB(fShaderB->asFragmentProcessor(args));
            if (!fpB) {
                return nullptr;
            }
            return GrXfermodeFragmentProcessor::MakeFromTwoProcessors(std::move(fpB),
                                                                      std::move(fpA), fMode);
    }
}
#endif

#ifndef SK_IGNORE_TO_STRING
void SkComposeShader::toString(SkString* str) const {
    str->append("SkComposeShader: (");

    str->append("ShaderA: ");
    fShaderA->toString(str);
    str->append(" ShaderB: ");
    fShaderB->toString(str);
    if (SkBlendMode::kSrcOver != fMode) {
        str->appendf(" Xfermode: %s", SkXfermode::ModeName(fMode));
    }

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkMixerShader::mixSpans(SkPMColor result[], const SkPMColor spanB[], int count,
                             uint8_t alpha) const {
    const unsigned scale = SkAlpha255To256(alpha);
    for (int i = 0; i < count; ++i) {
        result[i] = SkFastFourByteInterp256(result[i], spanB[i], (unsigned)(fWeight * 256));
        if (256 == scale) {
            result[i] = SkAlphaMulQ(result[i], scale);
        }
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkMixerShader::toString(SkString* str) const {
    str->appendf("SkMixerShader(%g)", fWeight);
}
#endif

void SkMixerShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fWeight);
}

sk_sp<SkFlattenable> SkMixerShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> sh0(buffer.readShader());
    sk_sp<SkShader> sh1(buffer.readShader());
    const float weight = buffer.readScalar();
    return MakeMixer(std::move(sh0), std::move(sh1), weight);
}

sk_sp<SkShader> SkShader::MakeMixer(sk_sp<SkShader> sh0, sk_sp<SkShader> sh1, float weight) {
    if (!sh0 || !sh1) {
        return nullptr;
    }
    if (!SkScalarIsFinite(weight)) {
        return nullptr;
    }
    
    weight = SkTMin(SkTMax(weight, 0.f), 1.f);
    return sk_sp<SkShader>(new SkMixerShader(std::move(sh0), std::move(sh1), weight));
}

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkMixerShader::asFragmentProcessor(const AsFPArgs&) const {
    return nullptr;
}
#endif
