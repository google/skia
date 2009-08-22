#include "SampleCode.h"
#include "SkColorPriv.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkUtils.h"

static SkBitmap make_bitmap() {
    SkBitmap bm;
    SkColorTable* ctable = new SkColorTable(256);

    SkPMColor* c = ctable->lockColors();
    for (int i = 0; i < 256; i++) {
        c[i] = SkPackARGB32(0xFF, i, 0, 0);
    }
    ctable->unlockColors(true);
    bm.setConfig(SkBitmap::kIndex8_Config, 256, 32);
    bm.allocPixels(ctable);
    ctable->unref();

    bm.lockPixels();
    for (int y = 0; y < bm.height(); y++) {
        uint8_t* p = bm.getAddr8(0, y);
        for (int x = 0; x < 256; x++) {
            p[x] = x;
        }
    }
    bm.unlockPixels();
    return bm;
}

class DitherBitmapView : public SkView {
    SkBitmap    fBM8;
    SkBitmap    fBM32;
public:
	DitherBitmapView() {
        fBM8 = make_bitmap();
        fBM8.copyTo(&fBM32, SkBitmap::kARGB_8888_Config);
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "DitherBitmap");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    static void setBitmapOpaque(SkBitmap* bm, bool isOpaque) {
        SkAutoLockPixels alp(*bm);  // needed for ctable
        bm->setIsOpaque(isOpaque);
        SkColorTable* ctable = bm->getColorTable();
        if (ctable) {
            ctable->setIsOpaque(isOpaque);
        }
    }
    
    static void draw2(SkCanvas* canvas, const SkBitmap& bm) {
        SkPaint paint;
        SkBitmap bitmap(bm);

        setBitmapOpaque(&bitmap, false);
        paint.setDither(false);
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        paint.setDither(true);
        canvas->drawBitmap(bitmap, 0, SkIntToScalar(bm.height() + 10), &paint);

        setBitmapOpaque(&bitmap, true);
        SkScalar x = SkIntToScalar(bm.width() + 10);
        paint.setDither(false);
        canvas->drawBitmap(bitmap, x, 0, &paint);
        paint.setDither(true);
        canvas->drawBitmap(bitmap, x, SkIntToScalar(bm.height() + 10), &paint);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        drawBG(canvas);
        
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        draw2(canvas, fBM8);
        canvas->translate(0, SkIntToScalar(fBM8.height() *3));
        draw2(canvas, fBM32);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DitherBitmapView; }
static SkViewRegister reg(MyFactory);

