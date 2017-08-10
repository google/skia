/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBigPicture.h"
#include "SkBBoxHierarchy.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorMatrixFilter.h"
#include "SkColorPriv.h"
#include "SkDashPathEffect.h"
#include "SkData.h"
#include "SkImageGenerator.h"
#include "SkImageEncoder.h"
#include "SkImageGenerator.h"
#include "SkMD5.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureAnalyzer.h"
#include "SkPictureRecorder.h"
#include "SkPixelRef.h"
#include "SkPixelSerializer.h"
#include "SkMiniRecorder.h"
#include "SkRRect.h"
#include "SkRandom.h"
#include "SkRecord.h"
#include "SkShader.h"
#include "SkStream.h"
#include "sk_tool_utils.h"

#include "Test.h"

#include "SkLumaColorFilter.h"
#include "SkColorFilterImageFilter.h"

static void make_bm(SkBitmap* bm, int w, int h, SkColor color, bool immutable) {
    bm->allocN32Pixels(w, h);
    bm->eraseColor(color);
    if (immutable) {
        bm->setImmutable();
    }
}

// For a while willPlayBackBitmaps() ignored SkImages and just looked for SkBitmaps.
static void test_images_are_found_by_willPlayBackBitmaps(skiatest::Reporter* reporter) {
    // We just need _some_ SkImage
    const SkPMColor pixel = 0;
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    sk_sp<SkImage> image(SkImage::MakeRasterCopy(SkPixmap(info, &pixel, sizeof(pixel))));

    SkPictureRecorder recorder;
    recorder.beginRecording(100,100)->drawImage(image, 0,0);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    REPORTER_ASSERT(reporter, picture->willPlayBackBitmaps());
}

/* Hit a few SkPicture::Analysis cases not handled elsewhere. */
static void test_analysis(skiatest::Reporter* reporter) {
    SkPictureRecorder recorder;

    SkCanvas* canvas = recorder.beginRecording(100, 100);
    {
        canvas->drawRect(SkRect::MakeWH(10, 10), SkPaint ());
    }
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    REPORTER_ASSERT(reporter, !picture->willPlayBackBitmaps());

    canvas = recorder.beginRecording(100, 100);
    {
        SkPaint paint;
        // CreateBitmapShader is too smart for us; an empty (or 1x1) bitmap shader
        // gets optimized into a non-bitmap form, so we create a 2x2 bitmap here.
        SkBitmap bitmap;
        bitmap.allocPixels(SkImageInfo::MakeN32Premul(2, 2));
        bitmap.eraseColor(SK_ColorBLUE);
        *(bitmap.getAddr32(0, 0)) = SK_ColorGREEN;
        paint.setShader(SkShader::MakeBitmapShader(bitmap, SkShader::kClamp_TileMode,
                                                   SkShader::kClamp_TileMode));
        REPORTER_ASSERT(reporter, paint.getShader()->isAImage());

        canvas->drawRect(SkRect::MakeWH(10, 10), paint);
    }
    REPORTER_ASSERT(reporter, recorder.finishRecordingAsPicture()->willPlayBackBitmaps());
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
    picture->serialize(&stream);
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

#if SK_SUPPORT_GPU

static SkPath make_convex_path() {
    SkPath path;
    path.lineTo(100, 0);
    path.lineTo(50, 100);
    path.close();

    return path;
}

static SkPath make_concave_path() {
    SkPath path;
    path.lineTo(50, 50);
    path.lineTo(100, 0);
    path.lineTo(50, 100);
    path.close();

    return path;
}

static void test_gpu_veto(skiatest::Reporter* reporter) {
    SkPictureRecorder recorder;

    SkCanvas* canvas = recorder.beginRecording(100, 100);
    {
        SkPath path;
        path.moveTo(0, 0);
        path.lineTo(50, 50);

        SkScalar intervals[] = { 1.0f, 1.0f };
        sk_sp<SkPathEffect> dash(SkDashPathEffect::Make(intervals, 2, 0));

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setPathEffect(dash);

        for (int i = 0; i < 50; ++i) {
            canvas->drawPath(path, paint);
        }
    }
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    // path effects currently render an SkPicture undesireable for GPU rendering

    const char *reason = nullptr;
    REPORTER_ASSERT(reporter,
        !SkPictureGpuAnalyzer(picture).suitableForGpuRasterization(&reason));
    REPORTER_ASSERT(reporter, reason);

    canvas = recorder.beginRecording(100, 100);
    {
        SkPath path;

        path.moveTo(0, 0);
        path.lineTo(0, 50);
        path.lineTo(25, 25);
        path.lineTo(50, 50);
        path.lineTo(50, 0);
        path.close();
        REPORTER_ASSERT(reporter, !path.isConvex());

        SkPaint paint;
        paint.setAntiAlias(true);
        for (int i = 0; i < 50; ++i) {
            canvas->drawPath(path, paint);
        }
    }
    picture = recorder.finishRecordingAsPicture();
    // A lot of small AA concave paths should be fine for GPU rendering
    REPORTER_ASSERT(reporter, SkPictureGpuAnalyzer(picture).suitableForGpuRasterization());

    canvas = recorder.beginRecording(100, 100);
    {
        SkPath path;

        path.moveTo(0, 0);
        path.lineTo(0, 100);
        path.lineTo(50, 50);
        path.lineTo(100, 100);
        path.lineTo(100, 0);
        path.close();
        REPORTER_ASSERT(reporter, !path.isConvex());

        SkPaint paint;
        paint.setAntiAlias(true);
        for (int i = 0; i < 50; ++i) {
            canvas->drawPath(path, paint);
        }
    }
    picture = recorder.finishRecordingAsPicture();
    // A lot of large AA concave paths currently render an SkPicture undesireable for GPU rendering
    REPORTER_ASSERT(reporter, !SkPictureGpuAnalyzer(picture).suitableForGpuRasterization());

    canvas = recorder.beginRecording(100, 100);
    {
        SkPath path;

        path.moveTo(0, 0);
        path.lineTo(0, 50);
        path.lineTo(25, 25);
        path.lineTo(50, 50);
        path.lineTo(50, 0);
        path.close();
        REPORTER_ASSERT(reporter, !path.isConvex());

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0);
        for (int i = 0; i < 50; ++i) {
            canvas->drawPath(path, paint);
        }
    }
    picture = recorder.finishRecordingAsPicture();
    // hairline stroked AA concave paths are fine for GPU rendering
    REPORTER_ASSERT(reporter, SkPictureGpuAnalyzer(picture).suitableForGpuRasterization());

    canvas = recorder.beginRecording(100, 100);
    {
        SkPaint paint;
        SkScalar intervals [] = { 10, 20 };
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 25));

        SkPoint points [2] = { { 0, 0 }, { 100, 0 } };

        for (int i = 0; i < 50; ++i) {
            canvas->drawPoints(SkCanvas::kLines_PointMode, 2, points, paint);
        }
    }
    picture = recorder.finishRecordingAsPicture();
    // fast-path dashed effects are fine for GPU rendering ...
    REPORTER_ASSERT(reporter, SkPictureGpuAnalyzer(picture).suitableForGpuRasterization());

    canvas = recorder.beginRecording(100, 100);
    {
        SkPaint paint;
        SkScalar intervals [] = { 10, 20 };
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 25));

        for (int i = 0; i < 50; ++i) {
            canvas->drawRect(SkRect::MakeWH(10, 10), paint);
        }
    }
    picture = recorder.finishRecordingAsPicture();
    // ... but only when applied to drawPoint() calls
    REPORTER_ASSERT(reporter, !SkPictureGpuAnalyzer(picture).suitableForGpuRasterization());

    canvas = recorder.beginRecording(100, 100);
    {
        const SkPath convexClip = make_convex_path();
        const SkPath concaveClip = make_concave_path();

        for (int i = 0; i < 50; ++i) {
            canvas->clipPath(convexClip);
            canvas->clipPath(concaveClip);
            canvas->clipPath(convexClip, kIntersect_SkClipOp, true);
            canvas->drawRect(SkRect::MakeWH(100, 100), SkPaint());
        }
    }
    picture = recorder.finishRecordingAsPicture();
    // Convex clips and non-AA concave clips are fine on the GPU.
    REPORTER_ASSERT(reporter, SkPictureGpuAnalyzer(picture).suitableForGpuRasterization());

    canvas = recorder.beginRecording(100, 100);
    {
        const SkPath concaveClip = make_concave_path();
        for (int i = 0; i < 50; ++i) {
            canvas->clipPath(concaveClip, kIntersect_SkClipOp, true);
            canvas->drawRect(SkRect::MakeWH(100, 100), SkPaint());
        }
    }
    picture = recorder.finishRecordingAsPicture();
    // ... but AA concave clips are not.
    REPORTER_ASSERT(reporter, !SkPictureGpuAnalyzer(picture).suitableForGpuRasterization());

    // Nest the previous picture inside a new one.
    canvas = recorder.beginRecording(100, 100);
    {
        canvas->drawPicture(picture);
    }
    picture = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(reporter, !SkPictureGpuAnalyzer(picture).suitableForGpuRasterization());
}

#endif // SK_SUPPORT_GPU

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
        , fRestoreCount(0){
    }

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
        ++fSaveLayerCount;
        return this->INHERITED::getSaveLayerStrategy(rec);
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
    unsigned int getRestoreCount() const { return fRestoreCount; }

private:
    unsigned int fSaveCount;
    unsigned int fSaveLayerCount;
    unsigned int fRestoreCount;

    typedef SkCanvas INHERITED;
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
        canvas->clipRect(clipRect, kReplace_SkClipOp);
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

    // (partially) check leakage of draw ops
    {
        SkPictureRecorder recorder;

        SkCanvas* canvas = recorder.beginRecording(10, 10);

        SkRect r = SkRect::MakeWH(5, 5);
        SkPaint p;

        canvas->drawRect(r, p);

        sk_sp<SkPicture> copy(SkPictureRecorderReplayTester::Copy(&recorder));

        REPORTER_ASSERT(reporter, !copy->willPlayBackBitmaps());

        SkBitmap bm;
        make_bm(&bm, 10, 10, SK_ColorRED, true);

        r.offset(5.0f, 5.0f);
        canvas->drawBitmapRect(bm, r, nullptr);

        sk_sp<SkPicture> final(recorder.finishRecordingAsPicture());
        REPORTER_ASSERT(reporter, final->willPlayBackBitmaps());

        REPORTER_ASSERT(reporter, copy->uniqueID() != final->uniqueID());

        // The snapshot shouldn't pick up any operations added after it was made
        REPORTER_ASSERT(reporter, !copy->willPlayBackBitmaps());
    }

    // Recreate the Android partialReplay test case
    {
        SkPictureRecorder recorder;

        SkCanvas* canvas = recorder.beginRecording(4, 3, nullptr, 0);
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

#ifndef SK_DEBUG
// Only test this is in release mode. We deliberately crash in debug mode, since a valid caller
// should never do this.
static void test_bad_bitmap() {
    // This bitmap has a width and height but no pixels. As a result, attempting to record it will
    // fail.
    SkBitmap bm;
    bm.setInfo(SkImageInfo::MakeN32Premul(100, 100));
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(100, 100);
    recordingCanvas->drawBitmap(bm, 0, 0);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    SkCanvas canvas;
    canvas.drawPicture(picture);
}
#endif

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
    invPath.setFillType(SkPath::kInverseEvenOdd_FillType);
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
        canvas->clipPath(path);
        canvas->clipPath(invPath, kUnion_SkClipOp);
        clipBounds = canvas->getDeviceClipBounds();
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, kDifference_SkClipOp);
        clipBounds = canvas->getDeviceClipBounds();
        REPORTER_ASSERT(reporter, 0 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 0 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 10 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, kReverseDifference_SkClipOp);
        clipBounds = canvas->getDeviceClipBounds();
        // True clip is actually empty in this case, but the best
        // determination we can make using only bounds as input is that the
        // clip is included in the bounds of 'path'.
        REPORTER_ASSERT(reporter, 7 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 7 == clipBounds.fTop);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fBottom);
        REPORTER_ASSERT(reporter, 8 == clipBounds.fRight);
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->clipPath(path, kIntersect_SkClipOp);
        canvas->clipPath(path2, kXOR_SkClipOp);
        clipBounds = canvas->getDeviceClipBounds();
        REPORTER_ASSERT(reporter, 6 == clipBounds.fLeft);
        REPORTER_ASSERT(reporter, 6 == clipBounds.fTop);
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
    const SkBigPicture* picture = p->asSkBigPicture();
    REPORTER_ASSERT(reporter, picture);

    SkRect finalCullRect = picture->cullRect();
    REPORTER_ASSERT(reporter, 0 == finalCullRect.fLeft);
    REPORTER_ASSERT(reporter, 0 == finalCullRect.fTop);
    REPORTER_ASSERT(reporter, 100 == finalCullRect.fBottom);
    REPORTER_ASSERT(reporter, 100 == finalCullRect.fRight);

    const SkBBoxHierarchy* pictureBBH = picture->bbh();
    SkRect bbhCullRect = pictureBBH->getRootBound();
    REPORTER_ASSERT(reporter, 0 == bbhCullRect.fLeft);
    REPORTER_ASSERT(reporter, 0 == bbhCullRect.fTop);
    REPORTER_ASSERT(reporter, 100 == bbhCullRect.fBottom);
    REPORTER_ASSERT(reporter, 100 == bbhCullRect.fRight);
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

    typedef SkCanvas INHERITED;
};

static void test_clip_expansion(skiatest::Reporter* reporter) {
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(10, 10);

    canvas->clipRect(SkRect::MakeEmpty(), kReplace_SkClipOp);
    // The following expanding clip should not be skipped.
    canvas->clipRect(SkRect::MakeXYWH(4, 4, 3, 3), kUnion_SkClipOp);
    // Draw something so the optimizer doesn't just fold the world.
    SkPaint p;
    p.setColor(SK_ColorBLUE);
    canvas->drawPaint(p);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    ClipCountingCanvas testCanvas(10, 10);
    picture->playback(&testCanvas);

    // Both clips should be present on playback.
    REPORTER_ASSERT(reporter, testCanvas.getClipCount() == 2);
}

static void test_hierarchical(skiatest::Reporter* reporter) {
    SkBitmap bm;
    make_bm(&bm, 10, 10, SK_ColorRED, true);

    SkPictureRecorder recorder;

    recorder.beginRecording(10, 10);
    sk_sp<SkPicture> childPlain(recorder.finishRecordingAsPicture());
    REPORTER_ASSERT(reporter, !childPlain->willPlayBackBitmaps()); // 0

    recorder.beginRecording(10, 10)->drawBitmap(bm, 0, 0);
    sk_sp<SkPicture> childWithBitmap(recorder.finishRecordingAsPicture());
    REPORTER_ASSERT(reporter, childWithBitmap->willPlayBackBitmaps()); // 1

    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->drawPicture(childPlain);
        sk_sp<SkPicture> parentPP(recorder.finishRecordingAsPicture());
        REPORTER_ASSERT(reporter, !parentPP->willPlayBackBitmaps()); // 0
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->drawPicture(childWithBitmap);
        sk_sp<SkPicture> parentPWB(recorder.finishRecordingAsPicture());
        REPORTER_ASSERT(reporter, parentPWB->willPlayBackBitmaps()); // 1
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->drawBitmap(bm, 0, 0);
        canvas->drawPicture(childPlain);
        sk_sp<SkPicture> parentWBP(recorder.finishRecordingAsPicture());
        REPORTER_ASSERT(reporter, parentWBP->willPlayBackBitmaps()); // 1
    }
    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        canvas->drawBitmap(bm, 0, 0);
        canvas->drawPicture(childWithBitmap);
        sk_sp<SkPicture> parentWBWB(recorder.finishRecordingAsPicture());
        REPORTER_ASSERT(reporter, parentWBWB->willPlayBackBitmaps()); // 2
    }
}

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
    SkPaint paint;
    paint.setTypeface(SkTypeface::MakeFromName("Arial",
                                               SkFontStyle::FromOldStyle(SkTypeface::kItalic)));
    canvas->drawString("Q", 0, 10, paint);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    SkDynamicMemoryWStream stream;
    picture->serialize(&stream);
}

DEF_TEST(Picture, reporter) {
    test_typeface(reporter);
#ifdef SK_DEBUG
    test_deleting_empty_picture();
    test_serializing_empty_picture();
#else
    test_bad_bitmap();
#endif
    test_unbalanced_save_restores(reporter);
    test_peephole();
#if SK_SUPPORT_GPU
    test_gpu_veto(reporter);
#endif
    test_images_are_found_by_willPlayBackBitmaps(reporter);
    test_analysis(reporter);
    test_clip_bound_opt(reporter);
    test_clip_expansion(reporter);
    test_hierarchical(reporter);
    test_gen_id(reporter);
    test_cull_rect_reset(reporter);
}

static void draw_bitmaps(const SkBitmap bitmap, SkCanvas* canvas) {
    const SkPaint paint;
    const SkRect rect = { 5.0f, 5.0f, 8.0f, 8.0f };
    const SkIRect irect =  { 2, 2, 3, 3 };
    int divs[] = { 2, 3 };
    SkCanvas::Lattice lattice;
    lattice.fXCount = lattice.fYCount = 2;
    lattice.fXDivs = lattice.fYDivs = divs;

    // Don't care what these record, as long as they're legal.
    canvas->drawBitmap(bitmap, 0.0f, 0.0f, &paint);
    canvas->drawBitmapRect(bitmap, rect, rect, &paint, SkCanvas::kStrict_SrcRectConstraint);
    canvas->drawBitmapNine(bitmap, irect, rect, &paint);
    canvas->drawBitmap(bitmap, 1, 1);   // drawSprite
    canvas->drawBitmapLattice(bitmap, lattice, rect, &paint);
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

    canvas->saveLayer(0, &semiTransparent);
    canvas->drawBitmap(blueBM, 25, 25);
    canvas->drawBitmap(redBM, 50, 50);
    canvas->restore();

    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    // Now replay the picture back on another canvas
    // and check a couple of its pixels.
    SkBitmap replayBM;
    make_bm(&replayBM, 100, 100, SK_ColorBLACK, false);
    SkCanvas replayCanvas(replayBM);
    picture->playback(&replayCanvas);
    replayCanvas.flush();

    // With the bug present, at (55, 55) we would get a fully opaque red
    // intead of a dark red.
    REPORTER_ASSERT(reporter, replayBM.getColor(30, 30) == 0xff000080);
    REPORTER_ASSERT(reporter, replayBM.getColor(55, 55) == 0xff800000);
}

struct CountingBBH : public SkBBoxHierarchy {
    mutable int searchCalls;
    SkRect rootBound;

    CountingBBH(const SkRect& bound) : searchCalls(0), rootBound(bound) {}

    void search(const SkRect& query, SkTDArray<int>* results) const override {
        this->searchCalls++;
    }

    void insert(const SkRect[], int) override {}
    virtual size_t bytesUsed() const override { return 0; }
    SkRect getRootBound() const override { return rootBound; }
};

class SpoonFedBBHFactory : public SkBBHFactory {
public:
    explicit SpoonFedBBHFactory(SkBBoxHierarchy* bbh) : fBBH(bbh) {}
    SkBBoxHierarchy* operator()(const SkRect&) const override {
        return SkRef(fBBH);
    }
private:
    SkBBoxHierarchy* fBBH;
};

// When the canvas clip covers the full picture, we don't need to call the BBH.
DEF_TEST(Picture_SkipBBH, r) {
    SkRect bound = SkRect::MakeWH(320, 240);
    CountingBBH bbh(bound);
    SpoonFedBBHFactory factory(&bbh);

    SkPictureRecorder recorder;
    SkCanvas* c = recorder.beginRecording(bound, &factory);
    // Record a few ops so we don't hit a small- or empty- picture optimization.
        c->drawRect(bound, SkPaint());
        c->drawRect(bound, SkPaint());
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    SkCanvas big(640, 480), small(300, 200);

    picture->playback(&big);
    REPORTER_ASSERT(r, bbh.searchCalls == 0);

    picture->playback(&small);
    REPORTER_ASSERT(r, bbh.searchCalls == 1);
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
            canvas->drawBitmap(mut, 0, 0);
            canvas->drawBitmap(immut, 800, 600);
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

DEF_TEST(MiniRecorderLeftHanging, r) {
    // Any shader or other ref-counted effect will do just fine here.
    SkPaint paint;
    paint.setShader(SkShader::MakeColorShader(SK_ColorRED));

    SkMiniRecorder rec;
    REPORTER_ASSERT(r, rec.drawRect(SkRect::MakeWH(20,30), paint));
    // Don't call rec.detachPicture().  Test succeeds by not asserting or leaking the shader.
}

DEF_TEST(Picture_preserveCullRect, r) {
    SkPictureRecorder recorder;

    SkCanvas* c = recorder.beginRecording(SkRect::MakeLTRB(1, 2, 3, 4));
    c->clear(SK_ColorCYAN);

    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    SkDynamicMemoryWStream wstream;
    picture->serialize(&wstream);

    std::unique_ptr<SkStream> rstream(wstream.detachAsStream());
    sk_sp<SkPicture> deserializedPicture(SkPicture::MakeFromStream(rstream.get()));

    REPORTER_ASSERT(r, deserializedPicture != nullptr);
    REPORTER_ASSERT(r, deserializedPicture->cullRect().left() == 1);
    REPORTER_ASSERT(r, deserializedPicture->cullRect().top() == 2);
    REPORTER_ASSERT(r, deserializedPicture->cullRect().right() == 3);
    REPORTER_ASSERT(r, deserializedPicture->cullRect().bottom() == 4);
}

#if SK_SUPPORT_GPU

DEF_TEST(PictureGpuAnalyzer, r) {
    SkPictureRecorder recorder;

    {
        SkCanvas* canvas = recorder.beginRecording(10, 10);
        SkPaint paint;
        SkScalar intervals [] = { 10, 20 };
        paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 25));

        for (int i = 0; i < 50; ++i) {
            canvas->drawRect(SkRect::MakeWH(10, 10), paint);
        }
    }
    sk_sp<SkPicture> vetoPicture(recorder.finishRecordingAsPicture());

    SkPictureGpuAnalyzer analyzer;
    REPORTER_ASSERT(r, analyzer.suitableForGpuRasterization());

    analyzer.analyzePicture(vetoPicture.get());
    REPORTER_ASSERT(r, !analyzer.suitableForGpuRasterization());

    analyzer.reset();
    REPORTER_ASSERT(r, analyzer.suitableForGpuRasterization());

    recorder.beginRecording(10, 10)->drawPicture(vetoPicture);
    sk_sp<SkPicture> nestedVetoPicture(recorder.finishRecordingAsPicture());

    analyzer.analyzePicture(nestedVetoPicture.get());
    REPORTER_ASSERT(r, !analyzer.suitableForGpuRasterization());

    analyzer.reset();

    const SkPath convexClip = make_convex_path();
    const SkPath concaveClip = make_concave_path();
    for (int i = 0; i < 50; ++i) {
        analyzer.analyzeClipPath(convexClip, kIntersect_SkClipOp, false);
        analyzer.analyzeClipPath(convexClip, kIntersect_SkClipOp, true);
        analyzer.analyzeClipPath(concaveClip, kIntersect_SkClipOp, false);
    }
    REPORTER_ASSERT(r, analyzer.suitableForGpuRasterization());

    for (int i = 0; i < 50; ++i) {
        analyzer.analyzeClipPath(concaveClip, kIntersect_SkClipOp, true);
    }
    REPORTER_ASSERT(r, !analyzer.suitableForGpuRasterization());
}

#endif // SK_SUPPORT_GPU

// If we record bounded ops into a picture with a big cull and calculate the
// bounds of those ops, we should trim down the picture cull to the ops' bounds.
// If we're not using an SkBBH, we shouldn't change it.
DEF_TEST(Picture_UpdatedCull_1, r) {
    // Testing 1 draw exercises SkMiniPicture.
    SkRTreeFactory factory;
    SkPictureRecorder recorder;

    auto canvas = recorder.beginRecording(SkRect::MakeLargest(), &factory);
    canvas->drawRect(SkRect::MakeWH(20,20), SkPaint{});
    auto pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->cullRect() == SkRect::MakeWH(20,20));

    canvas = recorder.beginRecording(SkRect::MakeLargest());
    canvas->drawRect(SkRect::MakeWH(20,20), SkPaint{});
    pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->cullRect() == SkRect::MakeLargest());
}
DEF_TEST(Picture_UpdatedCull_2, r) {
    // Testing >1 draw exercises SkBigPicture.
    SkRTreeFactory factory;
    SkPictureRecorder recorder;

    auto canvas = recorder.beginRecording(SkRect::MakeLargest(), &factory);
    canvas->drawRect(SkRect::MakeWH(20,20), SkPaint{});
    canvas->drawRect(SkRect::MakeWH(10,40), SkPaint{});
    auto pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->cullRect() == SkRect::MakeWH(20,40));

    canvas = recorder.beginRecording(SkRect::MakeLargest());
    canvas->drawRect(SkRect::MakeWH(20,20), SkPaint{});
    canvas->drawRect(SkRect::MakeWH(10,40), SkPaint{});
    pic = recorder.finishRecordingAsPicture();
    REPORTER_ASSERT(r, pic->cullRect() == SkRect::MakeLargest());
}
