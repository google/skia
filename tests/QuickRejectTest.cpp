/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkArenaAlloc.h"
#include "tests/Test.h"

static void test_drawBitmap(skiatest::Reporter* reporter) {
    SkBitmap src;
    src.allocN32Pixels(10, 10);
    src.eraseColor(SK_ColorWHITE);

    SkBitmap dst;
    dst.allocN32Pixels(10, 10);
    dst.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(dst);

    // we are initially transparent
    REPORTER_ASSERT(reporter, 0 == *dst.getAddr32(5, 5));

    // we see the bitmap drawn
    canvas.drawImage(src.asImage(), 0, 0);
    REPORTER_ASSERT(reporter, 0xFFFFFFFF == *dst.getAddr32(5, 5));

    // reverify we are clear again
    dst.eraseColor(SK_ColorTRANSPARENT);
    REPORTER_ASSERT(reporter, 0 == *dst.getAddr32(5, 5));

    // if the bitmap is clipped out, we don't draw it
    canvas.drawImage(src.asImage(), SkIntToScalar(-10), 0);
    REPORTER_ASSERT(reporter, 0 == *dst.getAddr32(5, 5));
}

static void test_layers(skiatest::Reporter* reporter) {
    SkCanvas canvas(100, 100);

    SkRect r = SkRect::MakeWH(10, 10);
    REPORTER_ASSERT(reporter, false == canvas.quickReject(r));

    r.offset(300, 300);
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r));

    // Test that saveLayer updates quickReject
    SkRect bounds = SkRect::MakeLTRB(50, 50, 70, 70);
    canvas.saveLayer(&bounds, nullptr);
    REPORTER_ASSERT(reporter, true == canvas.quickReject(SkRect::MakeWH(10, 10)));
    REPORTER_ASSERT(reporter, false == canvas.quickReject(SkRect::MakeWH(60, 60)));
}

static void test_quick_reject(skiatest::Reporter* reporter) {
    SkCanvas canvas(100, 100);
    SkRect r0 = SkRect::MakeLTRB(-50.0f, -50.0f, 50.0f, 50.0f);
    SkRect r1 = SkRect::MakeLTRB(-50.0f, 110.0f, 50.0f, 120.0f);
    SkRect r2 = SkRect::MakeLTRB(110.0f, -50.0f, 120.0f, 50.0f);
    SkRect r3 = SkRect::MakeLTRB(-120.0f, -50.0f, 120.0f, 50.0f);
    SkRect r4 = SkRect::MakeLTRB(-50.0f, -120.0f, 50.0f, 120.0f);
    SkRect r5 = SkRect::MakeLTRB(-120.0f, -120.0f, 120.0f, 120.0f);
    SkRect r6 = SkRect::MakeLTRB(-120.0f, -120.0f, -110.0f, -110.0f);
    SkRect r7 = SkRect::MakeLTRB(SK_ScalarNaN, -50.0f, 50.0f, 50.0f);
    SkRect r8 = SkRect::MakeLTRB(-50.0f, SK_ScalarNaN, 50.0f, 50.0f);
    SkRect r9 = SkRect::MakeLTRB(-50.0f, -50.0f, SK_ScalarNaN, 50.0f);
    SkRect r10 = SkRect::MakeLTRB(-50.0f, -50.0f, 50.0f, SK_ScalarNaN);
    REPORTER_ASSERT(reporter, false == canvas.quickReject(r0));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r1));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r2));
    REPORTER_ASSERT(reporter, false == canvas.quickReject(r3));
    REPORTER_ASSERT(reporter, false == canvas.quickReject(r4));
    REPORTER_ASSERT(reporter, false == canvas.quickReject(r5));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r6));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r7));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r8));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r9));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r10));

    SkMatrix m = SkMatrix::Scale(2, 2);
    m.setTranslateX(10.0f);
    m.setTranslateY(10.0f);
    canvas.setMatrix(m);
    SkRect r11 = SkRect::MakeLTRB(5.0f, 5.0f, 100.0f, 100.0f);
    SkRect r12 = SkRect::MakeLTRB(5.0f, 50.0f, 100.0f, 100.0f);
    SkRect r13 = SkRect::MakeLTRB(50.0f, 5.0f, 100.0f, 100.0f);
    REPORTER_ASSERT(reporter, false == canvas.quickReject(r11));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r12));
    REPORTER_ASSERT(reporter, true == canvas.quickReject(r13));
}

DEF_TEST(QuickReject, reporter) {
    test_drawBitmap(reporter);
    test_layers(reporter);
    test_quick_reject(reporter);
}

// Regression test to make sure that we keep fIsScaleTranslate up to date on the canvas.
// It is possible to set a new matrix on the canvas without calling setMatrix().  This tests
// that code path.
DEF_TEST(QuickReject_MatrixState, reporter) {
    SkCanvas canvas(100, 100);

    SkMatrix matrix;
    matrix.setRotate(45.0f);
    canvas.setMatrix(matrix);

    SkPaint paint;
    sk_sp<SkImageFilter> filter = SkImageFilters::DistantLitDiffuse(
            SkPoint3::Make(1.0f, 1.0f, 1.0f), 0xFF0000FF, 2.0f, 0.5f, nullptr);
    REPORTER_ASSERT(reporter, filter);
    paint.setImageFilter(filter);
    SkCanvas::SaveLayerRec rec;
    rec.fPaint = &paint;
    canvas.saveLayer(rec);

    // quickReject() will assert if the matrix is out of sync.
    canvas.quickReject(SkRect::MakeWH(100.0f, 100.0f));
}
