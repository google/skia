#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkNinePatch.h"
#include "SkPaint.h"
#include "SkUnPreMultiply.h"

class NinePatchView : public SkView {
public:
    SkBitmap fBM;

	NinePatchView() {
        SkImageDecoder::DecodeFile("/skimages/folder_background.9.png", &fBM);
        
        // trim off the edge guide-lines
        SkBitmap tmp;
        SkIRect r;
        r.set(1, 1, fBM.width() - 1, fBM.height() - 1);
        fBM.extractSubset(&tmp, r);
        fBM.swap(tmp);
        
        fX = SkIntToScalar(fBM.width());
        fY = 0;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "NinePatch");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        canvas->scale(1.5f, 1.5f);
        
        canvas->drawBitmap(fBM, 0, 0);
        
        SkIRect margins;
        SkRect  dst;
        int d = 25;
        
        margins.set(d, d, d, d);
        dst.set(0, 0, SkIntToScalar(200), SkIntToScalar(200));
        dst.offset(fX, fY);
        
        SkNinePatch::DrawNine(canvas, dst, fBM, margins);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fX = x / 1.5f;
        fY = y / 1.5f;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
private:
    SkScalar fX, fY;
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new NinePatchView; }
static SkViewRegister reg(MyFactory);

