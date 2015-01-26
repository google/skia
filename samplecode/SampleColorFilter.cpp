
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "sk_tool_utils.h"
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkDevice.h"
#include "SkPaint.h"
#include "SkShader.h"

static int inflate5To8(int x) {
    return (x << 3) | (x >> 2);
}

static int trunc5(int x) {
    return x >> 3;
}

#define SK_R16_BITS 5

#ifdef SK_DEBUG
static int round5_slow(int x) {
    int orig = x & 7;
    int fake = x >> 5;
    int trunc = x >> 3;

    int diff = fake - orig;

    int bias = 0;
    if (diff > 4) {
        bias = -1;
    } else if (diff < -4) {
        bias = 1;
    }
    return trunc + bias;
}
#endif

static int round5_fast(int x) {
    int result = x + 3 - (x >> 5) + (x >> 7);
    result >>= 3;
#ifdef SK_DEBUG
    {
        int r2 = round5_slow(x);
        SkASSERT(r2 == result);
    }
#endif
    return result;
}

static void test_5bits() {
    int e0 = 0;
    int e1 = 0;
    int e2 = 0;
    int ae0 = 0;
    int ae1 = 0;
    int ae2 = 0;
    for (int i = 0; i < 256; i++) {
        int t0 = trunc5(i);
        int t1 = round5_fast(i);
        int t2 = trunc5(i);
        int v0 = inflate5To8(t0);
        int v1 = inflate5To8(t1);
        int v2 = inflate5To8(t2);
        int err0 = i - v0;
        int err1 = i - v1;
        int err2 = i - v2;
        SkDebugf("--- %3d : trunc=%3d (%2d) round:%3d (%2d) \n"/*new:%d (%2d)\n"*/, i,
                 v0, err0, v1, err1, v2, err2);


        e0 += err0;
        e1 += err1;
        e2 += err2;
        ae0 += SkAbs32(err0);
        ae1 += SkAbs32(err1);
        ae2 += SkAbs32(err2);
    }
    SkDebugf("--- trunc: %d %d  round: %d %d new: %d %d\n", e0, ae0, e1, ae1, e2, ae2);
}

static SkBitmap createBitmap(int n) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(n, n);
    bitmap.eraseColor(SK_ColorTRANSPARENT);

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
        fShader = sk_tool_utils::create_checkerboard_shader(
                0xFFCCCCCC, 0xFFFFFFFF, 12);

        if (false) { // avoid bit rot, suppress warning
            test_5bits();
        }
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
            SkXfermode::kClear_Mode,
            SkXfermode::kSrc_Mode,
            SkXfermode::kDst_Mode,
            SkXfermode::kSrcOver_Mode,
            SkXfermode::kDstOver_Mode,
            SkXfermode::kSrcIn_Mode,
            SkXfermode::kDstIn_Mode,
            SkXfermode::kSrcOut_Mode,
            SkXfermode::kDstOut_Mode,
            SkXfermode::kSrcATop_Mode,
            SkXfermode::kDstATop_Mode,
            SkXfermode::kXor_Mode,
            SkXfermode::kPlus_Mode,
            SkXfermode::kModulate_Mode,
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
