#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkTypeface.h"
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

static int dither_4444(int x) {
    return ((x << 1) - ((x >> 4 << 4) | (x >> 4))) >> 4;
}

/** Ensure that the max of the original and dithered value (for alpha) is always
    >= any other dithered value. We apply this "max" in colorpriv.h when we
    predither down to 4444, to be sure that we stay in legal premultiplied form
 */
static void test_4444_dither() {
    int buckets[16];
    sk_bzero(buckets, sizeof(buckets));

    for (int a = 0; a <= 0xFF; a++) {
        int da = dither_4444(a);
        int maxa = SkMax32(a >> 4, da);
    //    SkDebugf("--- %02X %X\n", a, da);
        buckets[da] += 1;
        for (int c = 0; c <= a; c++) {
            int dc = dither_4444(c);
            if (maxa < dc) {
                SkDebugf("------------ error a=%d da=%d c=%d dc=%d\n", a, da,
                         c, dc);
            }
        }
    }
    for (int i = 0; i < 16; i++) {
    //    SkDebugf("[%d] = %d\n", i, buckets[i]);
    }
}

static const struct {
    const char* fName;
    SkTypeface::Style   fStyle;
} gFaces[] = {
    { NULL, SkTypeface::kNormal },
    { NULL, SkTypeface::kBold },
    { "serif", SkTypeface::kNormal },
    { "serif", SkTypeface::kBold },
    { "serif", SkTypeface::kItalic },
    { "serif", SkTypeface::kBoldItalic },
    { "monospace", SkTypeface::kNormal }
};

static const int gFaceCount = SK_ARRAY_COUNT(gFaces);

class TypefaceView : public SkView {
    SkTypeface* fFaces[gFaceCount];

public:
	TypefaceView() {
        test_4444_dither();
        for (int i = 0; i < gFaceCount; i++) {
            fFaces[i] = SkTypeface::CreateFromName(gFaces[i].fName,
                                                   gFaces[i].fStyle);
        }
    }
    
    virtual ~TypefaceView() {
        for (int i = 0; i < gFaceCount; i++) {
            fFaces[i]->safeUnref();
        }
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Typefaces");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(30));

        if (false) {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(SkIntToScalar(1));
        }

        const char* text = "Hamburgefons";
        const size_t textLen = strlen(text);
        
        SkScalar x = SkIntToScalar(10);
        SkScalar dy = paint.getFontMetrics(NULL);
        SkScalar y = dy;
        
        for (int i = 0; i < gFaceCount; i++) {
            paint.setTypeface(fFaces[i]);
            canvas->drawText(text, textLen, x, y, paint);
            y += dy;
        }
        
        SkRect r;
        if (false) {
        r.set(10, 10, 100, 100);
        paint.setStyle(SkPaint::kStrokeAndFill_Style);
        paint.setColor(SK_ColorBLUE);
        paint.setStrokeWidth(1);
        canvas->drawRect(r, paint);
        paint.setStrokeWidth(0);
        }

        if (false) {
        r.set(294912.75f, 294912.75f, 884738.25f, 884738.25f);
        canvas->scale(2.4414E-4f, 2.4414E-4f);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawRect(r, paint);
        }
        
        if (false) {
            SkScalar rad = 90;
            SkScalar angle = 210;
            SkScalar cx = 150;
            SkScalar cy = 105;
            r.set(cx - rad, cy - rad, cx + rad, cy + rad);
            SkPath path;
            path.arcTo(r, angle, -(angle + 90), true);
            path.close();
            
            paint.setColor(SK_ColorRED);
            canvas->drawRect(path.getBounds(), paint);
            paint.setColor(SK_ColorBLUE);
            canvas->drawPath(path, paint);
            
            paint.setColor(SK_ColorGREEN);
            SkPoint pts[100];
            int count = path.getPoints(pts, 100);
            paint.setStrokeWidth(5);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, count, pts, paint);
        }
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TypefaceView; }
static SkViewRegister reg(MyFactory);

