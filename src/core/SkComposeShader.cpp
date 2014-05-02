
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

SkComposeShader::SkComposeShader(SkShader* sA, SkShader* sB, SkXfermode* mode) {
    fShaderA = sA;  sA->ref();
    fShaderB = sB;  sB->ref();
    // mode may be null
    fMode = mode;
    SkSafeRef(mode);
}

SkComposeShader::SkComposeShader(SkReadBuffer& buffer) :
    INHERITED(buffer) {
    fShaderA = buffer.readShader();
    if (NULL == fShaderA) {
        fShaderA = SkNEW_ARGS(SkColorShader, ((SkColor)0));
    }
    fShaderB = buffer.readShader();
    if (NULL == fShaderB) {
        fShaderB = SkNEW_ARGS(SkColorShader, ((SkColor)0));
    }
    fMode = buffer.readXfermode();
}

SkComposeShader::~SkComposeShader() {
    SkSafeUnref(fMode);
    fShaderB->unref();
    fShaderA->unref();
}

size_t SkComposeShader::contextSize() const {
    return sizeof(ComposeShaderContext) + fShaderA->contextSize() + fShaderB->contextSize();
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

void SkComposeShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeFlattenable(fShaderA);
    buffer.writeFlattenable(fShaderB);
    buffer.writeFlattenable(fMode);
}

/*  We call validContext/createContext on our two worker shaders.
    However, we always let them see opaque alpha, and if the paint
    really is translucent, then we apply that after the fact.

 */
bool SkComposeShader::validContext(const ContextRec& rec, SkMatrix* totalInverse) const {
    if (!this->INHERITED::validContext(rec, totalInverse)) {
        return false;
    }

    // we preconcat our localMatrix (if any) with the device matrix
    // before calling our sub-shaders

    SkMatrix tmpM;
    tmpM.setConcat(*rec.fMatrix, this->getLocalMatrix());

    ContextRec newRec(rec);
    newRec.fMatrix = &tmpM;

    return fShaderA->validContext(newRec) && fShaderB->validContext(newRec);
}

SkShader::Context* SkComposeShader::createContext(const ContextRec& rec, void* storage) const {
    if (!this->validContext(rec)) {
        return NULL;
    }

    // TODO : must fix this to not "cheat" and modify fPaint
    SkAutoAlphaRestore  restore(const_cast<SkPaint*>(rec.fPaint), 0xFF);

    char* aStorage = (char*) storage + sizeof(ComposeShaderContext);
    char* bStorage = aStorage + fShaderA->contextSize();

    // we preconcat our localMatrix (if any) with the device matrix
    // before calling our sub-shaders

    SkMatrix tmpM;
    tmpM.setConcat(*rec.fMatrix, this->getLocalMatrix());

    ContextRec newRec(rec);
    newRec.fMatrix = &tmpM;

    SkShader::Context* contextA = fShaderA->createContext(newRec, aStorage);
    SkShader::Context* contextB = fShaderB->createContext(newRec, bStorage);

    // Both functions must succeed; otherwise validContext should have returned
    // false.
    SkASSERT(contextA);
    SkASSERT(contextB);

    return SkNEW_PLACEMENT_ARGS(storage, ComposeShaderContext, (*this, rec, contextA, contextB));
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

// larger is better (fewer times we have to loop), but we shouldn't
// take up too much stack-space (each element is 4 bytes)
#define TMP_COLOR_COUNT     64

void SkComposeShader::ComposeShaderContext::shadeSpan(int x, int y, SkPMColor result[], int count) {
    SkShader::Context* shaderContextA = fShaderContextA;
    SkShader::Context* shaderContextB = fShaderContextB;
    SkXfermode*        mode = static_cast<const SkComposeShader&>(fShader).fMode;
    unsigned           scale = SkAlpha255To256(this->getPaintAlpha());

    SkPMColor   tmp[TMP_COLOR_COUNT];

    if (NULL == mode) {   // implied SRC_OVER
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
            mode->xfer32(result, tmp, n, NULL);

            if (256 == scale) {
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

#ifndef SK_IGNORE_TO_STRING
void SkComposeShader::toString(SkString* str) const {
    str->append("SkComposeShader: (");

    str->append("ShaderA: ");
    fShaderA->toString(str);
    str->append(" ShaderB: ");
    fShaderB->toString(str);
    str->append(" Xfermode: ");
    fMode->toString(str);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif
