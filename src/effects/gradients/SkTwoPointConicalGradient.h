
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
    virtual GrEffectRef* asNewEffect(GrContext* context, const SkPaint& paint) const SK_OVERRIDE;
    virtual bool isOpaque() const SK_OVERRIDE;

    SkScalar getCenterX1() const { return SkPoint::Distance(fCenter1, fCenter2); }
    SkScalar getStartRadius() const { return fRadius1; }
    SkScalar getDiffRadius() const { return fRadius2 - fRadius1; }

    SK_DEVELOPER_TO_STRING()
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

#endif
