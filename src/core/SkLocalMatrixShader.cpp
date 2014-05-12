/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLocalMatrixShader.h"

SkLocalMatrixShader::SkLocalMatrixShader(SkReadBuffer& buffer) : INHERITED(buffer) {
    buffer.readMatrix(&fProxyLocalMatrix);
    fProxyShader.reset(buffer.readShader());
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
