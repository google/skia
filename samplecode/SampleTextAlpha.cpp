#include "SampleCode.h"
#include "SkView.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPorterDuff.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkImageRef.h"
#include "SkOSFile.h"
#include "SkStream.h"

static void check_for_nonwhite(const SkBitmap& bm, int alpha) {
    if (bm.config() != SkBitmap::kRGB_565_Config) {
        return;
    }
    
    for (int y = 0; y < bm.height(); y++) {
        for (int x = 0; x < bm.width(); x++) {
            uint16_t c = *bm.getAddr16(x, y);
            if (c != 0xFFFF) {
                SkDebugf("------ nonwhite alpha=%x [%d %d] %x\n", alpha, x, y, c);
                return;
            }
        }
    }
}

class TextAlphaView : public SkView {
public:    
	TextAlphaView() {
        fByte = 0xFF;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)  {
        if (SampleCode::TitleQ(*evt)) {
            SkString str("TextAlpha");
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        const char* str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        SkPaint paint;
        SkScalar    x = SkIntToScalar(10);
        SkScalar    y = SkIntToScalar(20);
        
        paint.setFlags(0x105);
        
        paint.setARGB(fByte, 0xFF, 0xFF, 0xFF);
        
        paint.setMaskFilter(SkBlurMaskFilter::Create(SkIntToScalar(3),
                                        SkBlurMaskFilter::kNormal_BlurStyle));
        paint.getMaskFilter()->unref();
        
        SkRandom rand;
        
        for (int ps = 6; ps <= 35; ps++) {
            paint.setColor(rand.nextU() | (0xFF << 24));
            paint.setTextSize(SkIntToScalar(ps));
            paint.setTextSize(SkIntToScalar(24));
            canvas->drawText(str, strlen(str), x, y, paint);
            y += paint.getFontMetrics(NULL);
        }
        //check_for_nonwhite(canvas->getDevice()->accessBitmap(), fByte);
        //SkDebugf("------ byte %x\n", fByte);

        if (false) {
            fByte += 1;
            fByte &= 0xFF;
            this->inval(NULL);
        }
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return new Click(this);
    }
    
    virtual bool onClick(Click* click) {
        int y = click->fICurr.fY;
        if (y < 0) {
            y = 0;
        } else if (y > 255) {
            y = 255;
        }
        fByte = y;
        this->inval(NULL);
        return true;
    }
    
private:
    int fByte;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TextAlphaView; }
static SkViewRegister reg(MyFactory);

