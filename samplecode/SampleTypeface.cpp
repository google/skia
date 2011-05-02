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
#include "SkTypefaceCache.h"

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
    { "sans-serif", SkTypeface::kNormal },
    { "sans-serif", SkTypeface::kBold },
    { "sans-serif", SkTypeface::kItalic },
    { "sans-serif", SkTypeface::kBoldItalic },
    { "serif", SkTypeface::kNormal },
    { "serif", SkTypeface::kBold },
    { "serif", SkTypeface::kItalic },
    { "serif", SkTypeface::kBoldItalic },
    { "monospace", SkTypeface::kNormal },
    { "monospace", SkTypeface::kBold },
    { "monospace", SkTypeface::kItalic },
    { "monospace", SkTypeface::kBoldItalic },
};

static const int gFaceCount = SK_ARRAY_COUNT(gFaces);

class TypefaceView : public SampleView {
    SkTypeface* fFaces[gFaceCount];

public:
	TypefaceView() {
        test_4444_dither();
        for (int i = 0; i < gFaceCount; i++) {
            fFaces[i] = SkTypeface::CreateFromName(gFaces[i].fName,
                                                   gFaces[i].fStyle);
        }

        this->setBGColor(0xFFDDDDDD);
    }

    virtual ~TypefaceView() {
        for (int i = 0; i < gFaceCount; i++) {
            SkSafeUnref(fFaces[i]);
        }

        SkTypefaceCache::Dump();
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

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(30));

        const char* text = "Hamburgefons";
        const size_t textLen = strlen(text);

        SkScalar x = SkIntToScalar(10);
        SkScalar dy = paint.getFontMetrics(NULL);
        SkScalar y = dy;

        paint.setLinearText(true);
        for (int i = 0; i < gFaceCount; i++) {
            paint.setTypeface(fFaces[i]);
            canvas->drawText(text, textLen, x, y, paint);
            y += dy;
        }
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TypefaceView; }
static SkViewRegister reg(MyFactory);

