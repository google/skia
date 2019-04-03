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
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

    sk_sp<SkShader> makeAsALocalMatrixShader(SkMatrix* localMatrix) const override {
        if (localMatrix) {
            *localMatrix = this->getLocalMatrix();
        }
        return fProxyShader;
    }

protected:
    void flatten(SkWriteBuffer&) const override;

#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

    SkImage* onIsAImage(SkMatrix* matrix, SkTileMode* mode) const override;

    bool onAppendStages(const SkStageRec&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkLocalMatrixShader)

    sk_sp<SkShader> fProxyShader;

    typedef SkShaderBase INHERITED;
};

#endif
