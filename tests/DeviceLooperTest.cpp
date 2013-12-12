/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkDeviceLooper.h"
#include "SkRasterClip.h"

static void make_bm(SkBitmap* bm, int w, int h) {
    bm->setConfig(SkBitmap::kA8_Config, w, h);
    bm->allocPixels();
}

static bool equal(const SkRasterClip& a, const SkRasterClip& b) {
    if (a.isBW()) {
        return b.isBW() && a.bwRgn() == b.bwRgn();
    } else {
        return a.isAA() && a.aaRgn() == b.aaRgn();
    }
}

static const struct {
    SkISize fDevSize;
    SkIRect fRCBounds;
    SkIRect fRect;
} gRec[] = {
    { { 4000, 10 }, { 0, 0, 4000, 10 }, { 0, 0, 4000, 4000 } },
    { { 10, 4000 }, { 0, 0, 10, 4000 }, { 0, 0, 4000, 4000 } },
    // very large devce, small rect
    { { 32000, 10 }, { 0, 0, 32000, 10 }, { 0, 0, 4000, 4000 } },
    { { 10, 32000 }, { 0, 0, 10, 32000 }, { 0, 0, 4000, 4000 } },
    // very large device, small clip
    { { 32000, 10 }, { 0, 0, 4000, 10 }, { 0, 0, 32000, 32000 } },
    { { 10, 32000 }, { 0, 0, 10, 4000 }, { 0, 0, 32000, 32000 } },
};

static void test_simple(skiatest::Reporter* reporter) {

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        SkBitmap bitmap;
        make_bm(&bitmap, gRec[i].fDevSize.width(), gRec[i].fDevSize.height());

        SkRasterClip rc(gRec[i].fRCBounds);

        for (int aa = 0; aa <= 1; ++aa) {
            SkDeviceLooper looper(bitmap, rc, gRec[i].fRect, SkToBool(aa));

            bool valid = looper.next();
            REPORTER_ASSERT(reporter, valid);
            if (valid) {
                REPORTER_ASSERT(reporter, looper.getBitmap().width() == bitmap.width());
                REPORTER_ASSERT(reporter, looper.getBitmap().height() == bitmap.height());
                REPORTER_ASSERT(reporter, equal(looper.getRC(), rc));

                REPORTER_ASSERT(reporter, !looper.next());
            }
        }
        // test that a rect that doesn't intersect returns no loops
        {
            SkIRect r = rc.getBounds();
            r.offset(r.width(), 0);
            SkDeviceLooper looper(bitmap, rc, r, false);
            REPORTER_ASSERT(reporter, !looper.next());
        }
    }
}

// mask-bits are interpreted as the areas where the clip is visible
//  [ 0x01  0x02 ]
//  [ 0x04  0x08 ]
//
static void make_rgn(SkRegion* rgn, int w, int h, unsigned mask) {
    SkASSERT(SkAlign2(w));
    SkASSERT(SkAlign2(h));
    w >>= 1;
    h >>= 1;
    const SkIRect baseR = SkIRect::MakeWH(w, h);

    int bit = 1;
    for (int y = 0; y <= 1; ++y) {
        for (int x = 0; x <= 1; ++x) {
            if (mask & bit) {
                SkIRect r = baseR;
                r.offset(x * w, y * h);
                rgn->op(r, SkRegion::kUnion_Op);
            }
            bit <<= 1;
        }
    }
}

static void test_complex(skiatest::Reporter* reporter) {
    // choose size values that will result in 4 quadrants, given fAA setting
    const int BW_SIZE = 17 * 1000;
    const int AA_SIZE = 7 * 1000;

    struct {
        SkISize fSize;
        bool    fAA;
    } const gRec[] = {
        { { BW_SIZE, BW_SIZE }, false },
        { {  AA_SIZE, AA_SIZE }, true },
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        const int w = gRec[i].fSize.width();
        const int h = gRec[i].fSize.height();

        SkBitmap bitmap;
        make_bm(&bitmap, w, h);

        const SkIRect rect = SkIRect::MakeWH(w, h);

        // mask-bits are interpreted as the areas where the clip is visible
        //  [ 0x01  0x02 ]
        //  [ 0x04  0x08 ]
        //
        for (int mask = 0; mask <= 15; ++mask) {
            SkRegion rgn;
            make_rgn(&rgn, w, h, mask);

            SkRasterClip rc;
            rc.op(rgn, SkRegion::kReplace_Op);

            SkDeviceLooper looper(bitmap, rc, rect, gRec[i].fAA);
            while (looper.next()) {
                REPORTER_ASSERT(reporter, !looper.getRC().isEmpty());
            }
        }
    }
}

DEF_TEST(DeviceLooper, reporter) {
    test_simple(reporter);
    test_complex(reporter);
}
