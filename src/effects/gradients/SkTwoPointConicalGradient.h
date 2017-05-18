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

// TODO(dominikg): Worth making it truly immutable (i.e. set values in constructor)?
// Should only be initialized once via init(). Immutable afterwards.
struct TwoPtRadial {
    enum {
        // This value is outside the range SK_FixedMin to SK_FixedMax.
        kDontDrawT  = 0x80000000
    };

    float   fCenterX, fCenterY;
    float   fDCenterX, fDCenterY;
    float   fRadius;
    float   fDRadius;
    float   fA;
    float   fRadius2;
    float   fRDR;
    bool    fFlipped;

    void init(const SkPoint& center0, SkScalar rad0,
              const SkPoint& center1, SkScalar rad1,
              bool flipped);

    static bool DontDrawT(SkFixed t) {
        return kDontDrawT == (uint32_t)t;
    }
};


class SkTwoPointConicalGradient : public SkGradientShaderBase {
    TwoPtRadial fRec;
public:
    SkTwoPointConicalGradient(const SkPoint& start, SkScalar startRadius,
                              const SkPoint& end, SkScalar endRadius,
                              bool flippedGrad, const Descriptor&);

    class TwoPointConicalGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        TwoPointConicalGradientContext(const SkTwoPointConicalGradient&, const ContextRec&);
        ~TwoPointConicalGradientContext() override {}

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;

    private:
        typedef SkGradientShaderBase::GradientShaderBaseContext INHERITED;
    };

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
    Context* onMakeContext(const ContextRec&, SkArenaAlloc*) const override;
    sk_sp<SkShader> onMakeColorSpace(SkColorSpaceXformer* xformer) const override;

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
