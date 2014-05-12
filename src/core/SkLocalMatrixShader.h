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

#endif
