/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLocalMatrixShader_DEFINED
#define SkLocalMatrixShader_DEFINED

#include "SkShader.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

class SkLocalMatrixShader : public SkShader {
public:
    SkLocalMatrixShader(SkShader* proxy, const SkMatrix& localMatrix)
    : INHERITED(&localMatrix)
    , fProxyShader(SkRef(proxy))
    {}

    size_t contextSize() const override {
        return fProxyShader->contextSize();
    }

    GradientType asAGradient(GradientInfo* info) const override {
        return fProxyShader->asAGradient(info);
    }

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* asFragmentProcessor(GrContext* context, const SkMatrix& viewM,
                                                   const SkMatrix* localMatrix,
                                                   SkFilterQuality fq) const override {
        SkMatrix tmp = this->getLocalMatrix();
        if (localMatrix) {
            tmp.preConcat(*localMatrix);
        }
        return fProxyShader->asFragmentProcessor(context, viewM, &tmp, fq);
    }
#endif

    SkShader* refAsALocalMatrixShader(SkMatrix* localMatrix) const override {
        if (localMatrix) {
            *localMatrix = this->getLocalMatrix();
        }
        return SkRef(fProxyShader.get());
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLocalMatrixShader)

protected:
    void flatten(SkWriteBuffer&) const override;
    Context* onCreateContext(const ContextRec&, void*) const override;

    bool onIsABitmap(SkBitmap* bitmap, SkMatrix* matrix, TileMode* mode) const override {
        return fProxyShader->isABitmap(bitmap, matrix, mode);
    }

private:
    SkAutoTUnref<SkShader> fProxyShader;

    typedef SkShader INHERITED;
};

#endif
