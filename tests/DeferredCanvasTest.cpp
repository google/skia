/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../src/image/SkImagePriv.h"
#include "../src/image/SkSurface_Base.h"
#include "SkBitmap.h"
#include "SkBitmapProcShader.h"
#include "SkDeferredCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "Test.h"
#include "sk_tool_utils.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#else
class GrContextFactory;
#endif

static const int gWidth = 2;
static const int gHeight = 2;

static void create(SkBitmap* bm, SkColor color) {
    bm->allocN32Pixels(gWidth, gHeight);
    bm->eraseColor(color);
}

static SkSurface* createSurface(SkColor color) {
    SkSurface* surface = SkSurface::NewRasterN32Premul(gWidth, gHeight);
    surface->getCanvas()->clear(color);
    return surface;
}

static SkPMColor read_pixel(SkSurface* surface, int x, int y) {
    SkPMColor pixel = 0;
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeN32Premul(1, 1), &pixel, 4);
    SkCanvas canvas(bitmap);

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    surface->draw(&canvas, -SkIntToScalar(x), -SkIntToScalar(y), &paint);
    return pixel;
}

class MockSurface : public SkSurface_Base {
public:
    MockSurface(int width, int height) : SkSurface_Base(width, height, NULL) {
        clearCounts();
        fBitmap.allocN32Pixels(width, height);
    }

    SkCanvas* onNewCanvas() override {
        return SkNEW_ARGS(SkCanvas, (fBitmap));
    }

    SkSurface* onNewSurface(const SkImageInfo&) override {
        return NULL;
    }

    SkImage* onNewImageSnapshot(Budgeted) override {
        return SkNewImageFromRasterBitmap(fBitmap, &this->props());
    }

    void onCopyOnWrite(ContentChangeMode mode) override {
        if (mode == SkSurface::kDiscard_ContentChangeMode) {
            fCOWDiscardCount++;
        } else {
            fCOWRetainCount++;
        }
    }

    void onDiscard() override {
        fDiscardCount++;
    }

    void clearCounts() {
        fCOWDiscardCount = 0;
        fCOWRetainCount = 0;
        fDiscardCount = 0;
    }

    int fCOWDiscardCount;
    int fCOWRetainCount;
    int fDiscardCount;
    SkBitmap fBitmap;
};

static void TestDeferredCanvasWritePixelsToSurface(skiatest::Reporter* reporter) {
    SkAutoTUnref<MockSurface> surface(SkNEW_ARGS(MockSurface, (10, 10)));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    SkBitmap srcBitmap;
    srcBitmap.allocPixels(SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kUnpremul_SkAlphaType));
    srcBitmap.eraseColor(SK_ColorGREEN);
    // Tests below depend on this bitmap being recognized as opaque

    // Preliminary sanity check: no copy on write if no active snapshot
    // Discard notification happens on SkSurface::onDiscard, since no
    // active snapshot.
    surface->clearCounts();
    canvas->clear(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 1 == surface->fDiscardCount);

    // Case 1: Discard notification happens upon flushing
    // with an Image attached.
    surface->clearCounts();
    SkAutoTUnref<SkImage> image1(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->clear(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 1 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    // Case 2: Opaque writePixels
    surface->clearCounts();
    SkAutoTUnref<SkImage> image2(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    // Case 3: writePixels that partially covers the canvas
    surface->clearCounts();
    SkAutoTUnref<SkImage> image3(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    // Case 4: unpremultiplied opaque writePixels that entirely
    // covers the canvas
    surface->clearCounts();
    SkAutoTUnref<SkImage> image4(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 0, 0);
    REPORTER_ASSERT(reporter, 1 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    // Case 5: unpremultiplied opaque writePixels that partially
    // covers the canvas
    surface->clearCounts();
    SkAutoTUnref<SkImage> image5(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 5, 0);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 1 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    // Case 6: unpremultiplied opaque writePixels that entirely
    // covers the canvas, preceded by clear
    surface->clearCounts();
    SkAutoTUnref<SkImage> image6(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->clear(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 0, 0);
    REPORTER_ASSERT(reporter, 1 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    // Case 7: unpremultiplied opaque writePixels that partially
    // covers the canvas, preceeded by a clear
    surface->clearCounts();
    SkAutoTUnref<SkImage> image7(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->clear(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 5, 0);
    REPORTER_ASSERT(reporter, 1 == surface->fCOWDiscardCount); // because of the clear
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    // Case 8: unpremultiplied opaque writePixels that partially
    // covers the canvas, preceeded by a drawREct that partially
    // covers the canvas
    surface->clearCounts();
    SkAutoTUnref<SkImage> image8(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    SkPaint paint;
    canvas->drawRect(SkRect::MakeLTRB(0, 0, 5, 5), paint);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 5, 0);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 1 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fCOWDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fCOWRetainCount);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
}

static void TestDeferredCanvasFlush(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(createSurface(0xFFFFFFFF));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    canvas->clear(0x00000000);

    // verify that clear was deferred
    REPORTER_ASSERT(reporter, 0xFFFFFFFF == read_pixel(surface, 0, 0));

    canvas->flush();

    // verify that clear was executed
    REPORTER_ASSERT(reporter, 0 == read_pixel(surface, 0, 0));
}

static void TestDeferredCanvasFreshFrame(skiatest::Reporter* reporter) {
    SkRect fullRect;
    fullRect.setXYWH(SkIntToScalar(0), SkIntToScalar(0), SkIntToScalar(gWidth),
        SkIntToScalar(gHeight));
    SkRect partialRect;
    partialRect.setXYWH(SkIntToScalar(0), SkIntToScalar(0),
        SkIntToScalar(1), SkIntToScalar(1));

    SkAutoTUnref<SkSurface> surface(createSurface(0xFFFFFFFF));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    // verify that frame is intially fresh
    REPORTER_ASSERT(reporter, canvas->isFreshFrame());
    // no clearing op since last call to isFreshFrame -> not fresh
    REPORTER_ASSERT(reporter, !canvas->isFreshFrame());

    // Verify that clear triggers a fresh frame
    canvas->clear(0x00000000);
    REPORTER_ASSERT(reporter, canvas->isFreshFrame());

    // Verify that clear with saved state triggers a fresh frame
    canvas->save();
    canvas->clear(0x00000000);
    canvas->restore();
    REPORTER_ASSERT(reporter, canvas->isFreshFrame());

    // Verify that clear within a layer does NOT trigger a fresh frame
    canvas->saveLayer(NULL, NULL);
    canvas->clear(0x00000000);
    canvas->restore();
    REPORTER_ASSERT(reporter, !canvas->isFreshFrame());

    // Verify that full frame rects with different forms of opaque paint
    // trigger frames to be marked as fresh
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, canvas->isFreshFrame());
    }
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        paint.setXfermodeMode(SkXfermode::kSrcIn_Mode);
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        SkBitmap bmp;
        create(&bmp, 0xFFFFFFFF);
        bmp.setAlphaType(kOpaque_SkAlphaType);
        SkShader* shader = SkShader::CreateBitmapShader(bmp,
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
        paint.setShader(shader)->unref();
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, canvas->isFreshFrame());
    }

    // Verify that full frame rects with different forms of non-opaque paint
    // do not trigger frames to be marked as fresh
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(254);
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        // Defining a cone that partially overlaps the canvas
        const SkPoint pt1 = SkPoint::Make(SkIntToScalar(0), SkIntToScalar(0));
        const SkScalar r1 = SkIntToScalar(1);
        const SkPoint pt2 = SkPoint::Make(SkIntToScalar(10), SkIntToScalar(0));
        const SkScalar r2 = SkIntToScalar(5);
        const SkColor colors[2] = {SK_ColorWHITE, SK_ColorWHITE};
        const SkScalar pos[2] = {0, SK_Scalar1};
        SkShader* shader = SkGradientShader::CreateTwoPointConical(
            pt1, r1, pt2, r2, colors, pos, 2, SkShader::kClamp_TileMode);
        paint.setShader(shader)->unref();
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        SkBitmap bmp;
        create(&bmp, 0xFFFFFFFF);
        bmp.setAlphaType(kPremul_SkAlphaType);
        SkShader* shader = SkShader::CreateBitmapShader(bmp,
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
        paint.setShader(shader)->unref();
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }

    // Verify that incomplete coverage does not trigger a fresh frame
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        canvas->drawRect(partialRect, paint);
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }

    // Verify that incomplete coverage due to clipping does not trigger a fresh
    // frame
    {
        canvas->save();
        canvas->clipRect(partialRect, SkRegion::kIntersect_Op, false);
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        canvas->drawRect(fullRect, paint);
        canvas->restore();
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }
    {
        canvas->save();
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        SkPath path;
        path.addCircle(SkIntToScalar(0), SkIntToScalar(0), SkIntToScalar(2));
        canvas->clipPath(path, SkRegion::kIntersect_Op, false);
        canvas->drawRect(fullRect, paint);
        canvas->restore();
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }

    // Verify that stroked rect does not trigger a fresh frame
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAlpha(255);
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }

    // Verify kSrcMode triggers a fresh frame even with transparent color
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(100);
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, canvas->isFreshFrame());
    }
}

class NotificationCounter : public SkDeferredCanvas::NotificationClient {
public:
    NotificationCounter() {
        fPrepareForDrawCount = fStorageAllocatedChangedCount =
            fFlushedDrawCommandsCount = fSkippedPendingDrawCommandsCount = 0;
    }

    void prepareForDraw() override {
        fPrepareForDrawCount++;
    }
    void storageAllocatedForRecordingChanged(size_t) override {
        fStorageAllocatedChangedCount++;
    }
    void flushedDrawCommands() override {
        fFlushedDrawCommandsCount++;
    }
    void skippedPendingDrawCommands() override {
        fSkippedPendingDrawCommandsCount++;
    }

    int fPrepareForDrawCount;
    int fStorageAllocatedChangedCount;
    int fFlushedDrawCommandsCount;
    int fSkippedPendingDrawCommandsCount;

private:
    typedef SkDeferredCanvas::NotificationClient INHERITED;
};

// Verifies that the deferred canvas triggers a flush when its memory
// limit is exceeded
static void TestDeferredCanvasMemoryLimit(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(100, 100));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    NotificationCounter notificationCounter;
    canvas->setNotificationClient(&notificationCounter);

    canvas->setMaxRecordingStorage(160000);

    SkBitmap sourceImage;
    // 100 by 100 image, takes 40,000 bytes in memory
    sourceImage.allocN32Pixels(100, 100);
    sourceImage.eraseColor(SK_ColorGREEN);

    for (int i = 0; i < 5; i++) {
        sourceImage.notifyPixelsChanged(); // to force re-serialization
        canvas->drawBitmap(sourceImage, 0, 0, NULL);
    }

    REPORTER_ASSERT(reporter, 1 == notificationCounter.fFlushedDrawCommandsCount);
}

static void TestDeferredCanvasSilentFlush(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(createSurface(0));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    NotificationCounter notificationCounter;
    canvas->setNotificationClient(&notificationCounter);

    canvas->silentFlush(); // will skip the initial clear that was recorded in createSurface

    REPORTER_ASSERT(reporter, 0 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fSkippedPendingDrawCommandsCount);
}

static void TestDeferredCanvasBitmapCaching(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(100, 100));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    NotificationCounter notificationCounter;
    canvas->setNotificationClient(&notificationCounter);

    const int imageCount = 2;
    SkBitmap sourceImages[imageCount];
    for (int i = 0; i < imageCount; i++) {
        sourceImages[i].allocN32Pixels(100, 100);
        sourceImages[i].eraseColor(SK_ColorGREEN);
    }

    size_t bitmapSize = sourceImages[0].getSize();

    canvas->drawBitmap(sourceImages[0], 0, 0, NULL);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fStorageAllocatedChangedCount);
    // stored bitmap + drawBitmap command
    REPORTER_ASSERT(reporter, canvas->storageAllocatedForRecording() > bitmapSize);

    // verify that nothing can be freed at this point
    REPORTER_ASSERT(reporter, 0 == canvas->freeMemoryIfPossible(~0U));

    // verify that flush leaves image in cache
    REPORTER_ASSERT(reporter, 0 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, 0 == notificationCounter.fPrepareForDrawCount);
    canvas->flush();
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fPrepareForDrawCount);
    REPORTER_ASSERT(reporter, canvas->storageAllocatedForRecording() >= bitmapSize);

    // verify that after a flush, cached image can be freed
    REPORTER_ASSERT(reporter, canvas->freeMemoryIfPossible(~0U) >= bitmapSize);

    // Verify that caching works for avoiding multiple copies of the same bitmap
    canvas->drawBitmap(sourceImages[0], 0, 0, NULL);
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fStorageAllocatedChangedCount);
    canvas->drawBitmap(sourceImages[0], 0, 0, NULL);
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fStorageAllocatedChangedCount);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, canvas->storageAllocatedForRecording() < 2 * bitmapSize);

    // Verify partial eviction based on bytesToFree
    canvas->drawBitmap(sourceImages[1], 0, 0, NULL);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fFlushedDrawCommandsCount);
    canvas->flush();
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, canvas->storageAllocatedForRecording() > 2 * bitmapSize);
    size_t bytesFreed = canvas->freeMemoryIfPossible(1);
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter,  bytesFreed >= bitmapSize);
    REPORTER_ASSERT(reporter,  bytesFreed < 2*bitmapSize);

    // Verifiy that partial purge works, image zero is in cache but not reffed by
    // a pending draw, while image 1 is locked-in.
    canvas->freeMemoryIfPossible(~0U);
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fFlushedDrawCommandsCount);
    canvas->drawBitmap(sourceImages[0], 0, 0, NULL);
    canvas->flush();
    canvas->drawBitmap(sourceImages[1], 0, 0, NULL);
    bytesFreed = canvas->freeMemoryIfPossible(~0U);
    // only one bitmap should have been freed.
    REPORTER_ASSERT(reporter,  bytesFreed >= bitmapSize);
    REPORTER_ASSERT(reporter,  bytesFreed < 2*bitmapSize);
    // Clear for next test
    canvas->flush();
    canvas->freeMemoryIfPossible(~0U);
    REPORTER_ASSERT(reporter, canvas->storageAllocatedForRecording() < bitmapSize);

    // Verify the image cache is sensitive to genID bumps
    canvas->drawBitmap(sourceImages[1], 0, 0, NULL);
    sourceImages[1].notifyPixelsChanged();
    canvas->drawBitmap(sourceImages[1], 0, 0, NULL);
    REPORTER_ASSERT(reporter, canvas->storageAllocatedForRecording() > 2*bitmapSize);

    // Verify that nothing in this test caused commands to be skipped
    REPORTER_ASSERT(reporter, 0 == notificationCounter.fSkippedPendingDrawCommandsCount);
}

static void TestDeferredCanvasSkip(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(100, 100));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    NotificationCounter notificationCounter;
    canvas->setNotificationClient(&notificationCounter);
    canvas->clear(0x0);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fSkippedPendingDrawCommandsCount);
    REPORTER_ASSERT(reporter, 0 == notificationCounter.fFlushedDrawCommandsCount);
    canvas->flush();
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fSkippedPendingDrawCommandsCount);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fFlushedDrawCommandsCount);

}

static void TestDeferredCanvasBitmapShaderNoLeak(skiatest::Reporter* reporter) {
    // This is a regression test for crbug.com/155875
    // This test covers a code path that inserts bitmaps into the bitmap heap through the
    // flattening of SkBitmapProcShaders. The refcount in the bitmap heap is maintained through
    // the flattening and unflattening of the shader.
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(100, 100));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));
    // test will fail if nbIterations is not in sync with
    // BITMAPS_TO_KEEP in SkGPipeWrite.cpp
    const int nbIterations = 5;
    size_t bytesAllocated = 0;
    for(int pass = 0; pass < 2; ++pass) {
        for(int i = 0; i < nbIterations; ++i) {
            SkPaint paint;
            SkBitmap paintPattern;
            paintPattern.allocN32Pixels(10, 10);
            paintPattern.eraseColor(SK_ColorGREEN);
            paint.setShader(SkNEW_ARGS(SkBitmapProcShader,
                (paintPattern, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode)))->unref();
            canvas->drawPaint(paint);
            canvas->flush();

            // In the first pass, memory allocation should be monotonically increasing as
            // the bitmap heap slots fill up.  In the second pass memory allocation should be
            // stable as bitmap heap slots get recycled.
            size_t newBytesAllocated = canvas->storageAllocatedForRecording();
            if (pass == 0) {
                REPORTER_ASSERT(reporter, newBytesAllocated > bytesAllocated);
                bytesAllocated = newBytesAllocated;
            } else {
                REPORTER_ASSERT(reporter, newBytesAllocated == bytesAllocated);
            }
        }
    }
    // All cached resources should be evictable since last canvas call was flush()
    canvas->freeMemoryIfPossible(~0U);
    REPORTER_ASSERT(reporter, 0 == canvas->storageAllocatedForRecording());
}

static void TestDeferredCanvasBitmapSizeThreshold(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(100, 100));

    SkBitmap sourceImage;
    // 100 by 100 image, takes 40,000 bytes in memory
    sourceImage.allocN32Pixels(100, 100);
    sourceImage.eraseColor(SK_ColorGREEN);

    // 1 under : should not store the image
    {
        SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));
        canvas->setBitmapSizeThreshold(39999);
        canvas->drawBitmap(sourceImage, 0, 0, NULL);
        size_t newBytesAllocated = canvas->storageAllocatedForRecording();
        REPORTER_ASSERT(reporter, newBytesAllocated == 0);
    }

    // exact value : should store the image
    {
        SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));
        canvas->setBitmapSizeThreshold(40000);
        canvas->drawBitmap(sourceImage, 0, 0, NULL);
        size_t newBytesAllocated = canvas->storageAllocatedForRecording();
        REPORTER_ASSERT(reporter, newBytesAllocated > 0);
    }

    // 1 over : should still store the image
    {
        SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));
        canvas->setBitmapSizeThreshold(40001);
        canvas->drawBitmap(sourceImage, 0, 0, NULL);
        size_t newBytesAllocated = canvas->storageAllocatedForRecording();
        REPORTER_ASSERT(reporter, newBytesAllocated > 0);
    }
}

static void TestDeferredCanvasImageFreeAfterFlush(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(100, 100));
    SkAutoTUnref<SkSurface> sourceSurface(SkSurface::NewRasterN32Premul(100, 100));
    SkAutoTUnref<SkImage> sourceImage(sourceSurface->newImageSnapshot());
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    canvas->drawImage(sourceImage, 0, 0, NULL);

    size_t newBytesAllocated = canvas->storageAllocatedForRecording();
    REPORTER_ASSERT(reporter, newBytesAllocated > 0);

    canvas->flush();
    
    newBytesAllocated = canvas->storageAllocatedForRecording();
    REPORTER_ASSERT(reporter, newBytesAllocated == 0);
}

typedef const void* PixelPtr;
// Returns an opaque pointer which, either points to a GrTexture or RAM pixel
// buffer. Used to test pointer equality do determine whether a surface points
// to the same pixel data storage as before.
static PixelPtr get_surface_ptr(SkSurface* surface, bool useGpu) {
#if SK_SUPPORT_GPU
    if (useGpu) {
        return surface->getCanvas()->internal_private_accessTopLayerRenderTarget()->asTexture();
    } else
#endif
    {
        return surface->peekPixels(NULL, NULL);
    }
}

static void TestDeferredCanvasSurface(skiatest::Reporter* reporter, GrContextFactory* factory) {
    SkImageInfo imageSpec = SkImageInfo::MakeN32Premul(10, 10);
    bool useGpu = SkToBool(factory);
    int cnt;
#if SK_SUPPORT_GPU
    if (useGpu) {
        cnt = GrContextFactory::kGLContextTypeCnt;
    } else {
        cnt = 1;
    }
#else
    SkASSERT(!useGpu);
    cnt = 1;
#endif
    for (int i = 0; i < cnt; ++i) {
        SkSurface* surface;
#if SK_SUPPORT_GPU
        if (useGpu) {
            GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;
            if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
                continue;
            }
            GrContext* context = factory->get(glCtxType);
            if (NULL == context) {
                return;
            }

            surface =
                SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, imageSpec, 0, NULL);
        } else
#endif
        {
           surface = SkSurface::NewRaster(imageSpec);
        }
        SkASSERT(surface);
        SkAutoTUnref<SkSurface> aur(surface);
        SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface));

        SkImage* image1 = canvas->newImageSnapshot();
        SkAutoTUnref<SkImage> aur_i1(image1);
        PixelPtr pixels1 = get_surface_ptr(surface, useGpu);
        // The following clear would normally trigger a copy on write, but
        // it won't because rendering is deferred.
        canvas->clear(SK_ColorBLACK);
        // Obtaining a snapshot directly from the surface (as opposed to the
        // SkDeferredCanvas) will not trigger a flush of deferred draw operations
        // and will therefore return the same image as the previous snapshot.
        SkImage* image2 = surface->newImageSnapshot();
        SkAutoTUnref<SkImage> aur_i2(image2);
        // Images identical because of deferral
        REPORTER_ASSERT(reporter, image1->uniqueID() == image2->uniqueID());
        // Now we obtain a snpshot via the deferred canvas, which triggers a flush.
        // Because there is a pending clear, this will generate a different image.
        SkImage* image3 = canvas->newImageSnapshot();
        SkAutoTUnref<SkImage> aur_i3(image3);
        REPORTER_ASSERT(reporter, image1->uniqueID() != image3->uniqueID());
        // Verify that backing store is now a different buffer because of copy on
        // write
        PixelPtr pixels2 = get_surface_ptr(surface, useGpu);
        REPORTER_ASSERT(reporter, pixels1 != pixels2);
        // Verify copy-on write with a draw operation that gets deferred by
        // the in order draw buffer.
        SkPaint paint;
        canvas->drawPaint(paint);
        SkImage* image4 = canvas->newImageSnapshot();  // implicit flush
        SkAutoTUnref<SkImage> aur_i4(image4);
        REPORTER_ASSERT(reporter, image4->uniqueID() != image3->uniqueID());
        PixelPtr pixels3 = get_surface_ptr(surface, useGpu);
        REPORTER_ASSERT(reporter, pixels2 != pixels3);
        // Verify that a direct canvas flush with a pending draw does not trigger
        // a copy on write when the surface is not sharing its buffer with an
        // SkImage.
        canvas->clear(SK_ColorWHITE);
        canvas->flush();
        PixelPtr pixels4 = get_surface_ptr(surface, useGpu);
        canvas->drawPaint(paint);
        canvas->flush();
        PixelPtr pixels5 = get_surface_ptr(surface, useGpu);
        REPORTER_ASSERT(reporter, pixels4 == pixels5);
    }
}

static void TestDeferredCanvasSetSurface(skiatest::Reporter* reporter, GrContextFactory* factory) {
    SkImageInfo imageSpec = SkImageInfo::MakeN32Premul(10, 10);
    SkSurface* surface;
    SkSurface* alternateSurface;
    bool useGpu = SkToBool(factory);
    int cnt;
#if SK_SUPPORT_GPU
    if (useGpu) {
        cnt = GrContextFactory::kGLContextTypeCnt;
    } else {
        cnt = 1;
    }
#else
    SkASSERT(!useGpu);
    cnt = 1;
#endif

    for (int i = 0; i < cnt; ++i) {
#if SK_SUPPORT_GPU
        if (useGpu) {
            GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;
            if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
                continue;
            }
            GrContext* context = factory->get(glCtxType);
            if (NULL == context) {
                continue;
            }
            surface =
                SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, imageSpec, 0, NULL);
            alternateSurface =
                SkSurface::NewRenderTarget(context, SkSurface::kNo_Budgeted, imageSpec, 0, NULL);
        } else
#endif
        {
            surface = SkSurface::NewRaster(imageSpec);
            alternateSurface = SkSurface::NewRaster(imageSpec);
        }
        SkASSERT(surface);
        SkASSERT(alternateSurface);
        SkAutoTUnref<SkSurface> aur1(surface);
        SkAutoTUnref<SkSurface> aur2(alternateSurface);
        PixelPtr pixels1 = get_surface_ptr(surface, useGpu);
        PixelPtr pixels2 = get_surface_ptr(alternateSurface, useGpu);
        SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface));
        SkAutoTUnref<SkImage> image1(canvas->newImageSnapshot());
        canvas->setSurface(alternateSurface);
        SkAutoTUnref<SkImage> image2(canvas->newImageSnapshot());
        REPORTER_ASSERT(reporter, image1->uniqueID() != image2->uniqueID());
        // Verify that none of the above operations triggered a surface copy on write.
        REPORTER_ASSERT(reporter, get_surface_ptr(surface, useGpu) == pixels1);
        REPORTER_ASSERT(reporter, get_surface_ptr(alternateSurface, useGpu) == pixels2);
        // Verify that a flushed draw command will trigger a copy on write on alternateSurface.
        canvas->clear(SK_ColorWHITE);
        canvas->flush();
        REPORTER_ASSERT(reporter, get_surface_ptr(surface, useGpu) == pixels1);
        REPORTER_ASSERT(reporter, get_surface_ptr(alternateSurface, useGpu) != pixels2);
    }
}

static void TestDeferredCanvasCreateCompatibleDevice(skiatest::Reporter* reporter) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRasterN32Premul(100, 100));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    NotificationCounter notificationCounter;
    canvas->setNotificationClient(&notificationCounter);

    SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);
    SkAutoTUnref<SkSurface> secondarySurface(canvas->newSurface(info));

    SkRect rect = SkRect::MakeWH(5, 5);
    SkPaint paint;
    // After spawning a compatible canvas:
    // 1) Verify that secondary canvas is usable and does not report to the notification client.
    surface->getCanvas()->drawRect(rect, paint);
    REPORTER_ASSERT(reporter, notificationCounter.fStorageAllocatedChangedCount == 0);
    // 2) Verify that original canvas is usable and still reports to the notification client.
    canvas->drawRect(rect, paint);
    REPORTER_ASSERT(reporter, notificationCounter.fStorageAllocatedChangedCount == 1);
}

static void TestDeferredCanvasGetCanvasSize(skiatest::Reporter* reporter) {
    SkRect rect;
    rect.setXYWH(SkIntToScalar(0), SkIntToScalar(0), SkIntToScalar(gWidth), SkIntToScalar(gHeight));
    SkRect clip;
    clip.setXYWH(SkIntToScalar(0), SkIntToScalar(0), SkIntToScalar(1), SkIntToScalar(1));

    SkPaint paint;
    SkISize size = SkISize::Make(gWidth, gHeight);

    SkAutoTUnref<SkSurface> surface(createSurface(0xFFFFFFFF));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));
    SkSurface* newSurface = SkSurface::NewRasterN32Premul(4, 4);
    SkAutoTUnref<SkSurface> aur(newSurface);

    for (int i = 0; i < 2; ++i) {
        if (i == 1) {
            canvas->setSurface(newSurface);
            size = SkISize::Make(4, 4);
        }

        // verify that canvas size is correctly initialized or set
        REPORTER_ASSERT(reporter, size == canvas->getCanvasSize());

        // Verify that clear, clip and draw the canvas will not change its size
        canvas->clear(0x00000000);
        canvas->clipRect(clip, SkRegion::kIntersect_Op, false);
        canvas->drawRect(rect, paint);
        REPORTER_ASSERT(reporter, size == canvas->getCanvasSize());

        // Verify that flush the canvas will not change its size
        canvas->flush();
        REPORTER_ASSERT(reporter, size == canvas->getCanvasSize());

        // Verify that clear canvas with saved state will not change its size
        canvas->save();
        canvas->clear(0xFFFFFFFF);
        REPORTER_ASSERT(reporter, size == canvas->getCanvasSize());

        // Verify that restore canvas state will not change its size
        canvas->restore();
        REPORTER_ASSERT(reporter, size == canvas->getCanvasSize());

        // Verify that clear within a layer will not change canvas size
        canvas->saveLayer(&clip, &paint);
        canvas->clear(0x00000000);
        REPORTER_ASSERT(reporter, size == canvas->getCanvasSize());

        // Verify that restore from a layer will not change canvas size
        canvas->restore();
        REPORTER_ASSERT(reporter, size == canvas->getCanvasSize());
    }
}

DEF_TEST(DeferredCanvas_CPU, reporter) {
    TestDeferredCanvasFlush(reporter);
    TestDeferredCanvasSilentFlush(reporter);
    TestDeferredCanvasFreshFrame(reporter);
    TestDeferredCanvasMemoryLimit(reporter);
    TestDeferredCanvasBitmapCaching(reporter);
    TestDeferredCanvasSkip(reporter);
    TestDeferredCanvasBitmapShaderNoLeak(reporter);
    TestDeferredCanvasBitmapSizeThreshold(reporter);
    TestDeferredCanvasImageFreeAfterFlush(reporter);    
    TestDeferredCanvasCreateCompatibleDevice(reporter);
    TestDeferredCanvasWritePixelsToSurface(reporter);
    TestDeferredCanvasGetCanvasSize(reporter);
    TestDeferredCanvasSurface(reporter, NULL);
    TestDeferredCanvasSetSurface(reporter, NULL);
}

DEF_GPUTEST(DeferredCanvas_GPU, reporter, factory) {
    if (factory != NULL) {
        TestDeferredCanvasSurface(reporter, factory);
        TestDeferredCanvasSetSurface(reporter, factory);
    }
}
