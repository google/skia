/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkComposeShader_DEFINED
#define SkComposeShader_DEFINED

#include "SkShaderBase.h"
#include "SkBlendMode.h"

class SkComposeShader : public SkShaderBase {
public:
    SkComposeShader(sk_sp<SkShader> sA, sk_sp<SkShader> sB, SkBlendMode mode, float lerpT)
        : fShaderA(std::move(sA))
        , fShaderB(std::move(sB))
        , fLerpT(lerpT)
        , fMode(mode)
    {
        SkASSERT(lerpT >= 0 && lerpT <= 1);
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    // Only used for isMode() == true
    //
    class ComposeShaderContext : public Context {
    public:
        // When this object gets destroyed, it will call contextA and contextB's destructor
        // but it will NOT free the memory.
        ComposeShaderContext(const SkComposeShader&, const ContextRec&,
                             SkShaderBase::Context* contextA, SkShaderBase::Context* contextB);

        void shadeSpan(int x, int y, SkPMColor[], int count) override;
        void shadeSpan4f(int x, int y, SkPM4f[], int count) override;

    private:
        SkShaderBase::Context* fShaderContextA;
        SkShaderBase::Context* fShaderContextB;

        typedef Context INHERITED;
    };

#ifdef SK_DEBUG
    SkShader* getShaderA() { return fShaderA.get(); }
    SkShader* getShaderB() { return fShaderB.get(); }
#endif

    bool asACompose(ComposeRec* rec) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeShader)

protected:
    SkComposeShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;
    bool onAppendStages(SkRasterPipeline*, SkColorSpace* dstCS, SkArenaAlloc*,
                        const SkMatrix&, const SkPaint&, const SkMatrix* localM) const override;

    bool isRasterPipelineOnly() const final;

private:
    sk_sp<SkShader>     fShaderA;
    sk_sp<SkShader>     fShaderB;
    const float         fLerpT;
    const SkBlendMode   fMode;

    bool isJustMode() const { return fLerpT == 1; }
    bool isJustLerp() const { return fMode == SkBlendMode::kSrc; }

    typedef SkShaderBase INHERITED;
};

#endif
