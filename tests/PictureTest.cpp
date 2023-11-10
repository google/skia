/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBBHFactory.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImage.h" // IWYU pragma: keep
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/base/SkRandom.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRectPriv.h"
#include "tests/Test.h"
#include "tools/fonts/FontToolUtils.h"

#include <cstddef>
#include <memory>
#include <vector>

class SkRRect;
class SkRegion;

static void make_bm(SkBitmap* bm, int w, int h, SkColor color, bool immutable) {
    bm->allocN32Pixels(w, h);
    bm->eraseColor(color);
    if (immutable) {
        bm->setImmutable();
    }
}

#ifdef SK_DEBUG
// Ensure that deleting an empty SkPicture does not assert. Asserts only fire
// in debug mode, so only run in debug mode.
static void test_deleting_empty_picture() {
    SkPictureRecorder recorder;
    // Creates an SkPictureRecord
    recorder.beginRecording(0, 0);
    // Turns that into an SkPicture
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    // Ceates a new SkPictureRecord
    recorder.beginRecording(0, 0);
}

// Ensure that serializing an empty picture does not assert. Likewise only runs in debug mode.
static void test_serializing_empty_picture() {
    SkPictureRecorder recorder;
    recorder.beginRecording(0, 0);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    SkDynamicMemoryWStream stream;
    picture->serialize(&stream, nullptr);  // default SkSerialProcs
}
#endif

static void rand_op(SkCanvas* canvas, SkRandom& rand) {
    SkPaint paint;
    SkRect rect = SkRect::MakeWH(50, 50);

    SkScalar unit = rand.nextUScalar1();
    if (unit <= 0.3) {
//        SkDebugf("save\n");
        canvas->save();
    } else if (unit <= 0.6) {
//        SkDebugf("restore\n");
        canvas->restore();
    } else if (unit <= 0.9) {
//        SkDebugf("clip\n");
        canvas->clipRect(rect);
    } else {
//        SkDebugf("draw\n");
        canvas->drawPaint(paint);
    }
}

static void set_canvas_to_save_count_4(SkCanvas* canvas) {
    canvas->restoreToCount(1);
    canvas->save();
    canvas->save();
    canvas->save();
}

/**
 * A canvas that records the number of saves, saveLayers and restores.
 */
class SaveCountingCanvas : public SkCanvas {
public:
    SaveCountingCanvas(int width, int height)
        : INHERITED(width, height)
        , fSaveCount(0)
        , fSaveLayerCount(0)
        , fSaveBehindCount(0)
        , fRestoreCount(0){
    }

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
        ++fSaveLayerCount;
        return this->INHERITED::getSaveLayerStrategy(rec);
    }

    bool onDoSaveBehind(const SkRect* subset) override {
        ++fSaveBehindCount;
        return this->INHERITED::onDoSaveBehind(subset);
    }

    void willSave() override {
        ++fSaveCount;
        this->INHERITED::willSave();
    }

    void willRestore() override {
        ++fRestoreCount;
        this->INHERITED::willRestore();
    }

    unsigned int getSaveCount() const { return fSaveCount; }
    unsigned int getSaveLayerCount() const { return fSaveLayerCount; }
    unsigned int getSaveBehindCount() const { return fSaveBehindCount; }
    unsigned int getRestoreCount() const { return fRestoreCount; }

private:
    unsigned int fSaveCount;
    unsigned int fSaveLayerCount;
    unsigned int fSaveBehindCount;
    unsigned int fRestoreCount;

    using INHERITED = SkCanvas;
};

void check_save_state(skiatest::Reporter* reporter, SkPicture* picture,
                      unsigned int numSaves, unsigned int numSaveLayers,
                      unsigned int numRestores) {
    SaveCountingCanvas canvas(SkScalarCeilToInt(picture->cullRect().width()),
                              SkScalarCeilToInt(picture->cullRect().height()));

    picture->playback(&canvas);

    // Optimizations may have removed these,
    // so expect to have seen no more than num{Saves,SaveLayers,Restores}.
    REPORTER_ASSERT(reporter, numSaves >= canvas.getSaveCount());
    REPORTER_ASSERT(reporter, numSaveLayers >= canvas.getSaveLayerCount());
    REPORTER_ASSERT(reporter, numRestores >= canvas.getRestoreCount());
}

// This class exists so SkPicture can friend it and give it access to
// the 'partialReplay' method.
class SkPictureRecorderReplayTester {
public:
    static sk_sp<SkPicture> Copy(SkPictureRecorder* recorder) {
        SkPictureRecorder recorder2;

        SkCanvas* canvas = recorder2.beginRecording(10, 10);

        recorder->partialReplay(canvas);

        return recorder2.finishRecordingAsPicture();
    }
};

static void create_imbalance(SkCanvas* canvas) {
    SkRect clipRect = SkRect::MakeWH(2, 2);
    SkRect drawRect = SkRect::MakeWH(10, 10);
    canvas->save();
        canvas->clipRect(clipRect, SkClipOp::kIntersect);
        canvas->translate(1.0f, 1.0f);
        SkPaint p;
        p.setColor(SK_ColorGREEN);
        canvas->drawRect(drawRect, p);
    // no restore
}

// This tests that replaying a potentially unbalanced picture into a canvas
// doesn't affect the canvas' save count or matrix/clip state.
static void check_balance(skiatest::Reporter* reporter, SkPicture* picture) {
    SkBitmap bm;
    bm.allocN32Pixels(4, 3);
    SkCanvas canvas(bm);

    int beforeSaveCount = canvas.getSaveCount();

    SkMatrix beforeMatrix = canvas.getTotalMatrix();

    SkRect beforeClip = canvas.getLocalClipBounds();

    canvas.drawPicture(picture);

    REPORTER_ASSERT(reporter, beforeSaveCount == canvas.getSaveCount());
    REPORTER_ASSERT(reporter, beforeMatrix == canvas.getTotalMatrix());

    SkRect afterClip = canvas.getLocalClipBounds();

    REPORTER_ASSERT(reporter, afterClip == beforeClip);
}

// Test out SkPictureRecorder::partialReplay
DEF_TEST(PictureRecorder_replay, reporter) {
    // check save/saveLayer state
    {
        SkPictureRecorder recorder;

        SkCanvas* canvas = recorder.beginRecording(10, 10);

        canvas->saveLayer(nullptr, nullptr);

        sk_sp<SkPicture> copy(SkPictureRecorderReplayTester::Copy(&recorder));

        // The extra save and restore comes from the Copy process.
        check_save_state(reporter, copy.get(), 2, 1, 3);

        canvas->saveLayer(nullptr, nullptr);

        sk_sp<SkPicture> final(recorder.finishRecordingAsPicture());

        check_save_state(reporter, final.get(), 1, 2, 3);

        // The copy shouldn't pick up any operations added after it was made
        check_save_state(reporter, copy.get(), 2, 1, 3);
    }

    // Recreate the Android partialReplay test case
    {
        SkPictureRecorder recorder;

        SkCanvas* canvas = recorder.beginRecording(4, 3);
        create_imbalance(canvas);

        int expectedSaveCount = canvas->getSaveCount();

        sk_sp<SkPicture> copy(SkPictureRecorderReplayTester::Copy(&recorder));
        check_balance(reporter, copy.get());

        REPORTER_ASSERT(reporter, expectedSaveCount = canvas->getSaveCount());

        // End the recording of source to test the picture finalization
        // process isn't complicated by the partialReplay step
        sk_sp<SkPicture> final(recorder.finishRecordingAsPicture());
    }
}

static void test_unbalanced_save_restores(skiatest::Reporter* reporter) {
    SkCanvas testCanvas(100, 100);
    set_canvas_to_save_count_4(&testCanvas);

    REPORTER_ASSERT(reporter, 4 == testCanvas.getSaveCount());

    SkPaint paint;
    SkRect rect = SkRect::MakeLTRB(-10000000, -10000000, 10000000, 10000000);

    SkPictureRecorder recorder;

    {
        // Create picture with 2 unbalanced saves
        SkCanvas* canvas = recorder.beginRecording(100, 100);
        canvas->save();
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        canvas->save();
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        sk_sp<SkPicture> extraSavePicture(recorder.finishRecordingAsPicture());

        testCanvas.drawPicture(extraSavePicture);
        REPORTER_ASSERT(reporter, 4 == testCanvas.getSaveCount());
    }

    set_canvas_to_save_count_4(&testCanvas);

    {
        // Create picture with 2 unbalanced restores
        SkCanvas* canvas = recorder.beginRecording(100, 100);
        canvas->save();
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        canvas->save();
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        canvas->restore();
        canvas->restore();
        canvas->restore();
        canvas->restore();
        sk_sp<SkPicture> extraRestorePicture(recorder.finishRecordingAsPicture());

        testCanvas.drawPicture(extraRestorePicture);
        REPORTER_ASSERT(reporter, 4 == testCanvas.getSaveCount());
    }

    set_canvas_to_save_count_4(&testCanvas);

    {
        SkCanvas* canvas = recorder.beginRecording(100, 100);
        canvas->translate(10, 10);
        canvas->drawRect(rect, paint);
        sk_sp<SkPicture> noSavePicture(recorder.finishRecordingAsPicture());

        testCanvas.drawPicture(noSavePicture);
        REPORTER_ASSERT(reporter, 4 == testCanvas.getSaveCount());
        REPORTER_ASSERT(reporter, testCanvas.getTotalMatrix().isIdentity());
    }
}

static void test_peephole() {
    SkRandom rand;

    SkPictureRecorder recorder;

    for (int j = 0; j < 100; j++) {
        SkRandom rand2(rand); // remember the seed

        SkCanvas* canvas = recorder.beginRecording(100, 100);

        for (int i = 0; i < 1000; ++i) {
            rand_op(canvas, rand);
        }
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

        rand = rand2;
    }

    {
        SkCanvas* canvas = recorder.beginRecording(100, 100);
        SkRect rect = SkRect::MakeWH(50, 50);

        for (int i = 0; i < 100; ++i) {
            canvas->save();
        }
        while (canvas->getSaveCount() > 1) {
            canvas->clipRect(rect);
            canvas->restore();
        }
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    }
}

static void test_bad_bitmap(skiatest::Reporter* reporter) {
    // missing pixels should return null for image
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(100, 100));
    auto img = bm.asImage();
    REPORTER_ASSERT(reporter, !img);

    // make sure we don't crash on a null image
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(100, 100);
    recordingCanvas->drawImage(nullptr, 0, 0);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    SkCanvas canvas;
    canvas.drawPicture(picture);
}

static void test_clip_bound_opt(skiatest::Reporter* reporter) {
    // Test for crbug.com/229011
    SkRect rect1 = SkRect::MakeXYWH(SkIntToScalar(4), SkIntToScalar(4),
                                    SkIntToScalar(2), SkIntToScalar(2));
    SkRect rect2 = SkRect::MakeXYWH(SkIntToScalar(7), SkIntToScalar(7),
                                    SkIntToScalar(1), SkIntToScalar(1));
    SkRect rect3 = SkRect::MakeXYWH(SkIntToScalar(6), SkIntToScalar(6),
                                    SkIntToScalar(1), SkIntToScalar(1));

    SkPath invPath;
    invPath.addOval(rect1);
    invPath.setFillType(SkPathFillType::kInverseEvenOdd);
    SkPath path;
    path.addOval(rect2);
    SkPath path2;
    path2.addOval(rect3);
    SkIRect clipBounds;
    SkPictureRecorder recorder;

    // Testing conservative-raster-clip that is enabled by PictureRecord
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(invPath);
        clipBounds = canvas->getDeviceClipBounds();
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path);
        canvas->clipPath(invPath);
        clipBounds = canvas->getDeviceClipBounds();
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, SkClipOp::kDifference);
        clipBounds = canvas->getDeviceClipBounds();
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, SkClipOp::kIntersect);
        canvas->clipPath(path2, SkClipOp::kDifference);
        clipBounds = canvas->getDeviceClipBounds();
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
}

static void test_cull_rect_reset(skiatest::Reporter* reporter) {
    SkPictureRecorder recorder;
    SkRect bounds = SkRect::MakeWH(10, 10);
    SkRTreeFactory factory;
    SkCanvas* canvas = recorder.beginRecording(bounds, &factory);
    bounds = SkRect::MakeWH(100, 100);
    SkPaint paint;
    canvas->drawRect(bounds, paint);
    canvas->drawRect(bounds, paint);
    sk_sp<SkPicture> p(recorder.finishRecordingAsPictureWithCull(bounds));
    const SkBigPicture* picture = SkPicturePriv::AsSkBigPicture(p);
    REPORTER_ASSERT(reporter, picture);

    SkRect finalCullRect = picture->cullRect();
    REPORTER_ASSERT(reporter, 0 == finalCullRect.fLeft);
    REPORTER_ASSERT(reporter, 0 == finalCullRect.fTop);
    REPORTER_ASSERT(reporter, 100 == finalCullRect.fBottom);
    REPORTER_ASSERT(reporter, 100 == finalCullRect.fRight);
}


/**
 * A canvas that records the number of clip commands.
 */
class ClipCountingCanvas : public SkCanvas {
public:
    ClipCountingCanvas(int width, int height)
        : INHERITED(width, height)
        , fClipCount(0){
    }

    void onClipRect(const SkRect& r, SkClipOp op, ClipEdgeStyle edgeStyle) override {
        fClipCount += 1;
        this->INHERITED::onClipRect(r, op, edgeStyle);
    }

    void onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle)override {
        fClipCount += 1;
        this->INHERITED::onClipRRect(rrect, op, edgeStyle);
    }

    void onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) override {
        fClipCount += 1;
        this->INHERITED::onClipPath(path, op, edgeStyle);
    }

    void onClipRegion(const SkRegion& deviceRgn, SkClipOp op) override {
        fClipCount += 1;
        this->INHERITED::onClipRegion(deviceRgn, op);
    }

    unsigned getClipCount() const { return fClipCount; }

private:
    unsigned fClipCount;

    using INHERITED = SkCanvas;
};

static void test_gen_id(skiatest::Reporter* reporter) {

    SkPictureRecorder recorder;
    recorder.beginRecording(0, 0);
    sk_sp<SkPicture> empty(recorder.finishRecordingAsPicture());

    // Empty pictures should still have a valid ID
    REPORTER_ASSERT(reporter, empty->uniqueID() != SK_InvalidGenID);

    SkCanvas* canvas = recorder.beginRecording(1, 1);
    canvas->drawColor(SK_ColorWHITE);
    sk_sp<SkPicture> hasData(recorder.finishRecordingAsPicture());
    // picture should have a non-zero id after recording
    REPORTER_ASSERT(reporter, hasData->uniqueID() != SK_InvalidGenID);

    // both pictures should have different ids
    REPORTER_ASSERT(reporter, hasData->uniqueID() != empty->uniqueID());
}

static void test_typeface(skiatest::Reporter* reporter) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(10, 10);
    SkFont font(ToolUtils::CreateTestTypeface("Arial", SkFontStyle::Italic()));
    canvas->drawString("Q", 0, 10, font, SkPaint());
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    SkDynamicMemoryWStream stream;
    picture->serialize(&stream, nullptr);  // default SkSerialProcs
}

DEF_TEST(Picture, reporter) {
    test_typeface(reporter);
#ifdef SK_DEBUG
    test_deleting_empty_picture();
    test_serializing_empty_picture();
#endif
    test_bad_bitmap(reporter);
    test_unbalanced_save_restores(reporter);
    test_peephole();
    test_clip_bound_opt(reporter);
    test_gen_id(reporter);
    test_cull_rect_reset(reporter);
}

static void draw_bitmaps(const SkBitmap& bitmap, SkCanvas* canvas) {
    const SkRect rect = { 5.0f, 5.0f, 8.0f, 8.0f };
    auto img = bitmap.asImage();

    // Don't care what these record, as long as they're legal.
    canvas->drawImage(img, 0.0f, 0.0f);
    canvas->drawImageRect(img, rect, rect, SkSamplingOptions(), nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);
    canvas->drawImage(img, 1, 1);   // drawSprite
}

static void test_draw_bitmaps(SkCanvas* canvas) {
    SkBitmap empty;
    draw_bitmaps(empty, canvas);
    empty.setInfo(SkImageInfo::MakeN32Premul(10, 10));
    draw_bitmaps(empty, canvas);
}

DEF_TEST(Picture_EmptyBitmap, r) {
    SkPictureRecorder recorder;
    test_draw_bitmaps(recorder.beginRecording(10, 10));
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
}

DEF_TEST(Canvas_EmptyBitmap, r) {
    SkBitmap dst;
    dst.allocN32Pixels(10, 10);
    SkCanvas canvas(dst);

    test_draw_bitmaps(&canvas);
}

DEF_TEST(DontOptimizeSaveLayerDrawDrawRestore, reporter) {
    // This test is from crbug.com/344987.
    // The commands are:
    //   saveLayer with paint that modifies alpha
    //     drawBitmapRect
    //     drawBitmapRect
    //   restore
    // The bug was that this structure was modified so that:
    //  - The saveLayer and restore were eliminated
    //  - The alpha was only applied to the first drawBitmapRectToRect

    // This test draws blue and red squares inside a 50% transparent
    // layer.  Both colours should show up muted.
    // When the bug is present, the red square (the second bitmap)
    // shows upwith full opacity.

    SkBitmap blueBM;
    make_bm(&blueBM, 100, 100, SkColorSetARGB(255, 0, 0, 255), true);
    SkBitmap redBM;
    make_bm(&redBM, 100, 100, SkColorSetARGB(255, 255, 0, 0), true);
    SkPaint semiTransparent;
    semiTransparent.setAlpha(0x80);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(100, 100);
    canvas->drawColor(0);

    canvas->saveLayer(nullptr, &semiTransparent);
    canvas->drawImage(blueBM.asImage(), 25, 25);
    canvas->drawImage(redBM.asImage(), 50, 50);
    canvas->restore();

    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    // Now replay the picture back on another canvas
    // and check a couple of its pixels.
    SkBitmap replayBM;
    make_bm(&replayBM, 100, 100, SK_ColorBLACK, false);
    SkCanvas replayCanvas(replayBM);
    picture->playback(&replayCanvas);

    // With the bug present, at (55, 55) we would get a fully opaque red
    // intead of a dark red.
    REPORTER_ASSERT(reporter, replayBM.getColor(30, 30) == 0xff000080);
    REPORTER_ASSERT(reporter, replayBM.getColor(55, 55) == 0xff800000);
}

struct CountingBBH : public SkBBoxHierarchy {
    mutable int searchCalls;

    CountingBBH() : searchCalls(0) {}

    void search(const SkRect& query, std::vector<int>* results) const override {
        this->searchCalls++;
    }

    void insert(const SkRect[], int) override {}
    size_t bytesUsed() const override { return 0; }
};

class SpoonFedBBHFactory : public SkBBHFactory {
public:
    explicit SpoonFedBBHFactory(sk_sp<SkBBoxHierarchy> bbh) : fBBH(std::move(bbh)) {}
    sk_sp<SkBBoxHierarchy> operator()() const override {
        return fBBH;
    }
private:
    sk_sp<SkBBoxHierarchy> fBBH;
};

// When the canvas clip covers the full picture, we don't need to call the BBH.
DEF_TEST(Picture_SkipBBH, r) {
    SkRect bound = SkRect::MakeWH(320, 240);

    auto bbh = sk_make_sp<CountingBBH>();
    SpoonFedBBHFactory factory(bbh);

    SkPictureRecorder recorder;
    SkCanvas* c = recorder.beginRecording(bound, &factory);
    // Record a few ops so we don't hit a small- or empty- picture optimization.
        c->drawRect(bound, SkPaint());
        c->drawRect(bound, SkPaint());
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    SkCanvas big(640, 480), small(300, 200);

    picture->playback(&big);
    REPORTER_ASSERT(r, bbh->searchCalls == 0);

    picture->playback(&small);
    REPORTER_ASSERT(r, bbh->searchCalls == 1);
}

DEF_TEST(Picture_BitmapLeak, r) {
    SkBitmap mut, immut;
    mut.allocN32Pixels(300, 200);
    immut.allocN32Pixels(300, 200);
    immut.setImmutable();
    SkASSERT(!mut.isImmutable());
    SkASSERT(immut.isImmutable());

    // No one can hold a ref on our pixels yet.
    REPORTER_ASSERT(r, mut.pixelRef()->unique());
    REPORTER_ASSERT(r, immut.pixelRef()->unique());

    sk_sp<SkPicture> pic;
    {
        // we want the recorder to go out of scope before our subsequent checks, so we
        // place it inside local braces.
        SkPictureRecorder rec;
        SkCanvas* canvas = rec.beginRecording(1920, 1200);
            canvas->drawImage(mut.asImage(), 0, 0);
            canvas->drawImage(immut.asImage(), 800, 600);
        pic = rec.finishRecordingAsPicture();
    }

    // The picture shares the immutable pixels but copies the mutable ones.
    REPORTER_ASSERT(r, mut.pixelRef()->unique());
    REPORTER_ASSERT(r, !immut.pixelRef()->unique());

    // When the picture goes away, it's just our bitmaps holding the refs.
    pic = nullptr;
    REPORTER_ASSERT(r, mut.pixelRef()->unique());
    REPORTER_ASSERT(r, immut.pixelRef()->unique());
}

// getRecordingCanvas() should return a SkCanvas when recording, null when not recording.
DEF_TEST(Picture_getRecordingCanvas, r) {
    SkPictureRecorder rec;
    REPORTER_ASSERT(r, !rec.getRecordingCanvas());
    for (int i = 0; i < 3; i++) {
        rec.beginRecording(100, 100);
        REPORTER_ASSERT(r, rec.getRecordingCanvas());
        rec.finishRecordingAsPicture();
        REPORTER_ASSERT(r, !rec.getRecordingCanvas());
    }
}

DEF_TEST(Picture_preserveCullRect, r) {
    SkPictureRecorder recorder;

    SkCanvas* c = recorder.beginRecording(SkRect::MakeLTRB(1, 2, 3, 4));
    c->clear(SK_ColorCYAN);

    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    SkDynamicMemoryWStream wstream;
    picture->serialize(&wstream, nullptr);  // default SkSerialProcs

    std::unique_ptr<SkStream> rstream(wstream.detachAsStream());
    sk_sp<SkPicture> deserializedPicture(SkPicture::MakeFromStream(rstream.get()));

    REPORTER_ASSERT(r, deserializedPicture != nullptr);
    REPORTER_ASSERT(r, deserializedPicture->cullRect().left() == 1);
    REPORTER_ASSERT(r, deserializedPicture->cullRect().top() == 2);
    REPORTER_ASSERT(r, deserializedPicture->cullRect().right() == 3);
    REPORTER_ASSERT(r, deserializedPicture->cullRect().bottom() == 4);
}


// If we record bounded ops into a picture with a big cull and calculate the
// bounds of those ops, we should trim down the picture cull to the ops' bounds.
// If we're not using an SkBBH, we shouldn't change it.
DEF_TEST(Picture_UpdatedCull_1, r) {
    SkRTreeFactory factory;
    SkPictureRecorder recorder;

    auto canvas = recorder.beginRecording(SkRectPriv::MakeLargest(), &factory);
    canvas->drawRect(SkRect::MakeWH(20,20), SkPaint{});
    auto pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->cullRect() == SkRect::MakeWH(20,20));

    canvas = recorder.beginRecording(SkRectPriv::MakeLargest());
    canvas->drawRect(SkRect::MakeWH(20,20), SkPaint{});
    pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->cullRect() == SkRectPriv::MakeLargest());
}
DEF_TEST(Picture_UpdatedCull_2, r) {
    SkRTreeFactory factory;
    SkPictureRecorder recorder;

    auto canvas = recorder.beginRecording(SkRectPriv::MakeLargest(), &factory);
    canvas->drawRect(SkRect::MakeWH(20,20), SkPaint{});
    canvas->drawRect(SkRect::MakeWH(10,40), SkPaint{});
    auto pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->cullRect() == SkRect::MakeWH(20,40));

    canvas = recorder.beginRecording(SkRectPriv::MakeLargest());
    canvas->drawRect(SkRect::MakeWH(20,20), SkPaint{});
    canvas->drawRect(SkRect::MakeWH(10,40), SkPaint{});
    pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->cullRect() == SkRectPriv::MakeLargest());
}

DEF_TEST(Placeholder, r) {
    SkRect cull = { 0,0, 10,20 };

    // Each placeholder is unique.
    sk_sp<SkPicture> p1 = SkPicture::MakePlaceholder(cull),
                     p2 = SkPicture::MakePlaceholder(cull);
    REPORTER_ASSERT(r, p1->cullRect() == p2->cullRect());
    REPORTER_ASSERT(r, p1->cullRect() == cull);
    REPORTER_ASSERT(r, p1->uniqueID() != p2->uniqueID());

    // Placeholders are never unrolled by SkCanvas (while other small pictures may be).
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(cull);
        canvas->drawPicture(p1);
        canvas->drawPicture(p2);
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->approximateOpCount() == 2);

    // Any upper limit when recursing into nested placeholders is fine as long
    // as it doesn't overflow an int.
    REPORTER_ASSERT(r, pic->approximateOpCount(/*nested?*/true) >=  2);
    REPORTER_ASSERT(r, pic->approximateOpCount(/*nested?*/true) <= 10);
}

DEF_TEST(Picture_empty_serial, reporter) {
    SkPictureRecorder rec;
    (void)rec.beginRecording(10, 10);
    auto pic = rec.finishRecordingAsPicture();
    REPORTER_ASSERT(reporter, pic);

    auto data = pic->serialize(); // explicitly testing the default SkSerialProcs
    REPORTER_ASSERT(reporter, data);

    auto pic2 = SkPicture::MakeFromData(data->data(), data->size());
    REPORTER_ASSERT(reporter, pic2);
}


DEF_TEST(Picture_drawsNothing, r) {
    // Tests that pic->cullRect().isEmpty() is a good way to test a picture
    // recorded with an R-tree draws nothing.
    struct {
        bool draws_nothing;
        void (*fn)(SkCanvas*);
    } cases[] = {
        {  true, [](SkCanvas* c) {                                                             } },
        {  true, [](SkCanvas* c) { c->save();                                    c->restore(); } },
        {  true, [](SkCanvas* c) { c->save(); c->clipRect({0,0,5,5});            c->restore(); } },
        {  true, [](SkCanvas* c) {            c->clipRect({0,0,5,5});                          } },

        { false, [](SkCanvas* c) {            c->drawRect({0,0,5,5}, SkPaint{});               } },
        { false, [](SkCanvas* c) { c->save(); c->drawRect({0,0,5,5}, SkPaint{}); c->restore(); } },
        { false, [](SkCanvas* c) {
            c->drawRect({0,0, 5, 5}, SkPaint{});
            c->drawRect({5,5,10,10}, SkPaint{});
        }},
    };

    for (const auto& c : cases) {
        SkPictureRecorder rec;
        SkRTreeFactory factory;
        c.fn(rec.beginRecording(10,10, &factory));
        sk_sp<SkPicture> pic = rec.finishRecordingAsPicture();

        REPORTER_ASSERT(r, pic->cullRect().isEmpty() == c.draws_nothing);
    }
}

DEF_TEST(Picture_emptyNestedPictureBug, r) {
    const SkRect bounds = {-5000, -5000, 5000, 5000};

    SkPictureRecorder recorder;
    SkRTreeFactory factory;

    // These three pictures should all draw the same but due to bugs they don't:
    //
    //   1) inner has enough content that it is recoreded as an SkBigPicture,
    //      and all its content falls outside the positive/positive quadrant,
    //      and it is recorded with an R-tree so we contract the cullRect to those bounds;
    //
    //   2) middle wraps inner,
    //      and it its recorded with an R-tree so we update middle's cullRect to inner's;
    //
    //   3) outer wraps inner,
    //      and notices that middle contains only one op, drawPicture(inner),
    //      so it plays middle back during recording rather than ref'ing middle,
    //      querying middle's R-tree with its SkCanvas' bounds* {0,0, 5000,5000},
    //      finding nothing to draw.
    //
    //  * The bug was that these bounds were not tracked as {-5000,-5000, 5000,5000}.
    {
        SkCanvas* canvas = recorder.beginRecording(bounds, &factory);
        canvas->translate(-100,-100);
        canvas->drawRect({0,0,50,50}, SkPaint{});
    }
    sk_sp<SkPicture> inner = recorder.finishRecordingAsPicture();

    recorder.beginRecording(bounds, &factory)->drawPicture(inner);
    sk_sp<SkPicture> middle = recorder.finishRecordingAsPicture();

    // This doesn't need &factory to reproduce the bug,
    // but it's nice to see we come up with the same {-100,-100, -50,-50} bounds.
    recorder.beginRecording(bounds, &factory)->drawPicture(middle);
    sk_sp<SkPicture> outer = recorder.finishRecordingAsPicture();

    REPORTER_ASSERT(r, (inner ->cullRect() == SkRect{-100,-100, -50,-50}));
    REPORTER_ASSERT(r, (middle->cullRect() == SkRect{-100,-100, -50,-50}));
    REPORTER_ASSERT(r, (outer ->cullRect() == SkRect{-100,-100, -50,-50}));   // Used to fail.
}

DEF_TEST(Picture_fillsBBH, r) {
    // Test empty (0 draws), mini (1 draw), and big (2+) pictures, making sure they fill the BBH.
    const SkRect rects[] = {
        { 0, 0, 20,20},
        {20,20, 40,40},
    };

    for (int n = 0; n <= 2; n++) {
        SkRTreeFactory factory;
        SkPictureRecorder rec;

        sk_sp<SkBBoxHierarchy> bbh = factory();

        SkCanvas* c = rec.beginRecording({0,0, 100,100}, bbh);
        for (int i = 0; i < n; i++) {
            c->drawRect(rects[i], SkPaint{});
        }
        sk_sp<SkPicture> pic = rec.finishRecordingAsPicture();

        std::vector<int> results;
        bbh->search({0,0, 100,100}, &results);
        REPORTER_ASSERT(r, (int)results.size() == n,
                        "results.size() == %d, want %d\n", (int)results.size(), n);
    }
}

DEF_TEST(Picture_nested_op_count, r) {
    auto make_pic = [](int n, const sk_sp<SkPicture>& pic) {
        SkPictureRecorder rec;
        SkCanvas* c = rec.beginRecording({0,0, 100,100});
        for (int i = 0; i < n; i++) {
            if (pic) {
                c->drawPicture(pic);
            } else {
                c->drawRect({0,0, 100,100}, SkPaint{});
            }
        }
        return rec.finishRecordingAsPicture();
    };

    auto check = [r](const sk_sp<SkPicture>& pic, int shallow, int nested) {
        int s = pic->approximateOpCount(false);
        int n = pic->approximateOpCount(true);
        REPORTER_ASSERT(r, s == shallow);
        REPORTER_ASSERT(r, n == nested);
    };

    sk_sp<SkPicture> leaf1 = make_pic(1, nullptr);
    check(leaf1, 1, 1);

    sk_sp<SkPicture> leaf10 = make_pic(10, nullptr);
    check(leaf10, 10, 10);

    check(make_pic( 1, leaf1),   1,   1);
    check(make_pic( 1, leaf10),  1,  10);
    check(make_pic(10, leaf1),  10,  10);
    check(make_pic(10, leaf10), 10, 100);
}
