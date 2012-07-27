
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
                              const SkColor colors[], const SkScalar pos[],
                              int colorCount, SkShader::TileMode mode,
                              SkUnitMapper* mapper);

    virtual BitmapType asABitmap(SkBitmap* bitmap,
                                 SkMatrix* matrix,
                                 TileMode* xy) const SK_OVERRIDE;
    virtual GradientType asAGradient(GradientInfo* info) const SK_OVERRIDE;
    virtual GrCustomStage* asNewCustomStage(GrContext* context,
        GrSamplerState* sampler) const SK_OVERRIDE;

    virtual void shadeSpan(int x, int y, SkPMColor* dstCParam,
                           int count) SK_OVERRIDE;
    virtual bool setContext(const SkBitmap& device,
                            const SkPaint& paint,
                            const SkMatrix& matrix) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTwoPointRadialGradient)

protected:
    SkTwoPointRadialGradient(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE;

private:
    typedef SkGradientShaderBase INHERITED;
    const SkPoint fCenter1;
    const SkPoint fCenter2;
    const SkScalar fRadius1;
    const SkScalar fRadius2;
    SkPoint fDiff;
    SkScalar fStartRadius, fDiffRadius, fSr2D2, fA, fOneOverTwoA;

    void init();
};

///////////////////////////////////////////////////////////////////////////////

class GrGLRadial2Gradient;

class GrRadial2Gradient : public GrGradientEffect {

public:

    GrRadial2Gradient(GrTexture* texture, GrScalar center, GrScalar radius, bool posRoot);
    GrRadial2Gradient(GrContext* ctx, const SkShader& shader,
                      GrSamplerState* sampler, SkScalar center, 
                      SkScalar radius, SkScalar diffRadius);
    virtual ~GrRadial2Gradient();

    static const char* Name() { return "Two-Point Radial Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return GR_Scalar1 == fCenterX1; }
    GrScalar center() const { return fCenterX1; }
    GrScalar radius() const { return fRadius0; }
    bool isPosRoot() const { return SkToBool(fPosRoot); }

    typedef GrGLRadial2Gradient GLProgramStage;

private:

    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrScalar fCenterX1;
    GrScalar fRadius0;
    SkBool8  fPosRoot;

    // @}

    typedef GrGradientEffect INHERITED;
};

#endif

