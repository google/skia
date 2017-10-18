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
    static sk_sp<SkShader> Create(const SkPoint& start, SkScalar startRadius,
                                  const SkPoint& end, SkScalar endRadius,
                                  const Descriptor&);

    SkShader::GradientType asAGradient(GradientInfo* info) const  override;
#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const AsFPArgs&) const override;
#endif
    bool isOpaque() const override;

    SkScalar getCenterX1() const { return SkPoint::Distance(fCenter1, fCenter2); }
    SkScalar getStartRadius() const { return fRadius1; }
    SkScalar getDiffRadius() const { return fRadius2 - fRadius1; }
    const SkPoint& getStartCenter() const { return fCenter1; }
    const SkPoint& getEndCenter() const { return fCenter2; }
    SkScalar getEndRadius() const { return fRadius2; }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTwoPointConicalGradient)

protected:
    void flatten(SkWriteBuffer& buffer) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;

    bool onIsRasterPipelineOnly(const SkMatrix&) const override { return true; }

private:
    enum class Type {
        kRadial,
        kTwoPoint,
    };

    SkTwoPointConicalGradient(const SkPoint& c0, SkScalar r0,
                              const SkPoint& c1, SkScalar r1,
                              const Descriptor&, Type, const SkMatrix&);

    SkPoint  fCenter1;
    SkPoint  fCenter2;
    SkScalar fRadius1;
    SkScalar fRadius2;
    Type     fType;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
