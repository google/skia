/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "Test.h"

#include "SkBitmap.h"
#include "SkJpegEncoder.h"
#include "SkStream.h"

DEF_TEST(Encode_Jpeg, r) {
    SkBitmap bitmap;
    bool success = GetResourceAsBitmap("mandrill_128.png", &bitmap);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkPixmap src;
    success = bitmap.peekPixels(&src);
    REPORTER_ASSERT(r, success);

    SkDynamicMemoryWStream dst0, dst1, dst2;
    SkJpegEncoder::Encode(&dst0, src, SkJpegEncoder::Options());

    auto encoder1 = SkJpegEncoder::Make(&dst1, src, SkJpegEncoder::Options());
    REPORTER_ASSERT(r, !encoder1->encodeRows(0));
    for (int i = 0; i < src.height(); i++) {
        success = encoder1->encodeRows(1);
        REPORTER_ASSERT(r, success);
    }

    auto encoder2 = SkJpegEncoder::Make(&dst2, src, SkJpegEncoder::Options());
    for (int i = 0; i < src.height(); i+=3) {
        success = encoder2->encodeRows(3);
        REPORTER_ASSERT(r, success);
    }

    sk_sp<SkData> data0 = dst0.detachAsData();
    sk_sp<SkData> data1 = dst1.detachAsData();
    sk_sp<SkData> data2 = dst2.detachAsData();
    REPORTER_ASSERT(r, data0->equals(data1.get()));
    REPORTER_ASSERT(r, data0->equals(data2.get()));

    REPORTER_ASSERT(r, !encoder1->encodeRows(0));
    REPORTER_ASSERT(r, !encoder1->encodeRows(1));
    REPORTER_ASSERT(r, !encoder2->encodeRows(1));
}
