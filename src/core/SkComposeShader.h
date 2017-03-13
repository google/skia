/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkComposeShader_DEFINED
#define SkComposeShader_DEFINED

#include "SkShader.h"
#include "SkBlendMode.h"

///////////////////////////////////////////////////////////////////////////////////////////

class SkPairShader : public SkShader {
public:
    SkPairShader(sk_sp<SkShader> sA, sk_sp<SkShader> sB)
        : fShaderA(std::move(sA)), fShaderB(std::move(sB))
    {}

    class PairContext : public SkShader::Context {
    public:
        // When this object gets destroyed, it will call contextA and contextB's destructor
        // but it will NOT free the memory.
        PairContext(const SkPairShader& shader, const ContextRec& rec,
                    SkShader::Context* ctxA, SkShader::Context* ctxB)
            : INHERITED(shader, rec)
            , fContextA(ctxA)
            , fContextB(ctxB)
        {}

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

    protected:
        SkShader::Context* fContextA;
        SkShader::Context* fContextB;

    private:
        typedef SkShader::Context INHERITED;
    };

protected:
    sk_sp<SkShader> fShaderA;
    sk_sp<SkShader> fShaderB;

    void flatten(SkWriteBuffer&) const override;
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;

    // result is spanB, and the result
    virtual void mixSpans(SkPMColor[], const SkPMColor spanB[], int, uint8_t alpha) const = 0;

private:
    typedef SkShader INHERITED;
};

class SkComposeShader : public SkPairShader {
public:
    /** Create a new compose shader, given shaders A, B, and a combining xfermode mode.
        When the xfermode is called, it will be given the result from shader A as its
        "dst", and the result from shader B as its "src".
        mode->xfer32(sA_result, sB_result, ...)
        @param shaderA  The colors from this shader are seen as the "dst" by the xfermode
        @param shaderB  The colors from this shader are seen as the "src" by the xfermode
        @param mode     The xfermode that combines the colors from the two shaders. If mode
                        is null, then SRC_OVER is assumed.
    */
    SkComposeShader(sk_sp<SkShader> sA, sk_sp<SkShader> sB, SkBlendMode mode)
        : INHERITED(std::move(sA), std::move(sB))
        , fMode(mode)
    {}

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    bool asACompose(ComposeRec* rec) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeShader)

protected:
    void flatten(SkWriteBuffer&) const override;

    void mixSpans(SkPMColor result[], const SkPMColor spanB[], int count, uint8_t) const override;

private:
    SkBlendMode fMode;

    typedef SkPairShader INHERITED;
};

class SkMixerShader : public SkPairShader {
public:
    SkMixerShader(sk_sp<SkShader> sh0, sk_sp<SkShader> sh1, float weight)
        : INHERITED(std::move(sh0), std::move(sh1))
        , fWeight(weight)
    {
        SkASSERT(fShaderA && fShaderA);
        SkASSERT(fWeight >= 0 && fWeight <= 1);
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override;
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMixerShader)

protected:
    void flatten(SkWriteBuffer&) const override;

    void mixSpans(SkPMColor result[], const SkPMColor spanB[], int count, uint8_t) const override;

private:
    const float fWeight;
    
    typedef SkPairShader INHERITED;
};
    
#endif
