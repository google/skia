#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkView.h"

#include "SkBlurMaskFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkDiscretePathEffect.h"
#include "SkGradientShader.h"

#include "SkEdgeClipper.h"

static void test_edgeclipper() {
    SkPoint pts[] = {
        { -8.38822452e+21f, -7.69721471e+19f },
        { 1.57645875e+23f, 1.44634003e+21f },
        { 1.61519691e+23f, 1.48208059e+21f },
        { 3.13963584e+23f, 2.88057438e+21f }
    };
    SkRect clip = { 0, 0, 300, 200 };
    
    SkEdgeClipper clipper;
    clipper.clipCubic(pts, clip);
}

///////////

//#define COLOR 0xFFFF8844
#define COLOR 0xFF888888

static void paint_proc0(SkPaint* paint) {
}

static void paint_proc1(SkPaint* paint) {
    paint->setMaskFilter(SkBlurMaskFilter::Create(2,
                                SkBlurMaskFilter::kNormal_BlurStyle))->unref();
}

static void paint_proc2(SkPaint* paint) {
    SkScalar dir[3] = { 1, 1, 1};
    paint->setMaskFilter(
                     SkBlurMaskFilter::CreateEmboss(dir, 0.1f, 0.05f, 1))->unref();
}

static void paint_proc3(SkPaint* paint) {
    SkColor colors[] = { SK_ColorRED, COLOR, SK_ColorBLUE };
    SkPoint pts[] = { { 3, 0 }, { 7, 5 } };
    paint->setShader(SkGradientShader::CreateLinear(pts, colors, NULL, SK_ARRAY_COUNT(colors),
                                        SkShader::kMirror_TileMode))->unref();
}

static void paint_proc5(SkPaint* paint) {
    paint_proc3(paint);
    paint_proc2(paint);
}

typedef void (*PaintProc)(SkPaint*);
const PaintProc gPaintProcs[] = {
    paint_proc0,
    paint_proc1,
    paint_proc2,
    paint_proc3,
    paint_proc5,
};

///////////////////////////////////////////////////////////////////////////////

class EffectsView : public SampleView {
public:
    SkPath fPath;
    SkPaint fPaint[SK_ARRAY_COUNT(gPaintProcs)];

	EffectsView() {
        size_t i;
        const float pts[] = {
            0, 0,
            10, 0,
            10, 5,
            20, -5,
            10, -15,
            10, -10,
            0, -10
        };
        fPath.moveTo(pts[0], pts[1]);
        for (i = 2; i < SK_ARRAY_COUNT(pts); i += 2) {
            fPath.lineTo(pts[i], pts[i+1]);
        }
        
        for (i = 0; i < SK_ARRAY_COUNT(gPaintProcs); i++) {
            fPaint[i].setAntiAlias(true);
            fPaint[i].setColor(COLOR);
            gPaintProcs[i](&fPaint[i]);
        }

        test_edgeclipper();
        SkColorMatrix cm;
        cm.setRotate(SkColorMatrix::kG_Axis, 180);
        cm.setIdentity();
        
        this->setBGColor(0xFFDDDDDD);
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Effects");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->scale(3, 3);
        canvas->translate(10, 30);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPaint); i++) {
            canvas->drawPath(fPath, fPaint[i]);
            canvas->translate(32, 0);
        }
    }
    
private:
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new EffectsView; }
static SkViewRegister reg(MyFactory);

