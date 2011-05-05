#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkShader.h"

static SkShader* createChecker() {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 2, 2);
    bm.allocPixels();
    bm.lockPixels();
    *bm.getAddr32(0, 0) = *bm.getAddr32(1, 1) = SkPreMultiplyColor(0xFFFFFFFF);
    *bm.getAddr32(0, 1) = *bm.getAddr32(1, 0) = SkPreMultiplyColor(0xFFCCCCCC);
    SkShader* s = SkShader::CreateBitmapShader(bm, SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode);

    SkMatrix m;
    m.setScale(12, 12);
    s->setLocalMatrix(m);
    return s;
}

static SkBitmap createBitmap(int n) {
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, n, n);
    bitmap.allocPixels();
    bitmap.eraseColor(0);
    
    SkCanvas canvas(bitmap);
    SkRect r;
    r.set(0, 0, SkIntToScalar(n), SkIntToScalar(n));
    r.inset(SK_Scalar1, SK_Scalar1);

    SkPaint paint;
    paint.setAntiAlias(true);
    
    paint.setColor(SK_ColorRED);
    canvas.drawOval(r, paint);

    r.inset(SK_Scalar1*n/4, SK_Scalar1*n/4);
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    paint.setColor(0x800000FF);
    canvas.drawOval(r, paint);
    
    return bitmap;
}

class ColorFilterView : public SampleView {
    SkBitmap fBitmap;
    SkShader* fShader;
    enum {
        N = 64
    };
public:
    ColorFilterView() {
        fBitmap = createBitmap(N);
        fShader = createChecker();
    }

    virtual ~ColorFilterView() {
        fShader->unref();
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ColorFilter");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawBackground(SkCanvas* canvas) {
        SkPaint paint;
        paint.setShader(fShader);
        canvas->drawPaint(paint);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        if (false) {
            SkPaint p;
            p.setAntiAlias(true);
            SkRect r = { 20.4f, 10, 20.6f, 20 };
            canvas->drawRect(r, p);
            r.set(30.9f, 10, 31.1f, 20);
            canvas->drawRect(r, p);
            return;
        }
        
        static const SkXfermode::Mode gModes[] = {
            SkXfermode::kSrc_Mode,
            SkXfermode::kDst_Mode,
            SkXfermode::kSrcIn_Mode,
            SkXfermode::kDstIn_Mode,
            SkXfermode::kSrcOut_Mode,
            SkXfermode::kDstOut_Mode,
            SkXfermode::kSrcATop_Mode,
            SkXfermode::kDstATop_Mode,
        };
    
        static const SkColor gColors[] = {
            0xFF000000,
            0x80000000,
            0xFF00FF00,
            0x8000FF00,
            0x00000000,
        };

        float scale = 1.5f;
        SkPaint paint;
        canvas->translate(N / 8, N / 8);

        for (size_t y = 0; y < SK_ARRAY_COUNT(gColors); y++) {
            for (size_t x = 0; x < SK_ARRAY_COUNT(gModes); x++) {
                SkColorFilter* cf = SkColorFilter::CreateModeFilter(gColors[y], gModes[x]);
                SkSafeUnref(paint.setColorFilter(cf));
                canvas->drawBitmap(fBitmap, x * N * 1.25f, y * N * scale, &paint);
            }
        }
        
    }
    
private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ColorFilterView; }
static SkViewRegister reg(MyFactory);

