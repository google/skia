#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"

static SkBitmap createBitmap(int n) {
    SkBitmap bitmap;
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, n, n);
    bitmap.allocPixels();
    bitmap.eraseColor(0);
    
    SkCanvas canvas(bitmap);
    SkRect r;
    r.set(0, 0, SkIntToScalar(n), SkIntToScalar(n));
    SkPaint paint;
    paint.setAntiAlias(true);
    
    paint.setColor(SK_ColorRED);
    canvas.drawOval(r, paint);
    paint.setColor(SK_ColorBLUE);
    paint.setStrokeWidth(SkIntToScalar(n)/15);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas.drawLine(0, 0, r.fRight, r.fBottom, paint);
    canvas.drawLine(0, r.fBottom, r.fRight, 0, paint);
    
    return bitmap;
}

class MipMapView : public SkView {
    SkBitmap fBitmap;
    enum {
        N = 128
    };
public:
    MipMapView() {
        fBitmap = createBitmap(N);
        
        fWidth = N;
        fDW = -1;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "MapMaps");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawN(SkCanvas* canvas, const SkBitmap& bitmap) {
        SkAutoCanvasRestore acr(canvas, true);
        for (int i = N; i > 1; i >>= 1) {
            canvas->drawBitmap(bitmap, 0, 0, NULL);
            canvas->translate(SkIntToScalar(N + 8), 0);
            canvas->scale(SK_ScalarHalf, SK_ScalarHalf);
        }
    }
    
    void drawN2(SkCanvas* canvas, const SkBitmap& bitmap) {
        SkBitmap bg;
        bg.setConfig(SkBitmap::kARGB_8888_Config, N, N);
        bg.allocPixels();
        
        SkAutoCanvasRestore acr(canvas, true);
        for (int i = 0; i < N-2; i++) {
            bg.eraseColor(0);
            SkCanvas c(bg);
            c.scale(SK_Scalar1 / (1 << i), SK_Scalar1 / (1 << i));
            c.drawBitmap(bitmap, 0, 0, NULL);

            canvas->save();
            canvas->scale(SkIntToScalar(1 << i), SkIntToScalar(1 << i));
            canvas->drawBitmap(bg, 0, 0, NULL);
            canvas->restore();
            canvas->translate(SkIntToScalar(N + 8), 0);
        }
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));

        drawN2(canvas, fBitmap);

        canvas->translate(0, SkIntToScalar(N + 8));
        SkBitmap bitmap(fBitmap);
        bitmap.buildMipMap();
        drawN2(canvas, bitmap);
        
        fWidth += fDW;
        if (fDW > 0 && fWidth > N) {
            fDW = -fDW;
            fWidth = N;
        } else if (fDW < 0 && fWidth < 8) {
            fDW = -fDW;
            fWidth = 8;
        }

        SkRect dst;
        dst.set(0, 0, SkIntToScalar(fWidth), SkIntToScalar(fWidth));

        SkPaint paint;
        paint.setFilterBitmap(true);
        paint.setAntiAlias(true);

        canvas->translate(0, SkIntToScalar(N + 8));
        canvas->drawBitmapRect(fBitmap, NULL, dst, NULL);
        canvas->translate(SkIntToScalar(N + 8), 0);
        canvas->drawBitmapRect(fBitmap, NULL, dst, &paint);
        canvas->translate(SkIntToScalar(N + 8), 0);
        canvas->drawBitmapRect(bitmap, NULL, dst, NULL);
        canvas->translate(SkIntToScalar(N + 8), 0);
        canvas->drawBitmapRect(bitmap, NULL, dst, &paint);
        
        this->inval(NULL);
    }
    
private:
    int fWidth, fDW;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new MipMapView; }
static SkViewRegister reg(MyFactory);

