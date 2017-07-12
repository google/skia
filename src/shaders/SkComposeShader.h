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

class SkComposeShader final : public SkShaderBase {
public:
    SkComposeShader(sk_sp<SkShader> dst, sk_sp<SkShader> src, SkBlendMode mode, float lerpT)
        : fDst(std::move(dst))
        , fSrc(std::move(src))
        , fLerpT(lerpT)
        , fMode(mode)
    {
        SkASSERT(lerpT >= 0 && lerpT <= 1);
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

#ifdef SK_DEBUGx
    SkShader* getShaderA() { return fShaderA.get(); }
    SkShader* getShaderB() { return fShaderB.get(); }
#endif

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    bool asACompose(ComposeRec* rec) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeShader)

protected:
    SkComposeShader(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;
    bool onAppendStages(SkRasterPipeline*, SkColorSpace* dstCS, SkArenaAlloc*,
                        const SkMatrix&, const SkPaint&, const SkMatrix* localM) const override;

    bool onIsRasterPipelineOnly() const override { return true; }

private:
    sk_sp<SkShader>     fDst;
    sk_sp<SkShader>     fSrc;
    const float         fLerpT;
    const SkBlendMode   fMode;

    bool isJustMode() const { return fLerpT == 1; }
    bool isJustLerp() const { return fMode == SkBlendMode::kSrc; }

    typedef SkShaderBase INHERITED;
};

#endif
