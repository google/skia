#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

class PathClipView : public SampleView {
public:
    SkRect fOval;
    SkPoint fCenter;

	PathClipView() {
        fOval.set(0, 0, SkIntToScalar(200), SkIntToScalar(50));
        fCenter.set(SkIntToScalar(250), SkIntToScalar(250));
        
//        test_ats();
    }
    
    virtual ~PathClipView() {}
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PathClip");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
        SkRect oval = fOval;
        oval.offset(fCenter.fX - oval.centerX(), fCenter.fY - oval.centerY());
        
        SkPaint p;
        p.setAntiAlias(true);
        
        p.setStyle(SkPaint::kStroke_Style);
        canvas->drawOval(oval, p);

        SkRect r;
        r.set(SkIntToScalar(200), SkIntToScalar(200),
              SkIntToScalar(300), SkIntToScalar(300));
        canvas->clipRect(r);
        
        p.setStyle(SkPaint::kFill_Style);
        p.setColor(SK_ColorRED);
        canvas->drawRect(r, p);
     
        p.setColor(0x800000FF);
        r.set(SkIntToScalar(150), SkIntToScalar(10),
              SkIntToScalar(250), SkIntToScalar(400));
        canvas->drawOval(oval, p);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }
        
    virtual bool onClick(Click* click) {
        fCenter.set(click->fCurr.fX, click->fCurr.fY);
        this->inval(NULL);
        return NULL;
    }
    
private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PathClipView; }
static SkViewRegister reg(MyFactory);

