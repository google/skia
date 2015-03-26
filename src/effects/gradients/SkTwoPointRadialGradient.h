
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #ifndef SkTwoPointRadialGradient_DEFINED
 #define SkTwoPointRadialGradient_DEFINED

 #include "SkGradientShaderPriv.h"

class SkTwoPointRadialGradient : public SkGradientShaderBase {
public:
    SkTwoPointRadialGradient(const SkPoint& start, SkScalar startRadius,
                             const SkPoint& end, SkScalar endRadius,
                             const Descriptor&);

    virtual BitmapType asABitmap(SkBitmap* bitmap,
                                 SkMatrix* matrix,
                                 TileMode* xy) const override;
    GradientType asAGradient(GradientInfo* info) const override;
    virtual bool asFragmentProcessor(GrContext* context, const SkPaint&, const SkMatrix& viewM,
                                     const SkMatrix*, GrColor*,
                                     GrFragmentProcessor**)  const override;

    size_t contextSize() const override;

    class TwoPointRadialGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        TwoPointRadialGradientContext(const SkTwoPointRadialGradient&, const ContextRec&);

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) override;

    private:
        typedef SkGradientShaderBase::GradientShaderBaseContext INHERITED;
    };

    SkScalar getCenterX1() const { return fDiff.length(); }
    SkScalar getStartRadius() const { return fStartRadius; }
    SkScalar getDiffRadius() const { return fDiffRadius; }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTwoPointRadialGradient)

protected:
    SkTwoPointRadialGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;
    Context* onCreateContext(const ContextRec&, void* storage) const override;

private:
    const SkPoint fCenter1;
    const SkPoint fCenter2;
    const SkScalar fRadius1;
    const SkScalar fRadius2;
    SkPoint fDiff;
    SkScalar fStartRadius, fDiffRadius, fSr2D2, fA, fOneOverTwoA;

    friend class SkGradientShader;
    typedef SkGradientShaderBase INHERITED;
};

#endif
