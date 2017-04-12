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
class SkArenaAlloc;

class SkLocalMatrixShader : public SkShader {
public:
    SkLocalMatrixShader(sk_sp<SkShader> proxy, const SkMatrix& localMatrix)
    : INHERITED(&localMatrix)
    , fProxyShader(std::move(proxy))
    {}

    GradientType asAGradient(GradientInfo* info) const override {
        return fProxyShader->asAGradient(info);
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    sk_sp<SkShader> makeAsALocalMatrixShader(SkMatrix* localMatrix) const override {
        if (localMatrix) {
            *localMatrix = this->getLocalMatrix();
        }
        return fProxyShader;
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLocalMatrixShader)

protected:
    void flatten(SkWriteBuffer&) const override;

    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;

    SkImage* onIsAImage(SkMatrix* matrix, TileMode* mode) const override;

    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        const SkMatrix&, const SkPaint&, const SkMatrix*) const override;

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    bool onIsABitmap(SkBitmap* bitmap, SkMatrix* matrix, TileMode* mode) const override {
        return fProxyShader->isABitmap(bitmap, matrix, mode);
    }
#endif

private:
    sk_sp<SkShader> fProxyShader;

    typedef SkShader INHERITED;
};

#endif
