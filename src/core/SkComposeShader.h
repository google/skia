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

/** \class SkComposeShader
    This subclass of shader returns the composition of two other shaders, combined by
    a xfermode.
*/
class SK_API SkComposeShader : public SkShader {
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
        : fShaderA(std::move(sA))
        , fShaderB(std::move(sB))
        , fMode(mode)
    {}

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    class ComposeShaderContext : public SkShader::Context {
    public:
        // When this object gets destroyed, it will call contextA and contextB's destructor
        // but it will NOT free the memory.
        ComposeShaderContext(const SkComposeShader&, const ContextRec&,
                             SkShader::Context* contextA, SkShader::Context* contextB);

        void shadeSpan(int x, int y, SkPMColor[], int count) override;

    private:
        SkShader::Context* fShaderContextA;
        SkShader::Context* fShaderContextB;

        typedef SkShader::Context INHERITED;
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

private:
    sk_sp<SkShader>     fShaderA;
    sk_sp<SkShader>     fShaderB;
    SkBlendMode         fMode;

    typedef SkShader INHERITED;
};

#endif
