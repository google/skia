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

#include "SkOSFile.h"
#include "SkStream.h"

static SkBitmap make_bitmap() {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 64, 64);
    bm.allocPixels();
    SkCanvas canvas(bm);
    canvas.drawColor(SK_ColorRED);
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint pts[] = { 0, 0, 64, 64 };
    const SkColor colors[] = { SK_ColorWHITE, SK_ColorBLUE };
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                       SkShader::kClamp_TileMode))->unref();
    canvas.drawCircle(32, 32, 32, paint);
    return bm;
}

class BitmapRectView : public SkView {
public:
    SkBitmap fBitmap;

	BitmapRectView() {
        fBitmap = make_bitmap();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "BitmapRect");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorGRAY);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        const SkIRect src[] = {
            { 0, 0, 32, 32 },
            { 0, 0, 80, 80 },
            { 32, 32, 96, 96 },
            { -32, -32, 32, 32, }
        };

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorGREEN);

        SkRect dstR = { 0, 200, 128, 380 };

        canvas->translate(16, 40);
        for (size_t i = 0; i < SK_ARRAY_COUNT(src); i++) {
            SkRect srcR;
            srcR.set(src[i]);
            
            canvas->drawBitmap(fBitmap, 0, 0, &paint);
            canvas->drawBitmapRect(fBitmap, &src[i], dstR, &paint);

            canvas->drawRect(srcR, paint);
            canvas->drawRect(dstR, paint);
            
            canvas->translate(160, 0);
        }
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BitmapRectView; }
static SkViewRegister reg(MyFactory);

