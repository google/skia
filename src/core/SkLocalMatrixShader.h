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

    virtual BitmapType asABitmap(SkBitmap* bitmap, SkMatrix* matrix,
                                 TileMode* mode) const override {
        return fProxyShader->asABitmap(bitmap, matrix, mode);
    }

    GradientType asAGradient(GradientInfo* info) const override {
        return fProxyShader->asAGradient(info);
    }

#if SK_SUPPORT_GPU

    virtual bool asFragmentProcessor(GrContext* context, const SkPaint& paint,
                                     const SkMatrix& viewM, const SkMatrix* localMatrix,
                                     GrColor* grColor, GrProcessorDataManager* procDataManager,
                                     GrFragmentProcessor** fp) const override {
        SkMatrix tmp = this->getLocalMatrix();
        if (localMatrix) {
            tmp.preConcat(*localMatrix);
        }
        return fProxyShader->asFragmentProcessor(context, paint, viewM, &tmp, grColor,
                                                 procDataManager, fp);
    }

#else

    virtual bool asFragmentProcessor(GrContext*, const SkPaint&, const SkMatrix&,
                                     const SkMatrix*, GrColor*, GrProcessorDataManager*,
                                     GrFragmentProcessor**) const override {
        SkDEBUGFAIL("Should not call in GPU-less build");
        return false;
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

private:
    SkAutoTUnref<SkShader> fProxyShader;

    typedef SkShader INHERITED;
};

#endif
