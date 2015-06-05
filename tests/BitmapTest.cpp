/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkMallocPixelRef.h"
#include "Test.h"

static void test_peekpixels(skiatest::Reporter* reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(10, 10);

    SkPixmap pmap;
    SkBitmap bm;

    // empty should return false
    REPORTER_ASSERT(reporter, !bm.peekPixels(NULL));
    REPORTER_ASSERT(reporter, !bm.peekPixels(&pmap));

    // no pixels should return false
    bm.setInfo(SkImageInfo::MakeN32Premul(10, 10));
    REPORTER_ASSERT(reporter, !bm.peekPixels(NULL));
    REPORTER_ASSERT(reporter, !bm.peekPixels(&pmap));

    // real pixels should return true
    bm.allocPixels(info);
    REPORTER_ASSERT(reporter, bm.peekPixels(NULL));
    REPORTER_ASSERT(reporter, bm.peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.info() == bm.info());
    REPORTER_ASSERT(reporter, pmap.addr() == bm.getPixels());
    REPORTER_ASSERT(reporter, pmap.rowBytes() == bm.rowBytes());
    REPORTER_ASSERT(reporter, pmap.ctable() == bm.getColorTable());
}

// https://code.google.com/p/chromium/issues/detail?id=446164
static void test_bigalloc(skiatest::Reporter* reporter) {
    const int width = 0x40000001;
    const int height = 0x00000096;
    const SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);

    SkBitmap bm;
    REPORTER_ASSERT(reporter, !bm.tryAllocPixels(info));

    SkPixelRef* pr = SkMallocPixelRef::NewAllocate(info, info.minRowBytes(), NULL);
    REPORTER_ASSERT(reporter, !pr);
}

static void test_allocpixels(skiatest::Reporter* reporter) {
    const int width = 10;
    const int height = 10;
    const SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
    const size_t explicitRowBytes = info.minRowBytes() + 24;

    SkBitmap bm;
    bm.setInfo(info);
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());
    bm.allocPixels();
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());
    bm.reset();
    bm.allocPixels(info);
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());

    bm.setInfo(info, explicitRowBytes);
    REPORTER_ASSERT(reporter, explicitRowBytes == bm.rowBytes());
    bm.allocPixels();
    REPORTER_ASSERT(reporter, explicitRowBytes == bm.rowBytes());
    bm.reset();
    bm.allocPixels(info, explicitRowBytes);
    REPORTER_ASSERT(reporter, explicitRowBytes == bm.rowBytes());

    bm.reset();
    bm.setInfo(info, 0);
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());
    bm.reset();
    bm.allocPixels(info, 0);
    REPORTER_ASSERT(reporter, info.minRowBytes() == bm.rowBytes());

    bm.reset();
    bool success = bm.setInfo(info, info.minRowBytes() - 1);   // invalid for 32bit
    REPORTER_ASSERT(reporter, !success);
    REPORTER_ASSERT(reporter, bm.isNull());
}

static void test_bigwidth(skiatest::Reporter* reporter) {
    SkBitmap bm;
    int width = 1 << 29;    // *4 will be the high-bit of 32bit int

    SkImageInfo info = SkImageInfo::MakeA8(width, 1);
    REPORTER_ASSERT(reporter, bm.setInfo(info));
    REPORTER_ASSERT(reporter, bm.setInfo(info.makeColorType(kRGB_565_SkColorType)));

    // for a 4-byte config, this width will compute a rowbytes of 0x80000000,
    // which does not fit in a int32_t. setConfig should detect this, and fail.

    // TODO: perhaps skia can relax this, and only require that rowBytes fit
    //       in a uint32_t (or larger), but for now this is the constraint.

    REPORTER_ASSERT(reporter, !bm.setInfo(info.makeColorType(kN32_SkColorType)));
}

/**
 *  This test contains basic sanity checks concerning bitmaps.
 */
DEF_TEST(Bitmap, reporter) {
    // Zero-sized bitmaps are allowed
    for (int width = 0; width < 2; ++width) {
        for (int height = 0; height < 2; ++height) {
            SkBitmap bm;
            bool setConf = bm.setInfo(SkImageInfo::MakeN32Premul(width, height));
            REPORTER_ASSERT(reporter, setConf);
            if (setConf) {
                bm.allocPixels();
            }
            REPORTER_ASSERT(reporter, SkToBool(width & height) != bm.empty());
        }
    }

    test_bigwidth(reporter);
    test_allocpixels(reporter);
    test_bigalloc(reporter);
    test_peekpixels(reporter);
}
