/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLocalMatrixShader_DEFINED
#define SkLocalMatrixShader_DEFINED

#include "SkShaderBase.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

class GrFragmentProcessor;
class SkArenaAlloc;
class SkColorSpaceXformer;

class SkLocalMatrixShader final : public SkShaderBase {
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

    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override {
        return as_SB(fProxyShader)->makeColorSpace(xformer)->makeWithLocalMatrix(
            this->getLocalMatrix());
    }

#ifdef SK_SUPPORT_LEGACY_SHADER_ISABITMAP
    bool onIsABitmap(SkBitmap* bitmap, SkMatrix* matrix, TileMode* mode) const override {
        return fProxyShader->isABitmap(bitmap, matrix, mode);
    }
#endif

    bool onIsRasterPipelineOnly() const override {
        return as_SB(fProxyShader)->isRasterPipelineOnly();
    }

private:
    sk_sp<SkShader> fProxyShader;

    typedef SkShaderBase INHERITED;
};

#endif
