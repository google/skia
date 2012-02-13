
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkBitmap.h"
#include "SkDeferredCanvas.h"
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
    partialRect.setXYWH(0, 0, 1, 1);
    create(&store, SkBitmap::kARGB_8888_Config, 0xFFFFFFFF);
    SkDevice device(store);
    SkDeferredCanvas canvas(&device);

    // verify that frame is intially fresh
    REPORTER_ASSERT(reporter, canvas.getDeferredDevice()->isFreshFrame());
    // no clearing op since last call to isFreshFrame -> not fresh
    REPORTER_ASSERT(reporter, !canvas.getDeferredDevice()->isFreshFrame());

    // Verify that clear triggers a fresh frame
    canvas.clear(0x00000000);
    REPORTER_ASSERT(reporter, canvas.getDeferredDevice()->isFreshFrame());

    // Verify that clear with saved state triggers a fresh frame
    canvas.save(SkCanvas::kMatrixClip_SaveFlag);
    canvas.clear(0x00000000);
    canvas.restore();
    REPORTER_ASSERT(reporter, canvas.getDeferredDevice()->isFreshFrame());

    // Verify that clear within a layer does NOT trigger a fresh frame
    canvas.saveLayer(NULL, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);
    canvas.clear(0x00000000);
    canvas.restore();
    REPORTER_ASSERT(reporter, !canvas.getDeferredDevice()->isFreshFrame());

    // Verify that a clear with clipping triggers a fresh frame
    // (clear is not affected by clipping)
    canvas.save(SkCanvas::kMatrixClip_SaveFlag);
    canvas.clipRect(partialRect, SkRegion::kIntersect_Op, false);
    canvas.clear(0x00000000);
    canvas.restore();
    REPORTER_ASSERT(reporter, canvas.getDeferredDevice()->isFreshFrame());    

    // Verify that full frame rects with different forms of opaque paint
    // trigger frames to be marked as fresh
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kFill_Style );
        paint.setAlpha( 255 );
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, canvas.getDeferredDevice()->isFreshFrame());
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
        REPORTER_ASSERT(reporter, canvas.getDeferredDevice()->isFreshFrame());        
    }

    // Verify that full frame rects with different forms of non-opaque paint
    // do not trigger frames to be marked as fresh
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kFill_Style );
        paint.setAlpha( 254 );
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas.getDeferredDevice()->isFreshFrame());
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
        REPORTER_ASSERT(reporter, !canvas.getDeferredDevice()->isFreshFrame());        
    }

    // Verify that incomplete coverage does not trigger a fresh frame
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAlpha(255);
        canvas.drawRect(partialRect, paint);
        REPORTER_ASSERT(reporter, !canvas.getDeferredDevice()->isFreshFrame());
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
        REPORTER_ASSERT(reporter, !canvas.getDeferredDevice()->isFreshFrame());
    }

    // Verify that stroked rect does not trigger a fresh frame
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kStroke_Style );
        paint.setAlpha( 255 );
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas.getDeferredDevice()->isFreshFrame());
    }
    
    // Verify kSrcMode triggers a fresh frame even with transparent color
    {
        SkPaint paint;
        paint.setStyle( SkPaint::kFill_Style );
        paint.setAlpha( 100 );
        paint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas.drawRect(fullRect, paint);
        REPORTER_ASSERT(reporter, !canvas.getDeferredDevice()->isFreshFrame());
    }
}

static void TestDeferredCanvas(skiatest::Reporter* reporter) {
    TestDeferredCanvasBitmapAccess(reporter);
    TestDeferredCanvasFlush(reporter);
    TestDeferredCanvasFreshFrame(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("DeferredCanvas", TestDeferredCanvasClass, TestDeferredCanvas)
