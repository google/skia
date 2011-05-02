#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkKernel33MaskFilter.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

#include "SkStream.h"
#include "SkXMLParser.h"

static const int gKernel[3][3] = {
//    { -1, -2, -1 }, { -2, 12, -2 }, { -1, -2, -1 }
    { 1, 2, 1 }, { 2, 64-12, 2 }, { 1, 2, 1 }
};
static const int gShift = 6;

class ReduceNoise : public SkKernel33ProcMaskFilter {
public:
    ReduceNoise(int percent256) : SkKernel33ProcMaskFilter(percent256) {}
    virtual uint8_t computeValue(uint8_t* const* srcRows)
    {
        int c = srcRows[1][1];
        int min = 255, max = 0;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                if (i != 1 || j != 1)
                {
                    int v = srcRows[i][j];
                    if (max < v)
                        max = v;
                    if  (min > v)
                        min = v;
                }
        if (c > max) c = max;
    //    if (c < min) c = min;
        return c;
    }
    virtual Factory getFactory() { return Create; }
private:
    ReduceNoise(SkFlattenableReadBuffer& rb) : SkKernel33ProcMaskFilter(rb) {}
    static SkFlattenable* Create(SkFlattenableReadBuffer& rb)
    {
        return new ReduceNoise(rb);
    }
};

class Darken : public SkKernel33ProcMaskFilter {
public:
    Darken(int percent256) : SkKernel33ProcMaskFilter(percent256) {}
    virtual uint8_t computeValue(uint8_t* const* srcRows)
    {
        int c = srcRows[1][1];
        float f = c / 255.f;

        if (c >= 0)
        {
            f = sqrtf(f);
        }
        else
        {
            f *= f;
        }
        SkASSERT(f >= 0 && f <= 1);
        return (int)(f * 255);
    }
    virtual Factory getFactory() { return Create; }
private:
    Darken(SkFlattenableReadBuffer& rb) : SkKernel33ProcMaskFilter(rb) {}
    static SkFlattenable* Create(SkFlattenableReadBuffer& rb)
    {
        return new Darken(rb);
    }
};

static SkMaskFilter* makemf() { return new Darken(0x30); }

//#ifdef TEST_CLICKX

static void test_typefaceCache()
{
#ifdef ANDROID
    SkTypeface* t0 = SkTypeface::CreateFromName("sans-serif",
                                                SkTypeface::kNormal);
    SkTypeface* t1 = SkTypeface::CreateFromName(NULL, SkTypeface::kNormal);
    SkTypeface* t2 = SkTypeface::CreateFromName("arial", SkTypeface::kNormal);
    SkTypeface* t3 = SkTypeface::CreateFromName("helvetica", SkTypeface::kItalic);

    SkASSERT(t0 == t1);
    SkASSERT(t0 == t2);
    SkASSERT(t0 == t3);
#endif
}

static void test_breakText()
{
    SkPaint paint;
    const char* text = "sdfkljAKLDFJKEWkldfjlk#$%&sdfs.dsj";
    size_t length = strlen(text);
    SkScalar width = paint.measureText(text, length);

    SkScalar mm = 0;
    SkScalar nn = 0;
    for (SkScalar w = 0; w <= width; w += SK_Scalar1)
    {
        SkScalar m;
        size_t n = paint.breakText(text, length, w, &m,
                                    SkPaint::kBackward_TextBufferDirection);

        SkASSERT(n <= length);
        SkASSERT(m <= width);

        if (n == 0)
            SkASSERT(m == 0);
        else
        {
            // now assert that we're monotonic
            if (n == nn)
                SkASSERT(m == mm);
            else
            {
                SkASSERT(n > nn);
                SkASSERT(m > mm);
            }
        }
        nn = SkIntToScalar(n);
        mm = m;
    }

    SkDEBUGCODE(int length2 =) paint.breakText(text, length, width, &mm);
    SkASSERT(length2 == length);
    SkASSERT(mm == width);
}

static SkRandom gRand;

class SkPowerMode : public SkXfermode {
public:
    SkPowerMode(SkScalar exponent) { this->init(exponent); }

    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count, const SkAlpha aa[]);

    typedef SkFlattenable* (*Factory)(SkFlattenableReadBuffer&);

    // overrides for SkFlattenable
    virtual Factory getFactory() { return Create; }
    virtual void flatten(SkFlattenableWriteBuffer& b)
    {
    //    this->INHERITED::flatten(b);  How can we know if this is legal????
        b.write32(SkScalarToFixed(fExp));
    }

private:
    SkScalar fExp;          // user's value
    uint8_t fTable[256];    // cache

    void init(SkScalar exponent);
    SkPowerMode(SkFlattenableReadBuffer& b) : SkXfermode(b)
    {
        // read the exponent
        this->init(SkFixedToScalar(b.readS32()));
    }
    static SkFlattenable* Create(SkFlattenableReadBuffer& b)
    {
        return SkNEW_ARGS(SkPowerMode, (b));
    }

    typedef SkXfermode INHERITED;
};

void SkPowerMode::init(SkScalar e)
{
    fExp = e;
    float ee = SkScalarToFloat(e);

    printf("------ %g\n", ee);
    for (int i = 0; i < 256; i++)
    {
        float x = i / 255.f;
     //   printf(" %d %g", i, x);
        x = powf(x, ee);
     //   printf(" %g", x);
        int xx = SkScalarRound(SkFloatToScalar(x * 255));
     //   printf(" %d\n", xx);
        fTable[i] = SkToU8(xx);
    }
}

void SkPowerMode::xfer16(uint16_t dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    for (int i = 0; i < count; i++)
    {
        SkPMColor c = src[i];
        int r = SkGetPackedR32(c);
        int g = SkGetPackedG32(c);
        int b = SkGetPackedB32(c);
        r = fTable[r];
        g = fTable[g];
        b = fTable[b];
        dst[i] = SkPack888ToRGB16(r, g, b);
    }
}

static const struct {
    const char* fName;
    uint32_t    fFlags;
    bool        fFlushCache;
} gHints[] = {
    { "Linear", SkPaint::kLinearText_Flag,     false },
    { "Normal",   0,                           true },
    { "Subpixel", SkPaint::kSubpixelText_Flag, true }
};

static int count_char_points(const SkPaint& paint, char c)
{
    SkPath  path;

    paint.getTextPath(&c, 1, 0, 0, &path);
    return path.getPoints(NULL, 0);
}

static int gOld, gNew, gCount;

static void dump(int c, int oldc, int newc)
{
    if (oldc != newc)
    {
        gOld += oldc;
        gNew += newc;
        gCount += 1;
        printf("char %c: old = %3d, new = %3d, reduction %g%%\n", c, oldc, newc, 100. * (oldc - newc) / oldc);
    }
}

static void tab(int n)
{
//    printf("[%d] ", n); return;
    SkASSERT(n >= 0);
    for (int i = 0; i < n; i++)
        printf("    ");
}

#if 0
#include "badrects.cpp"

static void make_badrgn(SkRegion* rgn, int insetAmount)
{
    SkRect16    r, bounds;
    int         i;

    rgn->setEmpty();
    bounds.setEmpty();

    for (i = 0; i < SK_ARRAY_COUNT(badrects); i++)
    {
        SkASSERT(badrects[i].width > 0 && badrects[i].height > 0);

        r.set(badrects[i].x, badrects[i].y, badrects[i].x + badrects[i].width, badrects[i].y + badrects[i].height);
        r.inset(insetAmount, insetAmount);
        rgn->op(r, SkRegion::kUnion_Op);
        bounds.join(r);
    }
    SkASSERT(bounds == rgn->getBounds());

    for (i = 0; i < SK_ARRAY_COUNT(badrects); i++)
    {
        r.set(badrects[i].x, badrects[i].y, badrects[i].x + badrects[i].width, badrects[i].y + badrects[i].height);
        SkASSERT(rgn->contains(r));
    }
}
#endif

static void draw_rgn(const SkRegion& rgn, SkCanvas* canvas, const SkPaint& paint)
{
    SkRect    r;
    SkRegion::Iterator  iter(rgn);

    for (; !iter.done(); iter.next())
    {
        r.set(iter.rect());
        canvas->drawRect(r, paint);
    }
}

static void test_break(SkCanvas* canvas, const char text[], size_t length,
                        SkScalar x, SkScalar y, const SkPaint& paint,
                        SkScalar clickX)
{
    SkPaint linePaint;

    linePaint.setAntiAlias(true);

    SkScalar measured;

    if (paint.breakText(text, length, clickX - x, &measured, SkPaint::kForward_TextBufferDirection))
    {
        linePaint.setColor(SK_ColorRED);
        canvas->drawLine(x, y, x + measured, y, linePaint);
    }

    x += paint.measureText(text, length);
    if (paint.breakText(text, length, x - clickX, &measured, SkPaint::kBackward_TextBufferDirection))
    {
        linePaint.setColor(SK_ColorBLUE);
        canvas->drawLine(x - measured, y, x, y, linePaint);
    }
}

static void test_poly()
{
    static const SkPoint dst[] = {
        SkIntToScalar(2), SkIntToScalar(1),
        SkIntToScalar(5), SkIntToScalar(1),
        SkIntToScalar(5), SkIntToScalar(3),
        SkIntToScalar(2), SkIntToScalar(3)
    };

    static const SkPoint src[] = {
        SkIntToScalar(0), SkIntToScalar(0),
        SkIntToScalar(1), SkIntToScalar(0),
        SkIntToScalar(1), SkIntToScalar(1),
        SkIntToScalar(0), SkIntToScalar(1)
    };

    SkMatrix matrix;

    if (matrix.setPolyToPoly(src, dst, 4))
    {
        SkPoint pt = { SK_Scalar1/2, SK_Scalar1/2 };
        matrix.mapPoints(&pt, 1);
        printf("---- x = %g y = %g\n", SkScalarToFloat(pt.fX), SkScalarToFloat(pt.fY));
    }
    else
        printf("---- setPolyToPoly failed\n");
}

#include "SkColorShader.h"

static void DrawTheText(SkCanvas* canvas, const char text[], size_t length,
                        SkScalar x, SkScalar y, const SkPaint& paint,
                        SkScalar clickX, SkMaskFilter* mf)
{
    SkPaint p(paint);

#if 0
    canvas->drawText(text, length, x, y, paint);
#else
    {
        SkPoint pts[1000];
        SkScalar xpos = x;
        SkASSERT(length <= SK_ARRAY_COUNT(pts));
        for (size_t i = 0; i < length; i++)
            pts[i].set(xpos, y), xpos += paint.getTextSize();
        canvas->drawPosText(text, length, pts, paint);
    }
#endif

    p.setSubpixelText(true);
    x += SkIntToScalar(180);
    canvas->drawText(text, length, x, y, p);

#ifdef TEST_CLICKX
    test_break(canvas, text, length, x, y, p, clickX);
#endif

#ifdef SK_DEBUG
    if (false)
    {
        SkColorShader   shader;
        p.setShader(&shader);
        x += SkIntToScalar(180);
        canvas->drawText(text, length, x, y, p);
        p.setShader(NULL);
    }

    if (true)
    {
    //    p.setMaskFilter(mf);
        p.setSubpixelText(false);
        p.setLinearText(true);
        x += SkIntToScalar(180);
        canvas->drawText(text, length, x, y, p);
    }
#endif
}

class TextSpeedView : public SkView {
public:
	TextSpeedView()
    {
        fMF = makemf();

        fHints = 0;

        if (false)
        {
            static const char extra[] = { '.', ',', ':', ';', '!' };
            SkPaint   paint, paint2;

            paint2.setTypeface(SkTypeface::CreateFromName(NULL,
                                                SkTypeface::kItalic))->unref();

            for (int i = 0; i < 26; i++)
                ::dump('a' + i, count_char_points(paint, 'a' + i), count_char_points(paint2, 'a' + i));
            for (int j = 0; j < SK_ARRAY_COUNT(extra); j++)
                ::dump(extra[j], count_char_points(paint, extra[j]), count_char_points(paint2, extra[j]));

            printf("--- ave reduction = %g%%\n", 100. * (gOld - gNew) / gOld);
        }

        if (true)
        {
            SkPoint pts[] = { SkIntToScalar(20), 0, SkIntToScalar(256+20), 0 };
            SkColor colors[] = { SkColorSetARGB(0, 255, 255, 255), SkColorSetARGB(255, 255, 255, 255) };
            fGradient = SkGradientShader::CreateLinear(pts, colors, NULL, 2, SkShader::kClamp_TileMode);
        }

        fClickX = 0;

        test_breakText();
        test_typefaceCache();
//        test_poly();
    }

    virtual ~TextSpeedView()
    {
        fGradient->unref();
        SkSafeUnref(fMF);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "Text");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas)
    {
//        canvas->drawColor(0xFFDDDDDD);
        canvas->drawColor(SK_ColorWHITE);
   //     canvas->drawColor(SK_ColorBLACK);
    }

    static void make_textstrip(SkBitmap* bm)
    {
        bm->setConfig(SkBitmap::kRGB_565_Config, 200, 18);
        bm->allocPixels();
        bm->eraseColor(SK_ColorWHITE);

        SkCanvas    canvas(*bm);
        SkPaint     paint;
        const char* s = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit";

        paint.setFlags(paint.getFlags() | SkPaint::kAntiAlias_Flag
                                        | SkPaint::kDevKernText_Flag);
        paint.setTextSize(SkIntToScalar(14));
        canvas.drawText(s, strlen(s), SkIntToScalar(8), SkIntToScalar(14), paint);
    }

    static void fill_pts(SkPoint pts[], size_t n, SkRandom* rand)
    {
        for (size_t i = 0; i < n; i++)
            pts[i].set(rand->nextUScalar1() * 640, rand->nextUScalar1() * 480);
    }

    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);

        SkAutoCanvasRestore restore(canvas, false);
        {
            SkRect r;
            r.set(0, 0, SkIntToScalar(1000), SkIntToScalar(20));
       //     canvas->saveLayer(&r, NULL, SkCanvas::kHasAlphaLayer_SaveFlag);
        }

        SkPaint paint;
//        const uint16_t glyphs[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19 };
        int         index = fHints % SK_ARRAY_COUNT(gHints);
        index = 1;
//        const char* style = gHints[index].fName;

//        canvas->translate(0, SkIntToScalar(50));

  //      canvas->drawText(style, strlen(style), SkIntToScalar(20), SkIntToScalar(20), paint);

        SkSafeUnref(paint.setTypeface(SkTypeface::CreateFromFile("/skimages/samplefont.ttf")));
        paint.setAntiAlias(true);
        paint.setFlags(paint.getFlags() | gHints[index].fFlags);

        SkRect clip;
        clip.set(SkIntToScalar(25), SkIntToScalar(34), SkIntToScalar(88), SkIntToScalar(155));

        if (0) {
            canvas->clipRect(clip);
        }

        if (0) {
            SkPath clipPath;
            clipPath.addOval(clip);
            canvas->clipPath(clipPath);
        }

        const char* text = "Hamburgefons";
        size_t length = strlen(text);

#ifdef TEST_CLICKX
        {
            SkPaint p;

            p.setColor(SK_ColorGREEN);
            p.setAntiAlias(true);
            canvas->drawLine(fClickX, 0, fClickX, SkIntToScalar(1000), p);
        }
#endif

        SkScalar y = SkIntToScalar(0);
        for (int i = 9; i <= 24; i++) {
            paint.setTextSize(SkIntToScalar(i) /*+ (gRand.nextU() & 0xFFFF)*/);
            for (SkScalar dx = 0; dx <= SkIntToScalar(3)/4; dx += SkIntToScalar(1) /* /4 */)
            {
                y += paint.getFontSpacing();
                DrawTheText(canvas, text, length, SkIntToScalar(20) + dx, y, paint, fClickX, fMF);
            }
        }
        if (gHints[index].fFlushCache) {
//                SkGraphics::SetFontCacheUsed(0);
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y)
    {
        fClickX = x;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }

    virtual bool onClick(Click* click)
    {
        return this->INHERITED::onClick(click);
    }

private:
    int fHints;
    SkScalar fClickX;
    SkMaskFilter* fMF;
    SkShader* fGradient;

    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TextSpeedView; }
static SkViewRegister reg(MyFactory);

