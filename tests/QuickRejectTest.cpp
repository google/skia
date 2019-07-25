/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkDrawLooper.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkLightingImageFilter.h"
#include "src/core/SkArenaAlloc.h"
#include "tests/Test.h"

/*
 *  Subclass of looper that just draws once, with an offset in X.
 */
class TestLooper : public SkDrawLooper {
public:

    SkDrawLooper::Context* makeContext(SkArenaAlloc* alloc) const override {
        return alloc->make<TestDrawLooperContext>();
    }

private:
    SK_FLATTENABLE_HOOKS(TestLooper)

    class TestDrawLooperContext : public SkDrawLooper::Context {
    public:
        TestDrawLooperContext() : fOnce(true) {}
        ~TestDrawLooperContext() override {}

        bool next(Info* info, SkPaint*) override {
            if (fOnce) {
                fOnce = false;
                info->fTranslate = {10, 0};
                info->fApplyPostCTM = false;
                return true;
            }
            return false;
        }

    private:
        bool fOnce;
    };
};

sk_sp<SkFlattenable> TestLooper::CreateProc(SkReadBuffer&) { return sk_make_sp<TestLooper>(); }

static void test_drawBitmap(skiatest::Reporter* reporter) {
    SkBitmap src;
    src.allocN32Pixels(10, 10);
    src.eraseColor(SK_ColorWHITE);

    SkBitmap dst;
    dst.allocN32Pixels(10, 10);
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

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
    // now install our looper, which will draw, since it internally translates
    // to the left. The test is to ensure that canvas' quickReject machinary
    // allows us through, even though sans-looper we would look like we should
    // be clipped out.
    paint.setLooper(sk_make_sp<TestLooper>());
    canvas.drawBitmap(src, SkIntToScalar(-10), 0, &paint);
    REPORTER_ASSERT(reporter, 0xFFFFFFFF == *dst.getAddr32(5, 5));
#endif
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

    SkMatrix m = SkMatrix::MakeScale(2.0f);
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
    sk_sp<SkImageFilter> filter = SkLightingImageFilter::MakeDistantLitDiffuse(
            SkPoint3::Make(1.0f, 1.0f, 1.0f), 0xFF0000FF, 2.0f, 0.5f, nullptr);
    REPORTER_ASSERT(reporter, filter);
    paint.setImageFilter(filter);
    SkCanvas::SaveLayerRec rec;
    rec.fPaint = &paint;
    canvas.saveLayer(rec);

    // quickReject() will assert if the matrix is out of sync.
    canvas.quickReject(SkRect::MakeWH(100.0f, 100.0f));
}

#ifdef SK_SUPPORT_LEGACY_DRAWLOOPER
#include "include/core/SkSurface.h"
#include "include/effects/SkLayerDrawLooper.h"
DEF_TEST(looper_nothingtodraw, reporter) {
    auto surf = SkSurface::MakeRasterN32Premul(20, 20);

    SkPaint paint;
    paint.setColor(0);
    REPORTER_ASSERT(reporter, paint.nothingToDraw());

    SkLayerDrawLooper::Builder builder;
    builder.addLayer();
    paint.setDrawLooper(builder.detach());
    // the presence of the looper fools this predicate, so we think it might draw
    REPORTER_ASSERT(reporter, !paint.nothingToDraw());

    // Before fixing, this would assert in ~AutoDrawLooper() in SkCanvas.cpp as it checked for
    // a balance in the save/restore count after handling the looper. Before the fix, this
    // code would call nothingToDraw() and since it now clears the looper, that predicate will
    // return true, aborting the sequence prematurely, and not finishing the iterator on the looper
    // which handles the final "restore". This was a bug -- we *must* call the looper's iterator
    // until it returns done to keep the canvas balanced. The fix was to remove this early-exit
    // in the autodrawlooper. Now this call won't assert.
    // See https://skia-review.googlesource.com/c/skia/+/121220
    surf->getCanvas()->drawRect({1, 1, 10, 10}, paint);
}
#endif
