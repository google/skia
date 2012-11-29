
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapFactory.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkData.h"
#include "SkImageEncoder.h"
#include "SkPaint.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "Test.h"

static SkBitmap* create_bitmap() {
    SkBitmap* bm = SkNEW(SkBitmap);
    const int W = 100, H = 100;
    bm->setConfig(SkBitmap::kARGB_8888_Config, W, H);
    bm->allocPixels();
    bm->eraseColor(SK_ColorBLACK);
    SkCanvas canvas(*bm);
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    canvas.drawRectCoords(0, 0, SkIntToScalar(W/2), SkIntToScalar(H/2), paint);
    return bm;
}

static SkData* create_data_from_bitmap(const SkBitmap& bm) {
    SkDynamicMemoryWStream stream;
    if (SkImageEncoder::EncodeStream(&stream, bm, SkImageEncoder::kPNG_Type, 100)) {
        return stream.copyToData();
    }
    return NULL;
}

static void assert_bounds_equal(skiatest::Reporter* reporter, const SkBitmap& bm1,
                                const SkBitmap& bm2) {
    REPORTER_ASSERT(reporter, bm1.width() == bm2.width());
    REPORTER_ASSERT(reporter, bm1.height() == bm2.height());
}

static void TestBitmapFactory(skiatest::Reporter* reporter) {
    SkAutoTDelete<SkBitmap> bitmap(create_bitmap());
    SkASSERT(bitmap.get() != NULL);

    SkAutoDataUnref encodedBitmap(create_data_from_bitmap(*bitmap.get()));
    if (encodedBitmap.get() == NULL) {
        // Encoding failed.
        return;
    }

    SkBitmap bitmapFromFactory;
    bool success = SkBitmapFactory::DecodeBitmap(&bitmapFromFactory, encodedBitmap);
    // This assumes that if the encoder worked, the decoder should also work, so the above call
    // should not fail.
    REPORTER_ASSERT(reporter, success);
    assert_bounds_equal(reporter, *bitmap.get(), bitmapFromFactory);
    REPORTER_ASSERT(reporter, bitmapFromFactory.pixelRef() != NULL);

    // When only requesting that the bounds be decoded, the bounds should be set properly while
    // the pixels should be empty.
    SkBitmap boundedBitmap;
    success = SkBitmapFactory::DecodeBitmap(&boundedBitmap, encodedBitmap,
                                            SkBitmapFactory::kDecodeBoundsOnly_Constraint);
    REPORTER_ASSERT(reporter, success);
    assert_bounds_equal(reporter, *bitmap.get(), boundedBitmap);
    REPORTER_ASSERT(reporter, boundedBitmap.pixelRef() == NULL);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("BitmapFactory", TestBitmapFactoryClass, TestBitmapFactory)
