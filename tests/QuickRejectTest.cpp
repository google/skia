/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkCanvas.h"
#include "SkDrawLooper.h"

/*
 *  Subclass of looper that just draws once, with an offset in X.
 */
class TestLooper : public SkDrawLooper {
public:
    bool fOnce;

    virtual void init(SkCanvas*) SK_OVERRIDE {
        fOnce = true;
    }

    virtual bool next(SkCanvas* canvas, SkPaint*) SK_OVERRIDE {
        if (fOnce) {
            fOnce = false;
            canvas->translate(SkIntToScalar(10), 0);
            return true;
        }
        return false;
    }

#ifdef SK_DEVELOPER
    virtual void toString(SkString* str) const SK_OVERRIDE {
        str->append("TestLooper:");
    }
#endif

    SK_DECLARE_UNFLATTENABLE_OBJECT()
};

static void test_drawBitmap(skiatest::Reporter* reporter) {
    SkBitmap src;
    src.setConfig(SkBitmap::kARGB_8888_Config, 10, 10);
    src.allocPixels();
    src.eraseColor(SK_ColorWHITE);

    SkBitmap dst;
    dst.setConfig(SkBitmap::kARGB_8888_Config, 10, 10);
    dst.allocPixels();
    dst.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(dst);
    SkPaint  paint;

    // we are initially transparent
    REPORTER_ASSERT(reporter, 0 == *dst.getAddr32(5, 5));

    // we see the bitmap drawn
    canvas.drawBitmap(src, 0, 0, &paint);
    REPORTER_ASSERT(reporter, 0xFFFFFFFF == *dst.getAddr32(5, 5));

    // reverify we are clear again
    dst.eraseColor(SK_ColorTRANSPARENT);
    REPORTER_ASSERT(reporter, 0 == *dst.getAddr32(5, 5));

    // if the bitmap is clipped out, we don't draw it
    canvas.drawBitmap(src, SkIntToScalar(-10), 0, &paint);
    REPORTER_ASSERT(reporter, 0 == *dst.getAddr32(5, 5));

    // now install our looper, which will draw, since it internally translates
    // to the left. The test is to ensure that canvas' quickReject machinary
    // allows us through, even though sans-looper we would look like we should
    // be clipped out.
    paint.setLooper(new TestLooper)->unref();
    canvas.drawBitmap(src, SkIntToScalar(-10), 0, &paint);
    REPORTER_ASSERT(reporter, 0xFFFFFFFF == *dst.getAddr32(5, 5));
}

DEF_TEST(QuickReject, reporter) {
    test_drawBitmap(reporter);
}
