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
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSweepGradient)

protected:
    void flatten(SkWriteBuffer& buffer) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;

    bool onIsRasterPipelineOnly(const SkMatrix&) const override { return true; }

private:
    const SkPoint  fCenter;
    const SkScalar fTBias,
                   fTScale;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
