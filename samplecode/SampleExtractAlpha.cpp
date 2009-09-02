#include "SampleCode.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkUtils.h"

static SkBitmap make_bitmap() {
    SkBitmap bm;
    SkColorTable* ctable = new SkColorTable(256);

    SkPMColor* c = ctable->lockColors();
    for (int i = 0; i < 256; i++) {
        c[i] = SkPackARGB32(255 - i, 0, 0, 0);
    }
    ctable->unlockColors(true);
    bm.setConfig(SkBitmap::kIndex8_Config, 256, 256);
    bm.allocPixels(ctable);
    ctable->unref();

    bm.lockPixels();
    const float cx = bm.width() * 0.5f;
    const float cy = bm.height() * 0.5f;
    for (int y = 0; y < bm.height(); y++) {
        float dy = y - cy;
        dy *= dy;
        uint8_t* p = bm.getAddr8(0, y);
        for (int x = 0; x < 256; x++) {
            float dx = x - cx;
            dx *= dx;
            float d = (dx + dy) / (cx/2);
            int id = (int)d;
            if (id > 255) {
                id = 255;
            }
            p[x] = id;
        }
    }
    bm.unlockPixels();
    return bm;
}

class ExtractAlphaView : public SkView {
    SkBitmap    fBM8;
    SkBitmap    fBM32;
    SkBitmap    fBM4;
public:
	ExtractAlphaView() {
        fBM8 = make_bitmap();
        fBM8.copyTo(&fBM32, SkBitmap::kARGB_8888_Config);
        fBM8.copyTo(&fBM4, SkBitmap::kARGB_4444_Config);
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
    
    virtual void onDraw(SkCanvas* canvas) {
        drawBG(canvas);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        SkMatrix matrix;
        matrix.setScale(3.55f, 80.f);
        canvas->setMatrix(matrix);
        
        paint.setStrokeWidth(0.0588f);
        canvas->drawLine(10, 5, 30, 4.8f, paint);
        
        paint.setStrokeWidth(0.06f);
        canvas->drawLine(20, 5, 40, 4.8f, paint);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ExtractAlphaView; }
static SkViewRegister reg(MyFactory);

