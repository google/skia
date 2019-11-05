/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterShader_DEFINED
#define SkColorFilterShader_DEFINED

#include "include/core/SkColorFilter.h"
#include "src/shaders/SkShaderBase.h"

class SkArenaAlloc;

class SkColorFilterShader : public SkShaderBase {
public:
    SkColorFilterShader(sk_sp<SkShader> shader, float alpha, sk_sp<SkColorFilter> filter);

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

private:
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec&) const override;

    bool onProgram(skvm::Builder*,
                   SkColorSpace* dstCS,
                   skvm::Arg uniforms, SkTDArray<uint32_t>*,
                   skvm::F32 x, skvm::F32 y,
                   skvm::I32* r, skvm::I32* g, skvm::I32* b, skvm::I32* a) const override;

    SK_FLATTENABLE_HOOKS(SkColorFilterShader)

    sk_sp<SkShader>      fShader;
    sk_sp<SkColorFilter> fFilter;
    float                fAlpha;

    typedef SkShaderBase INHERITED;
};

#endif
