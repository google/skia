#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPackBits.h"
#include "SkPath.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTypeface.h"
#include "SkAvoidXfermode.h"

#define REPEAT_COUNT    1

static const char gText[] = "Hamburgefons";

static bool gDevKern;

static void rand_text(char text[], SkRandom& rand, size_t count) {
    for (size_t i = 0; i < count; i++) {
        text[i] = rand.nextU() & 0x7F;
    }
}

static SkScalar sum_widths(const SkScalar widths[], int count) {
    SkScalar w = 0;
    for (int i = 0; i < count; i++) {
        w += widths[i];
    }
    return w;
}

static void test_measure(const SkPaint& paint) {
    char        text[256];
    SkScalar    widths[256];
    SkRect      rects[256];
    SkRect      bounds;
    int         count = 256;
    
    SkRandom rand;
    
    for (int i = 0; i < 100; i++) {
        rand_text(text, rand, 256);
        paint.getTextWidths(text, count, widths, NULL);
        SkScalar tw0 = sum_widths(widths, count);
        paint.getTextWidths(text, count, widths, rects);
        SkScalar tw1 = sum_widths(widths, count);
        SkASSERT(tw0 == tw1);

        SkScalar w0 = paint.measureText(text, count, NULL);
        SkScalar w1 = paint.measureText(text, count, &bounds);
        SkASSERT(w0 == w1);
        SkASSERT(w0 == tw0);
        
        SkRect r = rects[0];
        SkScalar x = 0;
        for (int j = 1; j < count; j++) {
            x += widths[j-1];
            rects[j].offset(x, 0);
            r.join(rects[j]);
        }
        SkASSERT(r == bounds);
        
        if (r != bounds) {
            printf("flags=%x i=%d [%g %g %g %g] [%g %g %g %g]\n",
                   paint.getFlags(), i,
                   SkScalarToFloat(r.fLeft),
                   SkScalarToFloat(r.fTop),
                   SkScalarToFloat(r.fRight),
                   SkScalarToFloat(r.fBottom),
                   SkScalarToFloat(bounds.fLeft),
                   SkScalarToFloat(bounds.fTop),
                   SkScalarToFloat(bounds.fRight),
                   SkScalarToFloat(bounds.fBottom));
        }
    }
}

static void test_measure() {
    SkPaint paint;
    
    for (int i = 0; i <= SkPaint::kAllFlags; i++) {
        paint.setFlags(i);
        test_measure(paint);
    }
}

//////////////////////////////////////////////////////////////////////////////

static void test_textBounds(SkCanvas* canvas) {
//    canvas->scale(SK_Scalar1/2, SK_Scalar1/2);
    
//    canvas->rotate(SkIntToScalar(30));

    gDevKern = !gDevKern;

    SkScalar x = SkIntToScalar(50);
    SkScalar y = SkIntToScalar(150);
    SkScalar w[100];
    SkRect   r[100], bounds;
    
    SkPaint paint;
    paint.setTextSize(SkIntToScalar(64));
    paint.setAntiAlias(true);
    paint.setDevKernText(gDevKern);
    
    (void)paint.measureText(gText, strlen(gText), &bounds, NULL);
    paint.setColor(SK_ColorGREEN);
    bounds.offset(x, y);
    canvas->drawRect(bounds, paint);

    int count = paint.getTextWidths(gText, strlen(gText), w, r);

    paint.setColor(SK_ColorRED);
    for (int i = 0; i < count; i++) {
        r[i].offset(x, y);
        canvas->drawRect(r[i], paint);
        x += w[i];
    }
    x = SkIntToScalar(50);
    paint.setColor(gDevKern ? SK_ColorDKGRAY : SK_ColorBLACK);
    canvas->drawText(gText, strlen(gText), x, y, paint);
}

static void create_src(SkBitmap* bitmap, SkBitmap::Config config) {
    bitmap->setConfig(config, 100, 100);
    bitmap->allocPixels();
    bitmap->eraseColor(0);
    
    SkCanvas    canvas(*bitmap);
    SkPaint     paint;

    paint.setAntiAlias(true);
    canvas.drawCircle(SkIntToScalar(50), SkIntToScalar(50),
                      SkIntToScalar(50), paint);
}

static void blur(SkBitmap* dst, const SkBitmap& src, SkScalar radius) {
    *dst = src;
}

static void test_bitmap_blur(SkCanvas* canvas) {
    SkBitmap    src, dst;
    
    create_src(&src, SkBitmap::kARGB_8888_Config);
    blur(&dst, src, SkIntToScalar(4));
    
    SkPaint paint;
    
    paint.setColor(SK_ColorRED);

    canvas->drawBitmap(dst, SkIntToScalar(30), SkIntToScalar(60), &paint);
}

static SkScalar getpathlen(const SkPath& path) {
    SkPathMeasure   meas(path, false);
    return meas.getLength();
}

static void test_textpathmatrix(SkCanvas* canvas) {
    SkPaint paint;
    SkPath  path;
    SkMatrix matrix;
    
    path.moveTo(SkIntToScalar(200), SkIntToScalar(300));
    path.quadTo(SkIntToScalar(400), SkIntToScalar(100),
                SkIntToScalar(600), SkIntToScalar(300));

    paint.setAntiAlias(true);
    
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setTextSize(SkIntToScalar(48));
    paint.setTextAlign(SkPaint::kRight_Align);
    
    const char* text = "Android";
    size_t      len = strlen(text);
    SkScalar    pathLen = getpathlen(path);

    canvas->drawTextOnPath(text, len, path, NULL, paint);
    
    paint.setColor(SK_ColorRED);
    matrix.setScale(-SK_Scalar1, SK_Scalar1);
    matrix.postTranslate(pathLen, 0);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);
    
    paint.setColor(SK_ColorBLUE);
    matrix.setScale(SK_Scalar1, -SK_Scalar1);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);
    
    paint.setColor(SK_ColorGREEN);
    matrix.setScale(-SK_Scalar1, -SK_Scalar1);
    matrix.postTranslate(pathLen, 0);
    canvas->drawTextOnPath(text, len, path, &matrix, paint);
}

class TextOnPathView : public SkView {
public:
    SkPath      fPath;
    SkScalar    fHOffset;

	TextOnPathView() {
        SkRect r;
        r.set(SkIntToScalar(100), SkIntToScalar(100),
              SkIntToScalar(300), SkIntToScalar(300));
        fPath.addOval(r);
        
        fHOffset = SkIntToScalar(50);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Text On Path");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
#if 0        
        SkRect r;
        SkPaint p;
        SkRandom rand;
        p.setAntiAlias(true);
        
        for (int i = 0; i < 100; i++) {
            SkScalar x = rand.nextUScalar1() * 300 + SkIntToScalar(50);
            SkScalar y = rand.nextUScalar1() * 200 + SkIntToScalar(50);
            SkScalar w = rand.nextUScalar1() * 10;
            SkScalar h = rand.nextUScalar1() * 10;
            r.set(x, y, x + w, y + h);
            canvas->drawRect(r, p);
        }
        
        test_textBounds(canvas);
//        return;

        SkBitmap    bm;
        if (SkImageDecoder::DecodeFile("/loading_tile.png",
                                       &bm, SkBitmap::kRGB_565_Config, true))
            canvas->drawBitmap(bm, 0, 0);
#endif
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkPaint paint;
        
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(50));

        for (int j = 0; j < REPEAT_COUNT; j++) {
            SkScalar x = fHOffset;

            paint.setColor(SK_ColorBLACK);
            canvas->drawTextOnPathHV(gText, sizeof(gText)-1, fPath,
                                     x, paint.getTextSize()/2, paint);

            paint.setColor(SK_ColorRED);
            canvas->drawTextOnPathHV(gText, sizeof(gText)-1, fPath,
                                     x + SkIntToScalar(50), 0, paint);

            paint.setColor(SK_ColorBLUE);
            canvas->drawTextOnPathHV(gText, sizeof(gText)-1, fPath,
                         x + SkIntToScalar(100), -paint.getTextSize()/2, paint);
        }
        
        paint.setColor(SK_ColorGREEN);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(fPath, paint);
        
        canvas->translate(SkIntToScalar(200), 0);
        test_textpathmatrix(canvas);

        test_bitmap_blur(canvas);
        
        if (REPEAT_COUNT > 1)
            this->inval(NULL);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fHints += 1;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) {
        return this->INHERITED::onClick(click);
    }
    
private:
    int fHints;
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() {
    return new TextOnPathView;
}

static SkViewRegister reg(MyFactory);

