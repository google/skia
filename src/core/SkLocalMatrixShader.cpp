/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLocalMatrixShader.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#endif

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkLocalMatrixShader::asFragmentProcessor(const AsFPArgs& args) const {
    SkMatrix tmp = this->getLocalMatrix();
    if (args.fLocalMatrix) {
        tmp.preConcat(*args.fLocalMatrix);
    }
    return as_SB(fProxyShader)->asFragmentProcessor(AsFPArgs(
        args.fContext, args.fViewMatrix, &tmp, args.fFilterQuality, args.fDstColorSpace));
}
#endif

sk_sp<SkFlattenable> SkLocalMatrixShader::CreateProc(SkReadBuffer& buffer) {
    SkMatrix lm;
    buffer.readMatrix(&lm);
    auto baseShader(buffer.readShader());
    if (!baseShader) {
        return nullptr;
    }
    return baseShader->makeWithLocalMatrix(lm);
}

void SkLocalMatrixShader::flatten(SkWriteBuffer& buffer) const {
    buffer.writeMatrix(this->getLocalMatrix());
    buffer.writeFlattenable(fProxyShader.get());
}

SkShaderBase::Context* SkLocalMatrixShader::onMakeContext(
    const ContextRec& rec, SkArenaAlloc* alloc) const
{
    ContextRec newRec(rec);
    SkMatrix tmp;
    if (rec.fLocalMatrix) {
        tmp.setConcat(*rec.fLocalMatrix, this->getLocalMatrix());
        newRec.fLocalMatrix = &tmp;
    } else {
        newRec.fLocalMatrix = &this->getLocalMatrix();
    }
    return as_SB(fProxyShader)->makeContext(newRec, alloc);
}

SkImage* SkLocalMatrixShader::onIsAImage(SkMatrix* outMatrix, enum TileMode* mode) const {
    SkMatrix imageMatrix;
    SkImage* image = fProxyShader->isAImage(&imageMatrix, mode);
    if (image && outMatrix) {
        // Local matrix must be applied first so it is on the right side of the concat.
        *outMatrix = SkMatrix::Concat(imageMatrix, this->getLocalMatrix());
    }

    return image;
}

bool SkLocalMatrixShader::onAppendStages(SkRasterPipeline* p,
                                         SkColorSpace* dst,
                                         SkArenaAlloc* scratch,
                                         const SkMatrix& ctm,
                                         const SkPaint& paint,
                                         const SkMatrix* localM) const {
    SkMatrix tmp;
    if (localM) {
        tmp.setConcat(*localM, this->getLocalMatrix());
    }
    return as_SB(fProxyShader)->appendStages(p, dst, scratch, ctm, paint,
                                             localM ? &tmp : &this->getLocalMatrix());
}

#ifndef SK_IGNORE_TO_STRING
void SkLocalMatrixShader::toString(SkString* str) const {
    str->append("SkLocalMatrixShader: (");

    as_SB(fProxyShader)->toString(str);

    this->INHERITED::toString(str);

    str->append(")");
}
#endif

sk_sp<SkShader> SkShader::makeWithLocalMatrix(const SkMatrix& localMatrix) const {
    if (localMatrix.isIdentity()) {
        return sk_ref_sp(const_cast<SkShader*>(this));
    }

    const SkMatrix* lm = &localMatrix;

    sk_sp<SkShader> baseShader;
    SkMatrix otherLocalMatrix;
    sk_sp<SkShader> proxy(as_SB(this)->makeAsALocalMatrixShader(&otherLocalMatrix));
    if (proxy) {
        otherLocalMatrix.preConcat(localMatrix);
        lm = &otherLocalMatrix;
        baseShader = proxy;
    } else {
        baseShader = sk_ref_sp(const_cast<SkShader*>(this));
    }

    return sk_make_sp<SkLocalMatrixShader>(std::move(baseShader), *lm);
}
