/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRadialGradient_DEFINED
#define SkRadialGradient_DEFINED

#include "SkGradientShaderPriv.h"

class SkRadialGradient : public SkGradientShaderBase {
public:
    SkRadialGradient(const SkPoint& center, SkScalar radius, const Descriptor&);

    class RadialGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        RadialGradientContext(const SkRadialGradient&, const ContextRec&);

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;

    private:
        typedef SkGradientShaderBase::GradientShaderBaseContext INHERITED;
    };

    GradientType asAGradient(GradientInfo* info) const override;
#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*,
                                                   const SkMatrix& viewM,
                                                   const SkMatrix*,
                                                   SkFilterQuality,
                                                   SkSourceGammaTreatment) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRadialGradient)

protected:
    SkRadialGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;
    size_t onContextSize(const ContextRec&) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

private:
    const SkPoint fCenter;
    const SkScalar fRadius;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
