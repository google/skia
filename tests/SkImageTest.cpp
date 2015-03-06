/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkImagePriv.h"
#include "Test.h"

static const int gWidth = 20;
static const int gHeight = 20;

// Tests that SkNewImageFromBitmap obeys pixelref origin.
DEF_TEST(SkImageFromBitmap_extractSubset, reporter) {
    SkAutoTUnref<SkImage> image;
    {
        SkBitmap srcBitmap;
        srcBitmap.allocN32Pixels(gWidth, gHeight);
        srcBitmap.eraseColor(SK_ColorRED);
        SkBitmapDevice dev(srcBitmap);
        SkCanvas canvas(&dev);
        SkIRect r = SkIRect::MakeXYWH(5, 5, gWidth - 5, gWidth - 5);
        SkPaint p;
        p.setColor(SK_ColorGREEN);
        canvas.drawIRect(r, p);
        SkBitmap dstBitmap;
        srcBitmap.extractSubset(&dstBitmap, r);
        image.reset(SkNewImageFromBitmap(dstBitmap, true, NULL));
    }

    SkBitmap tgt;
    tgt.allocN32Pixels(gWidth, gHeight);
    SkBitmapDevice dev(tgt);
    SkCanvas canvas(&dev);
    canvas.clear(SK_ColorTRANSPARENT);
    canvas.drawImage(image, 0, 0, NULL);

    uint32_t pixel = 0;
    SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
    canvas.readPixels(info, &pixel, 4, gWidth - 6, gWidth - 6);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    canvas.readPixels(info, &pixel, 4, gWidth - 5, gWidth - 5);
    REPORTER_ASSERT(reporter, pixel == SK_ColorTRANSPARENT);
}
