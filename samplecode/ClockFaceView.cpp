/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkStrokeRec.h"
#include "SkTypeface.h"

#include "SkGradientShader.h"
#include "SkBlurMaskFilter.h"

#include "Sk2DPathEffect.h"

class Dot2DPathEffect : public Sk2DPathEffect {
public:
    Dot2DPathEffect(SkScalar radius, const SkMatrix& matrix,
                    SkTDArray<SkPoint>* pts)
    : Sk2DPathEffect(matrix), fRadius(radius), fPts(pts) {}

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Dot2DPathEffect)
    class Registrar {
    public:
        Registrar() {
            SkFlattenable::Register("Dot2DPathEffect",
                                    Dot2DPathEffect::CreateProc,
                                    Dot2DPathEffect::GetFlattenableType());
        }
    };
protected:
    void begin(const SkIRect& uvBounds, SkPath* dst) const override {
        if (fPts) {
            fPts->reset();
        }
        this->INHERITED::begin(uvBounds, dst);
    }

    virtual void next(const SkPoint& loc, int u, int v,
                      SkPath* dst) const override {
        if (fPts) {
            *fPts->append() = loc;
        }
        dst->addCircle(loc.fX, loc.fY, fRadius);
    }

    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeMatrix(this->getMatrix());
        buffer.writeScalar(fRadius);
    }

private:
    SkScalar fRadius;
    SkTDArray<SkPoint>* fPts;

    typedef Sk2DPathEffect INHERITED;
};

static Dot2DPathEffect::Registrar gReg0;

sk_sp<SkFlattenable> Dot2DPathEffect::CreateProc(SkReadBuffer& buffer) {
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    return sk_make_sp<Dot2DPathEffect>(buffer.readScalar(), matrix, nullptr);
}

class InverseFillPE : public SkPathEffect {
public:
    InverseFillPE() {}
    virtual bool filterPath(SkPath* dst, const SkPath& src,
                            SkStrokeRec*, const SkRect*) const override {
        *dst = src;
        dst->setFillType(SkPath::kInverseWinding_FillType);
        return true;
    }

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override {
        str->appendf("InverseFillPE: ()");
    }
#endif

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(InverseFillPE)

private:
    typedef SkPathEffect INHERITED;
};

sk_sp<SkFlattenable> InverseFillPE::CreateProc(SkReadBuffer& buffer) {
    return sk_make_sp<InverseFillPE>();
}

static sk_sp<SkPathEffect> makepe(float interp, SkTDArray<SkPoint>* pts) {
    SkMatrix    lattice;
    SkScalar    rad = 3 + SkIntToScalar(4) * (1 - interp);
    lattice.setScale(rad*2, rad*2, 0, 0);
    lattice.postSkew(SK_Scalar1/3, 0, 0, 0);
    return sk_make_sp<Dot2DPathEffect>(rad, lattice, pts);
}

class ClockFaceView : public SkView {
    sk_sp<SkTypeface> fFace;
    SkScalar fInterp;
    SkScalar fDx;

public:
    ClockFaceView() {
        fFace = SkTypeface::MakeFromFile("/Users/reed/Downloads/p052024l.pfb");
        fInterp = 0;
        fDx = SK_Scalar1/64;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Text Effects");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
//        canvas->drawColor(0xFFDDDDDD);
        canvas->drawColor(SK_ColorWHITE);
    }

    static void drawdots(SkCanvas* canvas, const SkPaint& orig) {
        SkTDArray<SkPoint> pts;
        auto pe = makepe(0, &pts);

        SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
        SkPath path, dstPath;
        orig.getTextPath("9", 1, 0, 0, &path);
        pe->filterPath(&dstPath, path, &rec, nullptr);

        SkPaint p;
        p.setAntiAlias(true);
        p.setStrokeWidth(10);
        p.setColor(SK_ColorRED);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, pts.count(), pts.begin(), p);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkScalar    x = SkIntToScalar(20);
        SkScalar    y = SkIntToScalar(300);
        SkPaint     paint;

        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(240));
        paint.setTypeface(SkTypeface::MakeFromName("sans-serif", SkFontStyle::Bold()));

        SkString str("9");

        paint.setTypeface(fFace);

        canvas->drawString(str, x, y, paint);

    //    drawdots(canvas, paint);

        if (false) {
            fInterp += fDx;
            if (fInterp > 1) {
                fInterp = 1;
                fDx = -fDx;
            } else if (fInterp < 0) {
                fInterp = 0;
                fDx = -fDx;
            }
        }
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ClockFaceView; }
static SkViewRegister reg(MyFactory);
