#include "SampleCode.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkUtils.h"

static void draw_rect(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    canvas->drawRect(r, p);

    SkPaint frame(p);
    frame.setShader(NULL);
    frame.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(r, frame);
}

static void draw_gradient(SkCanvas* canvas) {
    SkRect r = { 0, 0, SkIntToScalar(256), SkIntToScalar(32) };
    SkPoint pts[] = { { r.fLeft, r.fTop }, { r.fRight, r.fTop } };
    SkColor colors[] = { 0xFF000000, 0xFFFF0000 };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                 SkShader::kClamp_TileMode);

    SkPaint p;
    p.setShader(s)->unref();
    draw_rect(canvas, r, p);

    canvas->translate(0, SkIntToScalar(40));
    p.setDither(true);
    draw_rect(canvas, r, p);
}

static void test_pathregion() {
    SkPath path;
    SkRegion region;
    path.moveTo(25071800.f, -141823808.f); 
    path.lineTo(25075500.f, -141824000.f);
    path.lineTo(25075400.f, -141827712.f);
    path.lineTo(25071810.f, -141827600.f);
    path.close();

    SkIRect bounds;
    path.getBounds().round(&bounds);
    SkRegion clip(bounds);
    bool result = region.setPath(path, clip); // <-- !! DOWN !!
    SkDebugf("----- result %d\n", result);
}

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

class DitherBitmapView : public SampleView {
    SkBitmap    fBM8;
    SkBitmap    fBM32;
public:
	DitherBitmapView() {
        test_pathregion();
        fBM8 = make_bitmap();
        fBM8.copyTo(&fBM32, SkBitmap::kARGB_8888_Config);
        
        this->setBGColor(0xFFDDDDDD);
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
    
    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        draw2(canvas, fBM8);
        canvas->translate(0, SkIntToScalar(fBM8.height() *3));
        draw2(canvas, fBM32);

        canvas->translate(0, SkIntToScalar(fBM8.height() *3));
        draw_gradient(canvas);
    }
    
private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DitherBitmapView; }
static SkViewRegister reg(MyFactory);

