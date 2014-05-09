/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

class SkLocalMatrixShader : public SkShader {
public:
    SkLocalMatrixShader(SkShader* proxy, const SkMatrix& localMatrix)
        : fProxyShader(SkRef(proxy))
        , fProxyLocalMatrix(localMatrix)
    {}

    virtual size_t contextSize() const SK_OVERRIDE {
        return fProxyShader->contextSize();
    }

    virtual BitmapType asABitmap(SkBitmap* bitmap, SkMatrix* matrix,
                                 TileMode* mode) const SK_OVERRIDE {
        return fProxyShader->asABitmap(bitmap, matrix, mode);
    }

    virtual GradientType asAGradient(GradientInfo* info) const SK_OVERRIDE {
        return fProxyShader->asAGradient(info);
    }

    // TODO: need to augment this API to pass in a localmatrix (which we can augment)
    virtual GrEffectRef* asNewEffect(GrContext* ctx, const SkPaint& paint,
                                     const SkMatrix* localMatrix) const SK_OVERRIDE {
        SkMatrix tmp = fProxyLocalMatrix;
        if (localMatrix) {
            tmp.preConcat(*localMatrix);
        }
        return fProxyShader->asNewEffect(ctx, paint, &tmp);
    }

    virtual SkShader* refAsALocalMatrixShader(SkMatrix* localMatrix) const SK_OVERRIDE {
        if (localMatrix) {
            *localMatrix = fProxyLocalMatrix;
        }
        return SkRef(fProxyShader.get());
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLocalMatrixShader)

protected:
    SkLocalMatrixShader(SkReadBuffer&);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    virtual Context* onCreateContext(const ContextRec&, void*) const SK_OVERRIDE;

private:
    SkAutoTUnref<SkShader> fProxyShader;
    SkMatrix  fProxyLocalMatrix;

    typedef SkShader INHERITED;
};

SkLocalMatrixShader::SkLocalMatrixShader(SkReadBuffer& buffer) : INHERITED(buffer) {
    buffer.readMatrix(&fProxyLocalMatrix);
    fProxyShader.reset(buffer.readFlattenable<SkShader>());
    if (NULL == fProxyShader.get()) {
        sk_throw();
    }
}

void SkLocalMatrixShader::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fProxyLocalMatrix);
    buffer.writeFlattenable(fProxyShader.get());
}

SkShader::Context* SkLocalMatrixShader::onCreateContext(const ContextRec& rec,
                                                        void* storage) const {
    ContextRec newRec(rec);
    SkMatrix tmp;
    if (rec.fLocalMatrix) {
        tmp.setConcat(fProxyLocalMatrix, *rec.fLocalMatrix);
        newRec.fLocalMatrix = &tmp;
    } else {
        newRec.fLocalMatrix = &fProxyLocalMatrix;
    }
    return fProxyShader->createContext(newRec, storage);
}

#ifndef SK_IGNORE_TO_STRING
void SkLocalMatrixShader::toString(SkString* str) const {
    str->append("SkLocalMatrixShader: (");

    fProxyShader->toString(str);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

SkShader* SkShader::CreateLocalMatrixShader(SkShader* proxy, const SkMatrix& localMatrix) {
    if (localMatrix.isIdentity()) {
        return SkRef(proxy);
    }

    const SkMatrix* lm = &localMatrix;

    SkMatrix otherLocalMatrix;
    SkAutoTUnref<SkShader> otherProxy(proxy->refAsALocalMatrixShader(&otherLocalMatrix));
    if (otherProxy.get()) {
        otherLocalMatrix.preConcat(localMatrix);
        lm = &otherLocalMatrix;
        proxy = otherProxy.get();
    }

    return SkNEW_ARGS(SkLocalMatrixShader, (proxy, *lm));
}
