
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkBitmap.h"
#include "SkDeferredCanvas.h"
#include "SkDevice.h"
#include "SkShader.h"

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
    SkDevice device(store);
    SkDeferredCanvas canvas(&device);

    canvas.clear(0x00000000);

    SkAutoLockPixels alp(store);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0xFFFFFFFF); //verify that clear was deferred
    SkBitmap accessed = canvas.getDevice()->accessBitmap(false);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0x00000000); //verify that clear was executed
    REPORTER_ASSERT(reporter, accessed.pixelRef() == store.pixelRef());
}

static void TestDeferredCanvasFlush(skiatest::Reporter* reporter) {
    SkBitmap store;

    create(&store, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice device(store);
    SkDeferredCanvas canvas(&device);

    canvas.clear(0x00000000);

    SkAutoLockPixels alp(store);
    REPORTER_ASSERT(reporter, store.getColor(0,0) == 0xFFFFFFFF); //verify that clear was deferred
    canvas.flush();
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
    SkDevice device(store);
    SkDeferredCanvas canvas(&device);

    // verify that frame is intially fresh
    REPORTER_ASSERT(reporter, canvas.isFreshFrame());
    // no clearing op since last call to isFreshFrame -> not fresh
    REPORTER_ASSERT(reporter, !canvas.isFreshFrame());

    // Verify that clear triggers a fresh frame
    canvas.clear(0x00000000);
    REPORTER_ASSERT(reporter, canvas.isFreshFrame());

    // Verify that clear with saved state triggers a fresh frame
    canvas.save(SkCanvas::kMatrixClip_SaveFlag);
    canvas.clear(0x00000000);
    canvas.restore();
    REPORTER_ASSERT(reporter, canvas.isFreshFrame());

    // Verify that clear within a layer does NOT trigger a fresh frame
    canvas.saveLayer(NULL, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);
    canvas.clear(0x00000000);
    canvas.restore();
    REPORTER_ASSERT(reporter, !canvas.isFreshFrame());

    // Verify that a clear with clipping triggers a fresh frame
    // (clear is not affected by clipping)
    canvas.save(SkCanvas::kMatrixClip_SaveFlag);
    canvas.clipRect(partialRect, SkRegion::kIntersect_Op, false);
    canvas.clear(0x00000000);
    canvas.restore();
    REPORTER_ASSERT(reporter, canvas.isFreshFrame());

    // Verify that full frame rects with different forms of opaque paint
    // trigger frames to be marked as fresh
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kFill_Style );
        paint.setAlpha( 255 );
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, canvas.isFreshFrame());
    }
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kFill_Style );
        SkBitmap bmp;
        create(&bmp, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
        bmp.setIsOpaque(true);
        SkShader* shader = SkShader::CreateBitmapShader(bmp,
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
        paint.setShader(shader)->unref();
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, canvas.isFreshFrame());
    }

    // Verify that full frame rects with different forms of non-opaque paint
    // do not trigger frames to be marked as fresh
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kFill_Style );
        paint.setAlpha( 254 );
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas.isFreshFrame());
    }
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kFill_Style );
        SkBitmap bmp;
        create(&bmp, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
        bmp.setIsOpaque(false);
        SkShader* shader = SkShader::CreateBitmapShader(bmp,
            SkShader::kClamp_TileMode, SkShader::kClamp_TileMode);
        paint.setShader(shader)->unref();
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas.isFreshFrame());
    }

    // Verify that incomplete coverage does not trigger a fresh frame
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        canvas.drawRect(partialRect, paint);
        REPORTER_ASSERT(reporter, !canvas.isFreshFrame());
    }

    // Verify that incomplete coverage due to clipping does not trigger a fresh
    // frame
    {
        canvas.save(SkCanvas::kMatrixClip_SaveFlag);
        canvas.clipRect(partialRect, SkRegion::kIntersect_Op, false);
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas.isFreshFrame());
    }

    // Verify that stroked rect does not trigger a fresh frame
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kStroke_Style );
        paint.setAlpha( 255 );
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas.isFreshFrame());
    }

    // Verify kSrcMode triggers a fresh frame even with transparent color
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kFill_Style );
        paint.setAlpha( 100 );
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas.isFreshFrame());
    }
}

class MockDevice : public SkDevice {
public:
    MockDevice(const SkBitmap& bm) : SkDevice(bm) {
        fDrawBitmapCallCount = 0;
    }
    virtual void drawBitmap(const SkDraw&, const SkBitmap&,
                            const SkIRect*,
                            const SkMatrix&, const SkPaint&) {
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
    SkDeferredCanvas canvas(&mockDevice);
    canvas.setMaxRecordingStorage(160000);

    SkBitmap sourceImage;
    // 100 by 100 image, takes 40,000 bytes in memory
    sourceImage.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    sourceImage.allocPixels();

    for (int i = 0; i < 5; i++) {
        sourceImage.notifyPixelsChanged(); // to force re-serialization
        canvas.drawBitmap(sourceImage, 0, 0, NULL);
    }

    REPORTER_ASSERT(reporter, mockDevice.fDrawBitmapCallCount == 4);
}

class NotificationCounter : public SkDeferredCanvas::NotificationClient {
public:
    NotificationCounter() {
        fPrepareForDrawCount = fStorageAllocatedChangedCount = fFlushedDrawCommandsCount = 0;
    }

    virtual void prepareForDraw() SK_OVERRIDE {
        fPrepareForDrawCount++;
    }
    virtual void storageAllocatedForRecordingChanged(size_t size) SK_OVERRIDE {
        fStorageAllocatedChangedCount++;
    }
    virtual void flushedDrawCommands() SK_OVERRIDE {
        fFlushedDrawCommandsCount++;
    }

    int fPrepareForDrawCount;
    int fStorageAllocatedChangedCount;
    int fFlushedDrawCommandsCount;
};

static void TestDeferredCanvasBitmapCaching(skiatest::Reporter* reporter) {
    SkBitmap store;
    store.setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
    store.allocPixels();
    SkDevice device(store);
    NotificationCounter notificationCounter;
    SkDeferredCanvas canvas(&device);
    canvas.setNotificationClient(&notificationCounter);

    const int imageCount = 2;
    SkBitmap sourceImages[imageCount];
    for (int i = 0; i < imageCount; i++)
    {
        sourceImages[i].setConfig(SkBitmap::kARGB_8888_Config, 100, 100);
        sourceImages[i].allocPixels();
    }

    size_t bitmapSize = sourceImages[0].getSize();

    canvas.drawBitmap(sourceImages[0], 0, 0, NULL);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fStorageAllocatedChangedCount);
    // stored bitmap + drawBitmap command
    REPORTER_ASSERT(reporter, canvas.storageAllocatedForRecording() > bitmapSize);

    // verify that nothing can be freed at this point
    REPORTER_ASSERT(reporter, 0 == canvas.freeMemoryIfPossible(~0U));

    // verify that flush leaves image in cache
    REPORTER_ASSERT(reporter, 0 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, 0 == notificationCounter.fPrepareForDrawCount);
    canvas.flush();
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fPrepareForDrawCount);
    REPORTER_ASSERT(reporter, canvas.storageAllocatedForRecording() >= bitmapSize);

    // verify that after a flush, cached image can be freed
    REPORTER_ASSERT(reporter, canvas.freeMemoryIfPossible(~0U) >= bitmapSize);

    // Verify that caching works for avoiding multiple copies of the same bitmap
    canvas.drawBitmap(sourceImages[0], 0, 0, NULL);
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fStorageAllocatedChangedCount);
    canvas.drawBitmap(sourceImages[0], 0, 0, NULL);
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fStorageAllocatedChangedCount);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, canvas.storageAllocatedForRecording() < 2 * bitmapSize);

    // Verify partial eviction based on bytesToFree
    canvas.drawBitmap(sourceImages[1], 0, 0, NULL);
    REPORTER_ASSERT(reporter, 1 == notificationCounter.fFlushedDrawCommandsCount);
    canvas.flush();
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter, canvas.storageAllocatedForRecording() > 2 * bitmapSize);
    size_t bytesFreed = canvas.freeMemoryIfPossible(1);
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fFlushedDrawCommandsCount);
    REPORTER_ASSERT(reporter,  bytesFreed >= bitmapSize);
    REPORTER_ASSERT(reporter,  bytesFreed < 2*bitmapSize);

    // Verifiy that partial purge works, image zero is in cache but not reffed by
    // a pending draw, while image 1 is locked-in.
    canvas.freeMemoryIfPossible(~0U);
    REPORTER_ASSERT(reporter, 2 == notificationCounter.fFlushedDrawCommandsCount);
    canvas.drawBitmap(sourceImages[0], 0, 0, NULL);
    canvas.flush();
    canvas.drawBitmap(sourceImages[1], 0, 0, NULL);
    bytesFreed = canvas.freeMemoryIfPossible(~0U);
    // only one bitmap should have been freed.
    REPORTER_ASSERT(reporter,  bytesFreed >= bitmapSize);
    REPORTER_ASSERT(reporter,  bytesFreed < 2*bitmapSize);
    // Clear for next test
    canvas.flush();
    canvas.freeMemoryIfPossible(~0U);
    REPORTER_ASSERT(reporter, canvas.storageAllocatedForRecording() < bitmapSize);

    // Verify the image cache is sensitive to genID bumps
    canvas.drawBitmap(sourceImages[1], 0, 0, NULL);
    sourceImages[1].notifyPixelsChanged();
    canvas.drawBitmap(sourceImages[1], 0, 0, NULL);
    REPORTER_ASSERT(reporter, canvas.storageAllocatedForRecording() > 2*bitmapSize);
}

static void TestDeferredCanvas(skiatest::Reporter* reporter) {
    TestDeferredCanvasBitmapAccess(reporter);
    TestDeferredCanvasFlush(reporter);
    TestDeferredCanvasFreshFrame(reporter);
    TestDeferredCanvasMemoryLimit(reporter);
    TestDeferredCanvasBitmapCaching(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DeferredCanvas", TestDeferredCanvasClass, TestDeferredCanvas)
