
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #ifndef SkTwoPointConicalGradient_DEFINED
 #define SkTwoPointConicalGradient_DEFINED

#include "SkGradientShaderPriv.h"

struct TwoPtRadial {
    enum {
        kDontDrawT  = 0x80000000
    };

    float   fCenterX, fCenterY;
    float   fDCenterX, fDCenterY;
    float   fRadius;
    float   fDRadius;
    float   fA;
    float   fRadius2;
    float   fRDR;

    void init(const SkPoint& center0, SkScalar rad0,
              const SkPoint& center1, SkScalar rad1);
    
    // used by setup and nextT
    float   fRelX, fRelY, fIncX, fIncY;
    float   fB, fDB;
    
    void setup(SkScalar fx, SkScalar fy, SkScalar dfx, SkScalar dfy);
    SkFixed nextT();
    
    static bool DontDrawT(SkFixed t) {
        return kDontDrawT == (uint32_t)t;
    }
};


class SkTwoPointConicalGradient : public SkGradientShaderBase {
    TwoPtRadial fRec;
    void init();

public:
    SkTwoPointConicalGradient(const SkPoint& start, SkScalar startRadius,
                              const SkPoint& end, SkScalar endRadius,
                              const SkColor colors[], const SkScalar pos[],
                              int colorCount, SkShader::TileMode mode,
                              SkUnitMapper* mapper);
    
    virtual void shadeSpan(int x, int y, SkPMColor* dstCParam,
                           int count) SK_OVERRIDE;
    virtual bool setContext(const SkBitmap& device,
                            const SkPaint& paint,
                            const SkMatrix& matrix) SK_OVERRIDE;

    virtual BitmapType asABitmap(SkBitmap* bitmap,
                                 SkMatrix* matrix,
                                 TileMode* xy) const;
    virtual SkShader::GradientType asAGradient(GradientInfo* info) const  SK_OVERRIDE;
    virtual GrCustomStage* asNewCustomStage(GrContext* context,
        GrSamplerState* sampler) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTwoPointConicalGradient)
    
protected:
    SkTwoPointConicalGradient(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE;
    
private:
    typedef SkGradientShaderBase INHERITED;
    const SkPoint fCenter1;
    const SkPoint fCenter2;
    const SkScalar fRadius1;
    const SkScalar fRadius2;
};

///////////////////////////////////////////////////////////////////////////////

class GrGLConical2Gradient;

class GrConical2Gradient : public GrGradientEffect {

public:

    GrConical2Gradient(GrTexture* texture, GrScalar center, GrScalar radius, GrScalar diffRadius);
    GrConical2Gradient(GrContext* ctx, const SkShader& shader,
                       GrSamplerState* sampler, SkScalar center, 
                       SkScalar radius, SkScalar diffRadius);
    virtual ~GrConical2Gradient();

    static const char* Name() { return "Two-Point Conical Gradient"; }
    virtual const GrProgramStageFactory& getFactory() const SK_OVERRIDE;
    virtual bool isEqual(const GrCustomStage&) const SK_OVERRIDE;

    // The radial gradient parameters can collapse to a linear (instead of quadratic) equation.
    bool isDegenerate() const { return SkScalarAbs(fDiffRadius) == SkScalarAbs(fCenterX1); }
    GrScalar center() const { return fCenterX1; }
    GrScalar diffRadius() const { return fDiffRadius; }
    GrScalar radius() const { return fRadius0; }

    typedef GrGLConical2Gradient GLProgramStage;

private:

    // @{
    // Cache of values - these can change arbitrarily, EXCEPT
    // we shouldn't change between degenerate and non-degenerate?!

    GrScalar fCenterX1;
    GrScalar fRadius0;
    GrScalar fDiffRadius;

    // @}

    typedef GrGradientEffect INHERITED;
};

#endif

