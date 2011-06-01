#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"

class SpiralView : public SampleView {
public:
	SpiralView() {
        this->setBGColor(0xFFDDDDDD);
	}
	
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Spiral");
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
        
        SkRect r;
        SkScalar l,t,x,y;
        l = SampleCode::GetAnimScalar(SkIntToScalar(10),
                                      SkIntToScalar(400));
        t = SampleCode::GetAnimScalar(SkIntToScalar(5),
                                      SkIntToScalar(200));
        
        canvas->translate(320,240);
        for (int i = 0; i < 35; i++) {
            paint.setColor(0xFFF00FF0 - i * 0x04000000);
            SkScalar step = SK_ScalarPI / (55 - i);
            SkScalar angle = t * step;
            x = (20 + SkIntToScalar(i) * 5) * SkScalarSinCos(angle, &y);
            y *= (20 + SkIntToScalar(i) * 5);
            r.set(x, y, x + SkIntToScalar(10), y + SkIntToScalar(10));
            canvas->drawRect(r, paint);
        }

        this->inval(NULL);
    }
	
private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new SpiralView; }
static SkViewRegister reg(MyFactory);