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
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TestGLView; }
static SkViewRegister reg(MyFactory);

