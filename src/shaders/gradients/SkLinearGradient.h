/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearGradient_DEFINED
#define SkLinearGradient_DEFINED

#include "SkGradientShaderPriv.h"

class SkLinearGradient : public SkGradientShaderBase {
public:
    SkLinearGradient(const SkPoint pts[2], const Descriptor&);

    GradientType asAGradient(GradientInfo* info) const override;
#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

protected:
    SkLinearGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
    Context* onMakeBurstPipelineContext(const ContextRec&, SkArenaAlloc*) const override;
#endif

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const final;


    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

private:
    SK_FLATTENABLE_HOOKS(SkLinearGradient)

    class LinearGradient4fContext;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
    const SkPoint fStart;
    const SkPoint fEnd;
};

#endif
