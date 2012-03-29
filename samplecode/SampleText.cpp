
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
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
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(ReduceNoise)

private:
    ReduceNoise(SkFlattenableReadBuffer& rb) : SkKernel33ProcMaskFilter(rb) {}
};

class Darken : public SkKernel33ProcMaskFilter {
public:
    Darken(int percent256) : SkKernel33ProcMaskFilter(percent256) {}
    virtual uint8_t computeValue(uint8_t* const* srcRows)
    {
        int c = srcRows[1][1];
        float f = c / 255.f;

        if (c >= 0) {
            f = sqrtf(f);
        } else {
            f *= f;
        }
        SkASSERT(f >= 0 && f <= 1);
        return (int)(f * 255);
    }
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(Darken)

private:
    Darken(SkFlattenableReadBuffer& rb) : SkKernel33ProcMaskFilter(rb) {}
};

static SkMaskFilter* makemf() { return new Darken(0x30); }

static void test_breakText() {
    SkPaint paint;
    const char* text = "sdfkljAKLDFJKEWkldfjlk#$%&sdfs.dsj";
    size_t length = strlen(text);
    SkScalar width = paint.measureText(text, length);

    SkScalar mm = 0;
    SkScalar nn = 0;
    for (SkScalar w = 0; w <= width; w += SK_Scalar1) {
        SkScalar m;
        size_t n = paint.breakText(text, length, w, &m,
                                    SkPaint::kBackward_TextBufferDirection);

        SkASSERT(n <= length);
        SkASSERT(m <= width);

        if (n == 0) {
            SkASSERT(m == 0);
        } else {
            // now assert that we're monotonic
            if (n == nn) {
                SkASSERT(m == mm);
            } else {
                SkASSERT(n > nn);
                SkASSERT(m > mm);
            }
        }
        nn = SkIntToScalar(n);
        mm = m;
    }

    SkDEBUGCODE(size_t length2 =) paint.breakText(text, length, width, &mm);
    SkASSERT(length2 == length);
    SkASSERT(mm == width);
}

static SkRandom gRand;

class SkPowerMode : public SkXfermode {
public:
    SkPowerMode(SkScalar exponent) { this->init(exponent); }

    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]);

    typedef SkFlattenable* (*Factory)(SkFlattenableReadBuffer&);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPowerMode)

private:
    SkScalar fExp;          // user's value
    uint8_t fTable[256];    // cache

    void init(SkScalar exponent);
    SkPowerMode(SkFlattenableReadBuffer& b) : INHERITED(b) {
        // read the exponent
        this->init(SkFixedToScalar(b.readS32()));
    }
    virtual void flatten(SkFlattenableWriteBuffer& b) const SK_OVERRIDE {
        this->INHERITED::flatten(b);
        b.write32(SkScalarToFixed(fExp));
    }

    typedef SkXfermode INHERITED;
};

void SkPowerMode::init(SkScalar e) {
    fExp = e;
    float ee = SkScalarToFloat(e);

    printf("------ %g\n", ee);
    for (int i = 0; i < 256; i++) {
        float x = i / 255.f;
     //   printf(" %d %g", i, x);
        x = powf(x, ee);
     //   printf(" %g", x);
        int xx = SkScalarRound(SkFloatToScalar(x * 255));
     //   printf(" %d\n", xx);
        fTable[i] = SkToU8(xx);
    }
}

void SkPowerMode::xfer16(uint16_t dst[], const SkPMColor src[], int count,
                         const SkAlpha aa[]) {
    for (int i = 0; i < count; i++) {
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

static void DrawTheText(SkCanvas* canvas, const char text[], size_t length,
                        SkScalar x, SkScalar y, const SkPaint& paint,
                        SkScalar clickX, SkMaskFilter* mf) {
    SkPaint p(paint);

#if 0
    canvas->drawText(text, length, x, y, paint);
#else
    {
        SkPoint pts[1000];
        SkScalar xpos = x;
        SkASSERT(length <= SK_ARRAY_COUNT(pts));
        for (size_t i = 0; i < length; i++) {
            pts[i].set(xpos, y), xpos += paint.getTextSize();
        }
        canvas->drawPosText(text, length, pts, paint);
    }
#endif

    p.setSubpixelText(true);
    x += SkIntToScalar(180);
    canvas->drawText(text, length, x, y, p);

#ifdef SK_DEBUG
    if (true) {
    //    p.setMaskFilter(mf);
        p.setSubpixelText(false);
        p.setLinearText(true);
        x += SkIntToScalar(180);
        canvas->drawText(text, length, x, y, p);
    }
#endif
}

class TextSpeedView : public SampleView {
public:
	TextSpeedView() {
        fMF = makemf();

        fHints = 0;
        fClickX = 0;

        test_breakText();
    }

    virtual ~TextSpeedView() {
        SkSafeUnref(fMF);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Text");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    static void make_textstrip(SkBitmap* bm) {
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

    static void fill_pts(SkPoint pts[], size_t n, SkRandom* rand) {
        for (size_t i = 0; i < n; i++)
            pts[i].set(rand->nextUScalar1() * 640, rand->nextUScalar1() * 480);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
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

        const char* text = "Hamburgefons";
        size_t length = strlen(text);

        SkScalar y = SkIntToScalar(0);
        for (int i = 9; i <= 24; i++) {
            paint.setTextSize(SkIntToScalar(i) /*+ (gRand.nextU() & 0xFFFF)*/);
            for (SkScalar dx = 0; dx <= SkIntToScalar(3)/4;
                                            dx += SkIntToScalar(1) /* /4 */) {
                y += paint.getFontSpacing();
                DrawTheText(canvas, text, length, SkIntToScalar(20) + dx, y,
                            paint, fClickX, fMF);
            }
        }
        if (gHints[index].fFlushCache) {
//                SkGraphics::SetFontCacheUsed(0);
        }
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fClickX = x;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }

    virtual bool onClick(Click* click) {
        return this->INHERITED::onClick(click);
    }

private:
    int fHints;
    SkScalar fClickX;
    SkMaskFilter* fMF;

    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TextSpeedView; }
static SkViewRegister reg(MyFactory);

