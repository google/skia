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
    struct FocalData {
        SkScalar    fR0;
        SkScalar    fR1;
        bool        fIsSwapped;

        void set(SkScalar r0, SkScalar r1, SkMatrix& matrix);
    };

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

    // Whether the focal point (0, 0) is on the end circle with center (1, 0) and radius r1. If this
    // is true, it's as if an aircraft is flying at Mach 1 and all circles (soundwaves) will go
    // through the focal point (aircraft). In our previous implementations, this was known as the
    // edge case where the inside circle touches the outside circle (on the focal point). If we were
    // to solve for t bruteforcely using a quadratic equation, this case implies that the quadratic
    // equation degenerates to a linear equation.
    bool isFocalOnCircle() const { return SkScalarNearlyZero(1 - fFocalData.fR1); }
    bool isSwapped() const { return fFocalData.fIsSwapped; }

    // Whether the t we solved is always valid (so we don't need to check r(t) > 0).
    bool isWellBehaved() const { return !this->isFocalOnCircle() && fFocalData.fR1 > 1; }

    // Whether r0 == 0 so it's focal without any transformation
    bool isNativelyFocal() const { return SkScalarNearlyZero(fFocalData.fR0); }

    bool isRadiusIncreasing() const { return fFocalData.fR1 > fFocalData.fR0; }

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
        kStrip,
        kFocal
    };

    SkTwoPointConicalGradient(const SkPoint& c0, SkScalar r0,
                              const SkPoint& c1, SkScalar r1,
                              const Descriptor&, Type, const SkMatrix&, const FocalData&);

    SkPoint  fCenter1;
    SkPoint  fCenter2;
    SkScalar fRadius1;
    SkScalar fRadius2;
    Type     fType;

    FocalData fFocalData;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
