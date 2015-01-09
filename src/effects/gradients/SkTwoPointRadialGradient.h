
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
                                 TileMode* xy) const SK_OVERRIDE;
    GradientType asAGradient(GradientInfo* info) const SK_OVERRIDE;
    virtual bool asFragmentProcessor(GrContext* context, const SkPaint&, const SkMatrix& viewM,
                                     const SkMatrix*, GrColor*,
                                     GrFragmentProcessor**)  const SK_OVERRIDE;

    size_t contextSize() const SK_OVERRIDE;

    class TwoPointRadialGradientContext : public SkGradientShaderBase::GradientShaderBaseContext {
    public:
        TwoPointRadialGradientContext(const SkTwoPointRadialGradient&, const ContextRec&);

        void shadeSpan(int x, int y, SkPMColor dstC[], int count) SK_OVERRIDE;

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
    void flatten(SkWriteBuffer& buffer) const SK_OVERRIDE;
    Context* onCreateContext(const ContextRec&, void* storage) const SK_OVERRIDE;

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
