/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSweepGradient_DEFINED
#define SkSweepGradient_DEFINED

#include "SkGradientShaderPriv.h"

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
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;

private:
    SK_FLATTENABLE_HOOKS(SkSweepGradient)

    const SkPoint  fCenter;
    const SkScalar fTBias,
                   fTScale;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
