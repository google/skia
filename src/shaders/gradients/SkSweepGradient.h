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
    SkSweepGradient(SkScalar cx, SkScalar cy, const Descriptor&);

    GradientType asAGradient(GradientInfo* info) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSweepGradient)

protected:
    void flatten(SkWriteBuffer& buffer) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

    bool adjustMatrixAndAppendStages(SkArenaAlloc* alloc,
                                     SkMatrix* matrix,
                                     SkRasterPipeline* tPipeline,
                                     SkRasterPipeline* postPipeline) const override;

    bool onIsRasterPipelineOnly() const override { return true; }

private:
    const SkPoint fCenter;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
