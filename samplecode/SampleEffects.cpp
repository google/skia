#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkView.h"

#include "SkBlurMaskFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkDiscretePathEffect.h"
#include "SkGradientShader.h"

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
                     SkBlurMaskFilter::CreateEmboss(dir, 0.1, 0.05, 1))->unref();
}

static void paint_proc3(SkPaint* paint) {
    SkColor colors[] = { SK_ColorRED, COLOR, SK_ColorBLUE };
    SkPoint pts[] = { 3, 0, 7, 5 };
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

class EffectsView : public SkView {
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
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        canvas->scale(3, 3);
        canvas->translate(10, 30);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPaint); i++) {
            canvas->drawPath(fPath, fPaint[i]);
            canvas->translate(32, 0);
        }
    }
    
private:
    typedef SkView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new EffectsView; }
static SkViewRegister reg(MyFactory);

