/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLocalMatrixShader.h"

SkFlattenable* SkLocalMatrixShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    SkAutoTUnref<SkShader> shader(buffer.readShader());
    if (!shader.get()) {
        return nullptr;
    }
    return SkShader::CreateLocalMatrixShader(shader, lm);
}

void SkLocalMatrixShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeFlattenable(fProxyShader.get());
}

SkShader::Context* SkLocalMatrixShader::onCreateContext(const ContextRec& rec,
                                                        void* storage) const {
    ContextRec newRec(rec);
    SkMatrix tmp;
    if (rec.fLocalMatrix) {
        tmp.setConcat(*rec.fLocalMatrix, this->getLocalMatrix());
        newRec.fLocalMatrix = &tmp;
    } else {
        newRec.fLocalMatrix = &this->getLocalMatrix();
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
    if (nullptr == proxy) {
        return nullptr;
    }

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

    return new SkLocalMatrixShader(proxy, *lm);
}
