
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

template <typename T> void safe_call_destructor(T* obj) {
    if (obj) {
        obj->~T();
    }
}

SkShader::Context* SkComposeShader::onCreateContext(const ContextRec& rec, void* storage) const {
    char* aStorage = (char*) storage + sizeof(ComposeShaderContext);
    char* bStorage = aStorage + fShaderA->contextSize();

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
        return NULL;
    }

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

bool SkComposeShader::asACompose(ComposeRec* rec) const {
    if (rec) {
        rec->fShaderA = fShaderA;
        rec->fShaderB = fShaderB;
        rec->fMode = fMode;
    }
    return true;
}


// larger is better (fewer times we have to loop), but we shouldn't
// take up too much stack-space (each element is 4 bytes)
#define TMP_COLOR_COUNT     64

void SkComposeShader::ComposeShaderContext::shadeSpan(int x, int y, SkPMColor result[], int count) {
    SkShader::Context* shaderContextA = fShaderContextA;
    SkShader::Context* shaderContextB = fShaderContextB;
    SkXfermode*        mode = static_cast<const SkComposeShader&>(fShader).fMode;
    unsigned           scale = SkAlpha255To256(this->getPaintAlpha());

#ifdef SK_BUILD_FOR_ANDROID
    scale = 256;    // ugh -- maintain old bug/behavior for now
#endif

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
