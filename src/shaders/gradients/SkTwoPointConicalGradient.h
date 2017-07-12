/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTwoPointConicalGradient_DEFINED
#define SkTwoPointConicalGradient_DEFINED

#include "SkColorSpaceXformer.h"
#include "SkGradientShaderPriv.h"

class SkTwoPointConicalGradient final : public SkGradientShaderBase {
public:
    SkTwoPointConicalGradient(const SkPoint& start, SkScalar startRadius,
                              const SkPoint& end, SkScalar endRadius,
                              bool flippedGrad, const Descriptor&);

    SkShader::GradientType asAGradient(GradientInfo* info) const  override;
#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif
    bool isOpaque() const override;

    SkScalar getCenterX1() const { return SkPoint::Distance(fCenter1, fCenter2); }
    SkScalar getStartRadius() const { return fRadius1; }
    SkScalar getDiffRadius() const { return fRadius2 - fRadius1; }
    const SkPoint& getStartCenter() const { return fCenter1; }
    const SkPoint& getEndCenter() const { return fCenter2; }
    SkScalar getEndRadius() const { return fRadius2; }
    bool isFlippedGrad() const { return fFlippedGrad; }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTwoPointConicalGradient)

protected:
    SkTwoPointConicalGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

    bool adjustMatrixAndAppendStages(SkArenaAlloc* alloc,
                                     SkMatrix* matrix,
                                     SkRasterPipeline* tPipeline,
                                     SkRasterPipeline* postPipeline) const override;

    bool onIsRasterPipelineOnly() const override { return true; }

private:
    SkPoint fCenter1;
    SkPoint fCenter2;
    SkScalar fRadius1;
    SkScalar fRadius2;
    bool fFlippedGrad;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
