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

class SkColorSpacXformer;

///////////////////////////////////////////////////////////////////////////////////////////

/*
 *  One class with two combining behaviors: either apply a blendmode or lerp
 *
 *  We store both values (mode and lerp), but only some modes are illegal, so we use one of them
 *  as a sentinel to know that we're in the lerp case. We use kSrc, since the factory will see
 *  kSrc and just return one of the shader parameters, rather than create a composit shader.
 */
class SkComposeShader : public SkShaderBase {
public:
    SkComposeShader(sk_sp<SkShader> sA, sk_sp<SkShader> sB, SkBlendMode mode)
        : fShaderA(std::move(sA))
        , fShaderB(std::move(sB))
        , fLerpT(GetIllegalLerp())
        , fMode(mode)
    {
        SkASSERT(IsLegalMode(mode));
    }

    SkComposeShader(sk_sp<SkShader> sA, sk_sp<SkShader> sB, float lerpT)
        : fShaderA(std::move(sA))
        , fShaderB(std::move(sB))
        , fLerpT(lerpT)
        , fMode(GetIllegalMode())
    {
        SkASSERT(IsLegalLerp(lerpT));
    }

    // Helpers to distinguish between BlendMode and Lerp, since we store both but only use
    // one or the other.

    static SkBlendMode GetIllegalMode() { return SkBlendMode::kSrc; }
    static float GetIllegalLerp() { return 0; }

    static bool IsLegalMode(SkBlendMode mode) {
        return mode != SkBlendMode::kSrc && mode != SkBlendMode::kDst;
    }

    static bool IsLegalLerp(float t) {
        return t > 0 && t < 1;
    }

    bool isMode() const { return fMode != GetIllegalMode(); }
    bool isLerp() const { return fMode == GetIllegalMode(); }


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

    typedef SkShaderBase INHERITED;
};

#endif
