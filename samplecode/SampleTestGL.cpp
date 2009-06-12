#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkCornerPathEffect.h"
#include "SkCullPoints.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"

static void show_ramp(SkCanvas* canvas, const SkRect& r) {
    SkPoint pts[] = { r.fLeft, 0, r.fRight, 0 };
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                 SkShader::kRepeat_TileMode);
    SkPaint p;
    p.setShader(s)->unref();
    canvas->drawRect(r, p);
    canvas->translate(r.width() + SkIntToScalar(8), 0);
    p.setDither(true);
    canvas->drawRect(r, p);
}

class TestGLView : public SkView {
public:
	TestGLView() {
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "TestGL");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        drawBG(canvas);
        
        SkRect r;
        r.set(0, 0, 100, 100);
        
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        
        SkPaint paint;
        paint.setAntiAlias(false);
        paint.setColor(SK_ColorRED);
        
        canvas->drawRect(r, paint);

        canvas->translate(r.width() + SkIntToScalar(20), 0);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(r, paint);

        canvas->translate(r.width() + SkIntToScalar(20), 0);
        paint.setStrokeWidth(SkIntToScalar(5));
        canvas->drawRect(r, paint);
        
        canvas->translate(r.width() * 10/9, 0);
        show_ramp(canvas, r);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TestGLView; }
static SkViewRegister reg(MyFactory);

