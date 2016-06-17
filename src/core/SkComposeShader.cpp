/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkComposeShader.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkColorShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkXfermode.h"
#include "SkString.h"

///////////////////////////////////////////////////////////////////////////////

size_t SkComposeShader::onContextSize(const ContextRec& rec) const {
    return sizeof(ComposeShaderContext)
        + fShaderA->contextSize(rec)
        + fShaderB->contextSize(rec);
}

class SkAutoAlphaRestore {
public:
    SkAutoAlphaRestore(SkPaint* paint, uint8_t newAlpha) {
        fAlpha = paint->getAlpha();
        fPaint = paint;
        paint->setAlpha(newAlpha);
    }

    ~SkAutoAlphaRestore() {
        fPaint->setAlpha(fAlpha);
    }
private:
    SkPaint*    fPaint;
    uint8_t     fAlpha;
};
#define SkAutoAlphaRestore(...) SK_REQUIRE_LOCAL_VAR(SkAutoAlphaRestore)

sk_sp<SkFlattenable> SkComposeShader::CreateProc(SkReadBuffer& buffer) {
    sk_sp<SkShader> shaderA(buffer.readShader());
    sk_sp<SkShader> shaderB(buffer.readShader());
    sk_sp<SkXfermode> mode(buffer.readXfermode());
    if (!shaderA || !shaderB) {
        return nullptr;
    }
    return sk_make_sp<SkComposeShader>(std::move(shaderA), std::move(shaderB), std::move(mode));
}

void SkComposeShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeFlattenable(fShaderA.get());
    buffer.writeFlattenable(fShaderB.get());
    buffer.writeFlattenable(fMode.get());
}

template <typename T> void safe_call_destructor(T* obj) {
    if (obj) {
        obj->~T();
    }
}

SkShader::Context* SkComposeShader::onCreateContext(const ContextRec& rec, void* storage) const {
    char* aStorage = (char*) storage + sizeof(ComposeShaderContext);
    char* bStorage = aStorage + fShaderA->contextSize(rec);

    // we preconcat our localMatrix (if any) with the device matrix
    // before calling our sub-shaders
    SkMatrix tmpM;
    tmpM.setConcat(*rec.fMatrix, this->getLocalMatrix());

    // Our sub-shaders need to see opaque, so by combining them we don't double-alphatize the
    // result. ComposeShader itself will respect the alpha, and post-apply it after calling the
    // sub-shaders.
    SkPaint opaquePaint(*rec.fPaint);
    opaquePaint.setAlpha(0xFF);

    ContextRec newRec(rec);
    newRec.fMatrix = &tmpM;
    newRec.fPaint = &opaquePaint;

    SkShader::Context* contextA = fShaderA->createContext(newRec, aStorage);
    SkShader::Context* contextB = fShaderB->createContext(newRec, bStorage);
    if (!contextA || !contextB) {
        safe_call_destructor(contextA);
        safe_call_destructor(contextB);
        return nullptr;
    }

    return new (storage) ComposeShaderContext(*this, rec, contextA, contextB);
}

SkComposeShader::ComposeShaderContext::ComposeShaderContext(
        const SkComposeShader& shader, const ContextRec& rec,
        SkShader::Context* contextA, SkShader::Context* contextB)
    : INHERITED(shader, rec)
    , fShaderContextA(contextA)
    , fShaderContextB(contextB) {}

SkComposeShader::ComposeShaderContext::~ComposeShaderContext() {
    fShaderContextA->~Context();
    fShaderContextB->~Context();
}

bool SkComposeShader::asACompose(ComposeRec* rec) const {
    if (rec) {
        rec->fShaderA = fShaderA.get();
        rec->fShaderB = fShaderB.get();
        rec->fMode = fMode.get();
    }
    return true;
}


// larger is better (fewer times we have to loop), but we shouldn't
// take up too much stack-space (each element is 4 bytes)
#define TMP_COLOR_COUNT     64

void SkComposeShader::ComposeShaderContext::shadeSpan(int x, int y, SkPMColor result[], int count) {
    SkShader::Context* shaderContextA = fShaderContextA;
    SkShader::Context* shaderContextB = fShaderContextB;
    SkXfermode*        mode = static_cast<const SkComposeShader&>(fShader).fMode.get();
    unsigned           scale = SkAlpha255To256(this->getPaintAlpha());

    SkPMColor   tmp[TMP_COLOR_COUNT];

    if (nullptr == mode) {   // implied SRC_OVER
        // TODO: when we have a good test-case, should use SkBlitRow::Proc32
        // for these loops
        do {
            int n = count;
            if (n > TMP_COLOR_COUNT) {
                n = TMP_COLOR_COUNT;
            }

            shaderContextA->shadeSpan(x, y, result, n);
            shaderContextB->shadeSpan(x, y, tmp, n);

            if (256 == scale) {
                for (int i = 0; i < n; i++) {
                    result[i] = SkPMSrcOver(tmp[i], result[i]);
                }
            } else {
                for (int i = 0; i < n; i++) {
                    result[i] = SkAlphaMulQ(SkPMSrcOver(tmp[i], result[i]),
                                            scale);
                }
            }

            result += n;
            x += n;
            count -= n;
        } while (count > 0);
    } else {    // use mode for the composition
        do {
            int n = count;
            if (n > TMP_COLOR_COUNT) {
                n = TMP_COLOR_COUNT;
            }

            shaderContextA->shadeSpan(x, y, result, n);
            shaderContextB->shadeSpan(x, y, tmp, n);
            mode->xfer32(result, tmp, n, nullptr);

            if (256 != scale) {
                for (int i = 0; i < n; i++) {
                    result[i] = SkAlphaMulQ(result[i], scale);
                }
            }

            result += n;
            x += n;
            count -= n;
        } while (count > 0);
    }
}

#if SK_SUPPORT_GPU

#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

/////////////////////////////////////////////////////////////////////

const GrFragmentProcessor* SkComposeShader::asFragmentProcessor(GrContext* context,
                                                            const SkMatrix& viewM,
                                                            const SkMatrix* localMatrix,
                                                            SkFilterQuality fq) const {
    // Fragment processor will only support SkXfermode::Mode modes currently.
    SkXfermode::Mode mode;
    if (!(SkXfermode::AsMode(fMode, &mode))) {
        return nullptr;
    }

    switch (mode) {
        case SkXfermode::kClear_Mode:
            return GrConstColorProcessor::Create(GrColor_TRANSPARENT_BLACK,
                                                 GrConstColorProcessor::kIgnore_InputMode);
            break;
        case SkXfermode::kSrc_Mode:
            return fShaderB->asFragmentProcessor(context, viewM, localMatrix, fq);
            break;
        case SkXfermode::kDst_Mode:
            return fShaderA->asFragmentProcessor(context, viewM, localMatrix, fq);
            break;
        default:
            SkAutoTUnref<const GrFragmentProcessor> fpA(fShaderA->asFragmentProcessor(context,
                                                        viewM, localMatrix, fq));
            if (!fpA.get()) {
                return nullptr;
            }
            SkAutoTUnref<const GrFragmentProcessor> fpB(fShaderB->asFragmentProcessor(context,
                                                        viewM, localMatrix, fq));
            if (!fpB.get()) {
                return nullptr;
            }
            return GrXfermodeFragmentProcessor::CreateFromTwoProcessors(fpB, fpA, mode);
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
    if (fMode) {
        str->append(" Xfermode: ");
        fMode->toString(str);
    }

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkShader::MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                            sk_sp<SkXfermode> xfer) {
    if (!dst || !src) {
        return nullptr;
    }
    return sk_make_sp<SkComposeShader>(std::move(dst), std::move(src), std::move(xfer));
}

sk_sp<SkShader> SkShader::MakeComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src,
                                            SkXfermode::Mode mode) {
    return MakeComposeShader(std::move(dst), std::move(src), SkXfermode::Make(mode));
}
