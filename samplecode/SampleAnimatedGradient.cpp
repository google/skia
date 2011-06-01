#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"

class GradientView : public SampleView {
public:
	GradientView() {
        this->setBGColor(0xFFDDDDDD);
	}
	
protected:
	struct GradData {
        int             fCount;
        const SkColor*  fColors;
        const SkScalar* fPos;
    };
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Gradients");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkScalarHalf(SkIntToScalar(3)));
        paint.setStyle(SkPaint::kFill_Style);
        
        SkPoint p = SkPoint::Make(0,0);
        SkPoint q = SkPoint::Make(100,100);
        SkPoint pts[] = {p, q};
        
        SkScalar t, temp, x, y;
        SkColor gColors[] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorWHITE, SK_ColorBLACK
        };
        t =    SampleCode::GetAnimScalar(SkIntToScalar(2), SkIntToScalar(20));
        temp = SampleCode::GetAnimScalar(SkIntToScalar(1), SkIntToScalar(8));
        SkScalar step = SK_ScalarPI / (10);
        SkScalar angle = t * step;
        x =  SkScalarSinCos(angle, &y);
        SkScalar colorPositions[] = { 0, 0.1 + x, 0.4 + y, 0.9 - x + y, 1.0};
        GradData data = { 5, gColors, colorPositions };
        
        
        SkRect r = { 0, 0, SkIntToScalar(200), SkIntToScalar(200) };
        SkShader* shader1 = SkGradientShader::CreateLinear(
                           pts, data.fColors, data.fPos,data.fCount, 
                           SkShader::kMirror_TileMode);
        paint.setShader(shader1)->unref();
        
        canvas->drawRect(r, paint);
        
        
        SkPoint s = SkPoint::Make(100,100);
        SkShader* shader2 = SkGradientShader::CreateRadial(
                           s, 100, data.fColors, data.fPos, data.fCount, 
                           SkShader::kMirror_TileMode);
        paint.setShader(shader2)->unref();
        canvas->translate(250, 0);
        canvas->drawRect(r, paint);
        
        SkShader* shader3 = SkGradientShader::CreateTwoPointRadial(
                           p, 0, q, 100, data.fColors, data.fPos, data.fCount,
                           SkShader::kMirror_TileMode);
        paint.setShader(shader3)->unref();
        canvas->translate(0, 250);
        canvas->drawRect(r, paint);
        
        SkShader* shader4 = SkGradientShader::CreateSweep(
                            100, 100, data.fColors, data.fPos, data.fCount);
 
        paint.setShader(shader4)->unref();
        canvas->translate(-250, 0);
        canvas->drawRect(r, paint);
        
        this->inval(NULL);		
    }
	
private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new GradientView; }
static SkViewRegister reg(MyFactory);