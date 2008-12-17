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
        
        canvas->drawBitmap(fBM, 0, 0);
        
        SkIRect margins;
        SkRect  dst;
        int d = 25;
        
        margins.set(d, d, d, d);
        dst.set(0, 0, SkIntToScalar(200), SkIntToScalar(200));
        dst.offset(SkIntToScalar(fBM.width()), 0);
        dst.offset(SkIntToScalar(2), SkIntToScalar(2));
        
        SkNinePatch::DrawNine(canvas, dst, fBM, margins);
        
        int cx = fBM.width()/2;
        int cy = fBM.height()/2;
        SkPMColor pm = *fBM.getAddr32(cx, cy);
        SkColor c = SkUnPreMultiply::PMColorToColor(pm);
        SkColor pm2 = SkPreMultiplyColor(c);
        //SkDebugf("--- pm %x c %x pm2 %x\n", pm, c, pm2);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new NinePatchView; }
static SkViewRegister reg(MyFactory);

