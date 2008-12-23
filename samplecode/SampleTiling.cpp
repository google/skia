#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTypeface.h"

// effects
#include "SkGradientShader.h"
#include "SkUnitMappers.h"
#include "SkBlurDrawLooper.h"

static void makebm(SkBitmap* bm, SkBitmap::Config config, int w, int h) {
    bm->setConfig(config, w, h);
    bm->allocPixels();
    bm->eraseColor(0);
    
    SkCanvas    canvas(*bm);
    SkPoint     pts[] = { 0, 0, SkIntToScalar(w), SkIntToScalar(h) };
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;
    
    SkUnitMapper*   um = NULL;    

    um = new SkCosineMapper;
//    um = new SkDiscreteMapper(12);

    SkAutoUnref au(um);

    paint.setDither(true);
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, pos,
                SK_ARRAY_COUNT(colors), SkShader::kClamp_TileMode, um))->unref();
    canvas.drawPaint(paint);
}

static void setup(SkPaint* paint, const SkBitmap& bm, bool filter,
                  SkShader::TileMode tmx, SkShader::TileMode tmy) {
    SkShader* shader = SkShader::CreateBitmapShader(bm, tmx, tmy);
    paint->setShader(shader)->unref();
    paint->setFilterBitmap(filter);
}

static const SkBitmap::Config gConfigs[] = {
    SkBitmap::kARGB_8888_Config,
    SkBitmap::kRGB_565_Config,
    SkBitmap::kARGB_4444_Config
};
static const int gWidth = 32;
static const int gHeight = 32;

class TilingView : public SkView {
    SkBlurDrawLooper    fLooper;
public:
	TilingView()
            : fLooper(SkIntToScalar(1), SkIntToScalar(2), SkIntToScalar(2),
                      0x88000000) {
        for (int i = 0; i < SK_ARRAY_COUNT(gConfigs); i++) {
            makebm(&fTexture[i], gConfigs[i], gWidth, gHeight);
        }
    }

    SkBitmap    fTexture[SK_ARRAY_COUNT(gConfigs)];
	
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Tiling");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkRect r = { 0, 0, SkIntToScalar(gWidth*2), SkIntToScalar(gHeight*2) };

        static const char* gConfigNames[] = { "8888", "565", "4444" };
    
        static const bool           gFilters[] = { false, true };
        static const char*          gFilterNames[] = {     "point",                     "bilinear" };
    
        static const SkShader::TileMode gModes[] = { SkShader::kClamp_TileMode, SkShader::kRepeat_TileMode, SkShader::kMirror_TileMode };
        static const char*          gModeNames[] = {    "C",                    "R",                   "M" };

        SkScalar y = SkIntToScalar(24);
        SkScalar x = SkIntToScalar(10);

        for (int kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
            for (int ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
                SkPaint p;
                SkString str;
                p.setAntiAlias(true);
                p.setDither(true);
                p.setLooper(&fLooper);
                str.printf("[%s,%s]", gModeNames[kx], gModeNames[ky]);

                p.setTextAlign(SkPaint::kCenter_Align);
                canvas->drawText(str.c_str(), str.size(), x + r.width()/2, y, p);
                
                x += r.width() * 4 / 3;
            }
        }
        
        y += SkIntToScalar(16);

        for (int i = 0; i < SK_ARRAY_COUNT(gConfigs); i++) {
            for (int j = 0; j < SK_ARRAY_COUNT(gFilters); j++) {
                x = SkIntToScalar(10);
                for (int kx = 0; kx < SK_ARRAY_COUNT(gModes); kx++) {
                    for (int ky = 0; ky < SK_ARRAY_COUNT(gModes); ky++) {
                        SkPaint paint;
                        setup(&paint, fTexture[i], gFilters[j], gModes[kx], gModes[ky]);
                        paint.setDither(true);
                        
                        canvas->save();
                        canvas->translate(x, y);
                        canvas->drawRect(r, paint);
                        canvas->restore();
                        
                        x += r.width() * 4 / 3;
                    }
                }
                {
                    SkPaint p;
                    SkString str;
                    p.setAntiAlias(true);
                    p.setLooper(&fLooper);
                    str.printf("%s, %s", gConfigNames[i], gFilterNames[j]);
                    canvas->drawText(str.c_str(), str.size(), x, y + r.height() * 2 / 3, p);
                }

                y += r.height() * 4 / 3;
            }
        }
        
    #ifdef SK_RELEASE
        this->inval(NULL);
    #endif
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) {
        return this->INHERITED::onClick(click);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TilingView; }
static SkViewRegister reg(MyFactory);

