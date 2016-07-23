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

class GrFragmentProcessor;

class SkLocalMatrixShader : public SkShader {
public:
    SkLocalMatrixShader(SkShader* proxy, const SkMatrix& localMatrix)
    : INHERITED(&localMatrix)
    , fProxyShader(SkRef(proxy))
    {}

    GradientType asAGradient(GradientInfo* info) const override {
        return fProxyShader->asAGradient(info);
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(
                                            GrContext* context, const SkMatrix& viewM,
                                            const SkMatrix* localMatrix, SkFilterQuality fq,
                                            SkSourceGammaTreatment gammaTreatment) const override;
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

    size_t onContextSize(const ContextRec& rec) const override {
        return fProxyShader->contextSize(rec);
    }

    bool onIsABitmap(SkBitmap* bitmap, SkMatrix* matrix, TileMode* mode) const override {
        return fProxyShader->isABitmap(bitmap, matrix, mode);
    }

private:
    SkAutoTUnref<SkShader> fProxyShader;

    typedef SkShader INHERITED;
};

#endif
