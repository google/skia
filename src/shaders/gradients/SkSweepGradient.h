/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSweepGradient_DEFINED
#define SkSweepGradient_DEFINED

#include "src/shaders/gradients/SkGradientShaderPriv.h"

class SkSweepGradient final : public SkGradientShaderBase {
public:
    SkSweepGradient(const SkPoint& center, SkScalar t0, SkScalar t1, const Descriptor&);

    GradientType asAGradient(GradientInfo* info) const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

    SkScalar getTBias() const { return fTBias; }

    SkScalar getTScale() const { return fTScale; }

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;

    skvm::F32 transformT(skvm::Builder*, skvm::Uniforms*,
                         skvm::F32 x, skvm::F32 y, skvm::I32* mask) const final;
private:
    SK_FLATTENABLE_HOOKS(SkSweepGradient)

    const SkPoint  fCenter;
    const SkScalar fTBias,
                   fTScale;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
