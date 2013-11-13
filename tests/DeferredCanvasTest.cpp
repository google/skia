
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkBitmap.h"
#include "SkBitmapDevice.h"
#include "SkBitmapProcShader.h"
#include "SkDeferredCanvas.h"
#include "SkGradientShader.h"
#include "SkShader.h"
#include "../src/image/SkSurface_Base.h"
#include "../src/image/SkImagePriv.h"
#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#else
class GrContextFactory;
#endif

static const int gWidth = 2;
static const int gHeight = 2;

static void create(SkBitmap* bm, SkBitmap::Config config, SkColor color) {
    bm->setConfig(config, gWidth, gHeight);
    bm->allocPixels();
    bm->eraseColor(color);
}

static void TestDeferredCanvasBitmapAccess(skiatest::Reporter* reporter) {
    SkBitmap store;

    create(&store, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkBitmapDevice device(store);
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));

    canvas->clear(0x00000000);

    SkAutoLockPixels alp(store);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0xFFFFFFFF); //verify that clear was deferred
    SkBitmap accessed = canvas->getDevice()->accessBitmap(false);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0x00000000); //verify that clear was executed
    REPORTER_ASSERT(reporter, accessed.pixelRef() == store.pixelRef());
}

class MockSurface : public SkSurface_Base {
public:
    MockSurface(int width, int height) : SkSurface_Base(width, height) {
        clearCounts();
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        fBitmap.allocPixels();
    }

    virtual SkCanvas* onNewCanvas() SK_OVERRIDE {
        return SkNEW_ARGS(SkCanvas, (fBitmap));
    }

    virtual SkSurface* onNewSurface(const SkImageInfo&) SK_OVERRIDE {
        return NULL;
    }

    virtual SkImage* onNewImageSnapshot() SK_OVERRIDE {
        return SkNewImageFromBitmap(fBitmap, true);
    }

    virtual void onCopyOnWrite(ContentChangeMode mode) SK_OVERRIDE {
        if (mode == SkSurface::kDiscard_ContentChangeMode) {
            fDiscardCount++;
        } else {
            fRetainCount++;
        }
    }

    void clearCounts() {
        fDiscardCount = 0;
        fRetainCount = 0;
    }

    int fDiscardCount, fRetainCount;
    SkBitmap fBitmap;
};

static void TestDeferredCanvasWritePixelsToSurface(skiatest::Reporter* reporter) {
    SkAutoTUnref<MockSurface> surface(SkNEW_ARGS(MockSurface, (10, 10)));
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface.get()));

    SkBitmap srcBitmap;
    srcBitmap.setConfig(SkBitmap::kARGB_8888_Config, 10, 10);
    srcBitmap.allocPixels();
    srcBitmap.eraseColor(SK_ColorGREEN);
    // Tests below depend on this bitmap being recognized as opaque

    // Preliminary sanity check: no copy on write if no active snapshot
    surface->clearCounts();
    canvas->clear(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    // Case 1: Discard notification happens upon flushing
    // with an Image attached.
    surface->clearCounts();
    SkAutoTUnref<SkImage> image1(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->clear(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 1 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    // Case 2: Opaque writePixels
    surface->clearCounts();
    SkAutoTUnref<SkImage> image2(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 0, 0);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 1 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    // Case 3: writePixels that partially covers the canvas
    surface->clearCounts();
    SkAutoTUnref<SkImage> image3(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 5, 0);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 1 == surface->fRetainCount);

    // Case 4: unpremultiplied opaque writePixels that entirely
    // covers the canvas
    surface->clearCounts();
    SkAutoTUnref<SkImage> image4(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 0, 0, SkCanvas::kRGBA_Unpremul_Config8888);
    REPORTER_ASSERT(reporter, 1 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    // Case 5: unpremultiplied opaque writePixels that partially
    // covers the canvas
    surface->clearCounts();
    SkAutoTUnref<SkImage> image5(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 5, 0, SkCanvas::kRGBA_Unpremul_Config8888);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 1 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    // Case 6: unpremultiplied opaque writePixels that entirely
    // covers the canvas, preceded by clear
    surface->clearCounts();
    SkAutoTUnref<SkImage> image6(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->clear(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 0, 0, SkCanvas::kRGBA_Unpremul_Config8888);
    REPORTER_ASSERT(reporter, 1 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    // Case 7: unpremultiplied opaque writePixels that partially
    // covers the canvas, preceeded by a clear
    surface->clearCounts();
    SkAutoTUnref<SkImage> image7(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->clear(SK_ColorWHITE);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 5, 0, SkCanvas::kRGBA_Unpremul_Config8888);
    REPORTER_ASSERT(reporter, 1 == surface->fDiscardCount); // because of the clear
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    // Case 8: unpremultiplied opaque writePixels that partially
    // covers the canvas, preceeded by a drawREct that partially
    // covers the canvas
    surface->clearCounts();
    SkAutoTUnref<SkImage> image8(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    SkPaint paint;
    canvas->drawRect(SkRect::MakeLTRB(0, 0, 5, 5), paint);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);

    surface->clearCounts();
    canvas->writePixels(srcBitmap, 5, 0, SkCanvas::kRGBA_Unpremul_Config8888);
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 1 == surface->fRetainCount);

    surface->clearCounts();
    canvas->flush();
    REPORTER_ASSERT(reporter, 0 == surface->fDiscardCount);
    REPORTER_ASSERT(reporter, 0 == surface->fRetainCount);
}

static void TestDeferredCanvasFlush(skiatest::Reporter* reporter) {
    SkBitmap store;

    create(&store, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkBitmapDevice device(store);
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));

    canvas->clear(0x00000000);

    SkAutoLockPixels alp(store);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0xFFFFFFFF); //verify that clear was deferred
    canvas->flush();
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0x00000000); //verify that clear was executed
}

static void TestDeferredCanvasFreshFrame(skiatest::Reporter* reporter) {
    SkBitmap store;
    SkRect fullRect;
    fullRect.setXYWH(SkIntToScalar(0), SkIntToScalar(0), SkIntToScalar(gWidth),
        SkIntToScalar(gHeight));
    SkRect partialRect;
    partialRect.setXYWH(SkIntToScalar(0), SkIntToScalar(0),
        SkIntToScalar(1), SkIntToScalar(1));
    create(&store, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkBitmapDevice device(store);
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));

    // verify that frame is intially fresh
    REPORTER_ASSERT(reporter, canvas->isFreshFrame());
    // no clearing op since last call to isFreshFrame -> not fresh
    REPORTER_ASSERT(reporter, !canvas->isFreshFrame());

    // Verify that clear triggers a fresh frame
    canvas->clear(0x00000000);
    REPORTER_ASSERT(reporter, canvas->isFreshFrame());

    // Verify that clear with saved state triggers a fresh frame
    canvas->save(SkCanvas::kMatrixClip_SaveFlag);
    canvas->clear(0x00000000);
    canvas->restore();
    REPORTER_ASSERT(reporter, canvas->isFreshFrame());

    // Verify that clear within a layer does NOT trigger a fresh frame
    canvas->saveLayer(NULL, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);
    canvas->clear(0x00000000);
    canvas->restore();
    REPORTER_ASSERT(reporter, !canvas->isFreshFrame());

    // Verify that a clear with clipping triggers a fresh frame
    // (clear is not affected by clipping)
    canvas->save(SkCanvas::kMatrixClip_SaveFlag);
    canvas->clipRect(partialRect, SkRegion::kIntersect_Op, false);
    canvas->clear(0x00000000);
    canvas->restore();
    REPORTER_ASSERT(reporter, canvas->isFreshFrame());

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
        create(&bmp, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
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
            pt1, r1, pt2, r2, colors, pos, 2, SkShader::kClamp_TileMode, NULL);
        paint.setShader(shader)->unref();
        canvas->drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        SkBitmap bmp;
        create(&bmp, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
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
        canvas->save(SkCanvas::kMatrixClip_SaveFlag);
        canvas->clipRect(partialRect, SkRegion::kIntersect_Op, false);
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        canvas->drawRect(fullRect, paint);
        canvas->restore();
        REPORTER_ASSERT(reporter, !canvas->isFreshFrame());
    }
    {
        canvas->save(SkCanvas::kMatrixClip_SaveFlag);
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

class MockDevice : public SkBitmapDevice {
public:
    MockDevice(const SkBitmap& bm) : SkBitmapDevice(bm) {
        fDrawBitmapCallCount = 0;
    }
    virtual void drawBitmap(const SkDraw&, const SkBitmap&,
                            const SkMatrix&, const SkPaint&) SK_OVERRIDE {
        fDrawBitmapCallCount++;
    }

    int fDrawBitmapCallCount;
};

// Verifies that the deferred canvas triggers a flush when its memory
// limit is exceeded
static void TestDeferredCanvasMemoryLimit(skiatest::Reporter* reporter) {
    SkBitmap store;
    store.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    store.allocPixels();
    MockDevice mockDevice(store);
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&mockDevice));
    canvas->setMaxRecordingStorage(160000);

    SkBitmap sourceImage;
    // 100 by 100 image, takes 40,000 bytes in memory
    sourceImage.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    sourceImage.allocPixels();

    for (int i = 0; i < 5; i++) {
        sourceImage.notifyPixelsChanged(); // to force re-serialization
        canvas->drawBitmap(sourceImage, 0, 0, NULL);
    }

    REPORTER_ASSERT(reporter, mockDevice.fDrawBitmapCallCount == 4);
}

class NotificationCounter : public SkDeferredCanvas::NotificationClient {
public:
    NotificationCounter() {
        fPrepareForDrawCount = fStorageAllocatedChangedCount =
            fFlushedDrawCommandsCount = fSkippedPendingDrawCommandsCount = 0;
    }

    virtual void prepareForDraw() SK_OVERRIDE {
        fPrepareForDrawCount++;
    }
    virtual void storageAllocatedForRecordingChanged(size_t) SK_OVERRIDE {
        fStorageAllocatedChangedCount++;
    }
    virtual void flushedDrawCommands() SK_OVERRIDE {
        fFlushedDrawCommandsCount++;
    }
    virtual void skippedPendingDrawCommands() SK_OVERRIDE {
        fSkippedPendingDrawCommandsCount++;
    }

    int fPrepareForDrawCount;
    int fStorageAllocatedChangedCount;
    int fFlushedDrawCommandsCount;
    int fSkippedPendingDrawCommandsCount;

private:
    typedef SkDeferredCanvas::NotificationClient INHERITED;
};

static void TestDeferredCanvasBitmapCaching(skiatest::Reporter* reporter) {
    SkBitmap store;
    store.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    store.allocPixels();
    SkBitmapDevice device(store);
    NotificationCounter notificationCounter;
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));
    canvas->setNotificationClient(&notificationCounter);

    const int imageCount = 2;
    SkBitmap sourceImages[imageCount];
    for (int i = 0; i < imageCount; i++)
    {
        sourceImages[i].setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
        sourceImages[i].allocPixels();
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
    SkBitmap store;
    store.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    store.allocPixels();
    SkBitmapDevice device(store);
    NotificationCounter notificationCounter;
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));
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
    SkBitmap store;
    store.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    store.allocPixels();
    SkBitmapDevice device(store);
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));
    // test will fail if nbIterations is not in sync with
    // BITMAPS_TO_KEEP in SkGPipeWrite.cpp
    const int nbIterations = 5;
    size_t bytesAllocated = 0;
    for(int pass = 0; pass < 2; ++pass) {
        for(int i = 0; i < nbIterations; ++i) {
            SkPaint paint;
            SkBitmap paintPattern;
            paintPattern.setConfig(SkBitmap::kARGB_8888_Config, 10, 10);
            paintPattern.allocPixels();
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
    SkBitmap store;
    store.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    store.allocPixels();

    SkBitmap sourceImage;
    // 100 by 100 image, takes 40,000 bytes in memory
    sourceImage.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    sourceImage.allocPixels();

    // 1 under : should not store the image
    {
        SkBitmapDevice device(store);
        SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));
        canvas->setBitmapSizeThreshold(39999);
        canvas->drawBitmap(sourceImage, 0, 0, NULL);
        size_t newBytesAllocated = canvas->storageAllocatedForRecording();
        REPORTER_ASSERT(reporter, newBytesAllocated == 0);
    }

    // exact value : should store the image
    {
        SkBitmapDevice device(store);
        SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));
        canvas->setBitmapSizeThreshold(40000);
        canvas->drawBitmap(sourceImage, 0, 0, NULL);
        size_t newBytesAllocated = canvas->storageAllocatedForRecording();
        REPORTER_ASSERT(reporter, newBytesAllocated > 0);
    }

    // 1 over : should still store the image
    {
        SkBitmapDevice device(store);
        SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));
        canvas->setBitmapSizeThreshold(40001);
        canvas->drawBitmap(sourceImage, 0, 0, NULL);
        size_t newBytesAllocated = canvas->storageAllocatedForRecording();
        REPORTER_ASSERT(reporter, newBytesAllocated > 0);
    }
}


typedef void* PixelPtr;
// Returns an opaque pointer which, either points to a GrTexture or RAM pixel
// buffer. Used to test pointer equality do determine whether a surface points
// to the same pixel data storage as before.
static PixelPtr getSurfacePixelPtr(SkSurface* surface, bool useGpu) {
    return useGpu ? surface->getCanvas()->getDevice()->accessBitmap(false).getTexture() :
        surface->getCanvas()->getDevice()->accessBitmap(false).getPixels();
}

static void TestDeferredCanvasSurface(skiatest::Reporter* reporter, GrContextFactory* factory) {
    SkImageInfo imageSpec = {
        10,  // width
        10,  // height
        kPMColor_SkColorType,
        kPremul_SkAlphaType
    };
    SkSurface* surface;
    bool useGpu = NULL != factory;
#if SK_SUPPORT_GPU
    if (useGpu) {
        GrContext* context = factory->get(GrContextFactory::kNative_GLContextType);
        if (NULL == context) {
            return;
        }

        surface = SkSurface::NewRenderTarget(context, imageSpec);
    } else {
        surface = SkSurface::NewRaster(imageSpec);
    }
#else
    SkASSERT(!useGpu);
    surface = SkSurface::NewRaster(imageSpec);
#endif
    SkASSERT(NULL != surface);
    SkAutoTUnref<SkSurface> aur(surface);
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface));

    SkImage* image1 = canvas->newImageSnapshot();
    SkAutoTUnref<SkImage> aur_i1(image1);
    PixelPtr pixels1 = getSurfacePixelPtr(surface, useGpu);
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
    PixelPtr pixels2 = getSurfacePixelPtr(surface, useGpu);
    REPORTER_ASSERT(reporter, pixels1 != pixels2);
    // Verify copy-on write with a draw operation that gets deferred by
    // the in order draw buffer.
    SkPaint paint;
    canvas->drawPaint(paint);
    SkImage* image4 = canvas->newImageSnapshot();  // implicit flush
    SkAutoTUnref<SkImage> aur_i4(image4);
    REPORTER_ASSERT(reporter, image4->uniqueID() != image3->uniqueID());
    PixelPtr pixels3 = getSurfacePixelPtr(surface, useGpu);
    REPORTER_ASSERT(reporter, pixels2 != pixels3);
    // Verify that a direct canvas flush with a pending draw does not trigger
    // a copy on write when the surface is not sharing its buffer with an
    // SkImage.
    canvas->clear(SK_ColorWHITE);
    canvas->flush();
    PixelPtr pixels4 = getSurfacePixelPtr(surface, useGpu);
    canvas->drawPaint(paint);
    canvas->flush();
    PixelPtr pixels5 = getSurfacePixelPtr(surface, useGpu);
    REPORTER_ASSERT(reporter, pixels4 == pixels5);
}

static void TestDeferredCanvasSetSurface(skiatest::Reporter* reporter, GrContextFactory* factory) {
    SkImageInfo imageSpec = {
        10,  // width
        10,  // height
        kPMColor_SkColorType,
        kPremul_SkAlphaType
    };
    SkSurface* surface;
    SkSurface* alternateSurface;
    bool useGpu = NULL != factory;
#if SK_SUPPORT_GPU
    if (useGpu) {
        GrContext* context = factory->get(GrContextFactory::kNative_GLContextType);
        if (NULL == context) {
            return;
        }
        surface = SkSurface::NewRenderTarget(context, imageSpec);
        alternateSurface = SkSurface::NewRenderTarget(context, imageSpec);
    } else {
        surface = SkSurface::NewRaster(imageSpec);
        alternateSurface = SkSurface::NewRaster(imageSpec);
    }
#else
    SkASSERT(!useGpu);
    surface = SkSurface::NewRaster(imageSpec);
    alternateSurface = SkSurface::NewRaster(imageSpec);
#endif
    SkASSERT(NULL != surface);
    SkASSERT(NULL != alternateSurface);
    SkAutoTUnref<SkSurface> aur1(surface);
    SkAutoTUnref<SkSurface> aur2(alternateSurface);
    PixelPtr pixels1 = getSurfacePixelPtr(surface, useGpu);
    PixelPtr pixels2 = getSurfacePixelPtr(alternateSurface, useGpu);
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(surface));
    SkAutoTUnref<SkImage> image1(canvas->newImageSnapshot());
    canvas->setSurface(alternateSurface);
    SkAutoTUnref<SkImage> image2(canvas->newImageSnapshot());
    REPORTER_ASSERT(reporter, image1->uniqueID() != image2->uniqueID());
    // Verify that none of the above operations triggered a surface copy on write.
    REPORTER_ASSERT(reporter, getSurfacePixelPtr(surface, useGpu) == pixels1);
    REPORTER_ASSERT(reporter, getSurfacePixelPtr(alternateSurface, useGpu) == pixels2);
    // Verify that a flushed draw command will trigger a copy on write on alternateSurface.
    canvas->clear(SK_ColorWHITE);
    canvas->flush();
    REPORTER_ASSERT(reporter, getSurfacePixelPtr(surface, useGpu) == pixels1);
    REPORTER_ASSERT(reporter, getSurfacePixelPtr(alternateSurface, useGpu) != pixels2);
}

static void TestDeferredCanvasCreateCompatibleDevice(skiatest::Reporter* reporter) {
    SkBitmap store;
    store.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    store.allocPixels();
    SkBitmapDevice device(store);
    NotificationCounter notificationCounter;
    SkAutoTUnref<SkDeferredCanvas> canvas(SkDeferredCanvas::Create(&device));
    canvas->setNotificationClient(&notificationCounter);
    SkAutoTUnref<SkBaseDevice> secondaryDevice(canvas->createCompatibleDevice(
        SkBitmap::kARGB_8888_Config, 10, 10, device.isOpaque()));
    SkCanvas secondaryCanvas(secondaryDevice.get());
    SkRect rect = SkRect::MakeWH(5, 5);
    SkPaint paint;
    // After spawning a compatible canvas:
    // 1) Verify that secondary canvas is usable and does not report to the notification client.
    secondaryCanvas.drawRect(rect, paint);
    REPORTER_ASSERT(reporter, notificationCounter.fStorageAllocatedChangedCount == 0);
    // 2) Verify that original canvas is usable and still reports to the notification client.
    canvas->drawRect(rect, paint);
    REPORTER_ASSERT(reporter, notificationCounter.fStorageAllocatedChangedCount == 1);
}

static void TestDeferredCanvas(skiatest::Reporter* reporter, GrContextFactory* factory) {
    TestDeferredCanvasBitmapAccess(reporter);
    TestDeferredCanvasFlush(reporter);
    TestDeferredCanvasFreshFrame(reporter);
    TestDeferredCanvasMemoryLimit(reporter);
    TestDeferredCanvasBitmapCaching(reporter);
    TestDeferredCanvasSkip(reporter);
    TestDeferredCanvasBitmapShaderNoLeak(reporter);
    TestDeferredCanvasBitmapSizeThreshold(reporter);
    TestDeferredCanvasCreateCompatibleDevice(reporter);
    TestDeferredCanvasWritePixelsToSurface(reporter);
    TestDeferredCanvasSurface(reporter, NULL);
    TestDeferredCanvasSetSurface(reporter, NULL);
    if (NULL != factory) {
        TestDeferredCanvasSurface(reporter, factory);
        TestDeferredCanvasSetSurface(reporter, factory);
    }
}

#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("DeferredCanvas", TestDeferredCanvasClass, TestDeferredCanvas)
