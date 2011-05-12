#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "Sk1DPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkDither.h"

static void make_bm(SkBitmap* bm) {
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN,
        SK_ColorBLUE, SK_ColorWHITE
    };
    SkColorTable* ctable = new SkColorTable(colors, 4);
    bm->setConfig(SkBitmap::kIndex8_Config, 2, 2);
    bm->allocPixels(ctable);
    ctable->unref();
    
    *bm->getAddr8(0, 0) = 0;
    *bm->getAddr8(1, 0) = 1;
    *bm->getAddr8(0, 1) = 2;
    *bm->getAddr8(1, 1) = 3;
}

static SkScalar draw_bm(SkCanvas* canvas, const SkBitmap& bm,
                        SkScalar x, SkScalar y, SkPaint* paint) {
#if 1
    canvas->drawBitmap(bm, x, y, paint);
    return SkIntToScalar(bm.width()) * 5/4;
#else
    SkAutoCanvasRestore acr(canvas, true);
    canvas->translate(x, y);

    SkScalar w = SkIntToScalar(bm.width());
    SkScalar h = SkIntToScalar(bm.height());
    SkShader* s = SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode);
    paint->setShader(s)->unref();
    canvas->drawRect(SkRect::MakeWH(w, h), *paint);
    paint->setShader(NULL);
    return w * 5/4;
#endif
}

static SkScalar draw_set(SkCanvas* c, const SkBitmap& bm, SkScalar x, SkPaint* p) {
    x += draw_bm(c, bm, x, 0, p);
    p->setFilterBitmap(true);
    x += draw_bm(c, bm, x, 0, p);
    p->setDither(true);
    return x + draw_bm(c, bm, x, 0, p);
}

static const char* gConfigNames[] = {
    "unknown config",
    "A1",
    "A8",
    "Index8",
    "565",
    "4444",
    "8888"
};

static SkScalar draw_row(SkCanvas* canvas, const SkBitmap& bm) {
    SkAutoCanvasRestore acr(canvas, true);

    SkPaint paint;
    SkScalar x = 0;
    const int scale = 32;

    paint.setAntiAlias(true);
    const char* name = gConfigNames[bm.config()];
    canvas->drawText(name, strlen(name), x, SkIntToScalar(bm.height())*scale*5/8,
                     paint);
    canvas->translate(SkIntToScalar(48), 0);

    canvas->scale(SkIntToScalar(scale), SkIntToScalar(scale));
    
    x += draw_set(canvas, bm, 0, &paint);
    paint.reset();
    paint.setAlpha(0x80);
    draw_set(canvas, bm, x, &paint);
    return x * scale / 3;
}

class FilterView : public SampleView {
public:
    SkBitmap    fBM8, fBM4444, fBM16, fBM32;

	FilterView() {
        make_bm(&fBM8);
        fBM8.copyTo(&fBM4444, SkBitmap::kARGB_4444_Config);
        fBM8.copyTo(&fBM16, SkBitmap::kRGB_565_Config);
        fBM8.copyTo(&fBM32, SkBitmap::kARGB_8888_Config);
        
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Filter");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(10);
        
        canvas->translate(x, y);
        y = draw_row(canvas, fBM8);
        canvas->translate(0, y);
        y = draw_row(canvas, fBM4444);
        canvas->translate(0, y);
        y = draw_row(canvas, fBM16);
        canvas->translate(0, y);
        draw_row(canvas, fBM32);
    }
    
private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FilterView; }
static SkViewRegister reg(MyFactory);

