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
    bool isOpaque() const override;
    void flatten(SkWriteBuffer&) const override;
    bool onAppendStages(const SkStageRec&) const override;

    skvm::Color onProgram(skvm::Builder*, skvm::F32 x, skvm::F32 y, skvm::Color paint,
                          const SkMatrix& ctm, const SkMatrix* localM,
                          SkFilterQuality quality, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;

    SK_FLATTENABLE_HOOKS(SkColorFilterShader)

    sk_sp<SkShader>      fShader;
    sk_sp<SkColorFilter> fFilter;
    float                fAlpha;

    typedef SkShaderBase INHERITED;
};

#endif
