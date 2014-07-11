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

#if SK_SUPPORT_GPU
    
    virtual bool asNewEffect(GrContext* context, const SkPaint& paint, const SkMatrix* localMatrix,
                             GrColor* grColor, GrEffect** grEffect) const SK_OVERRIDE {
        SkMatrix tmp = this->getLocalMatrix();
        if (localMatrix) {
            tmp.preConcat(*localMatrix);
        }
        return fProxyShader->asNewEffect(context, paint, &tmp, grColor, grEffect);
    }
    
#else 
    
    virtual bool asNewEffect(GrContext* context, const SkPaint& paint, const SkMatrix* localMatrix,
                             GrColor* grColor, GrEffect** grEffect) const SK_OVERRIDE {
        SkDEBUGFAIL("Should not call in GPU-less build");
        return false;
    }
    
#endif
    
    virtual SkShader* refAsALocalMatrixShader(SkMatrix* localMatrix) const SK_OVERRIDE {
        if (localMatrix) {
            *localMatrix = this->getLocalMatrix();
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

    typedef SkShader INHERITED;
};

#endif
