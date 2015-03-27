/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkMD5.h"
#include "Test.h"

static SkStreamAsset* resource(const char path[]) {
    SkString fullPath = GetResourcePath(path);
    return SkStream::NewFromFile(fullPath.c_str());
}

static void md5(const SkBitmap& bm, SkMD5::Digest* digest) {
    SkAutoLockPixels autoLockPixels(bm);
    SkASSERT(bm.getPixels());
    SkMD5 md5;
    size_t rowLen = bm.info().bytesPerPixel() * bm.width();
    for (int y = 0; y < bm.height(); ++y) {
        md5.update(static_cast<uint8_t*>(bm.getAddr(0, y)), rowLen);
    }
    md5.finish(*digest);
}

static void check(skiatest::Reporter* r,
                  const char path[],
                  SkISize size,
                  bool canRewind) {
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkImageGenerator> gen(
            SkCodec::NewFromStream(stream.detach()));
    if (!gen) {
        ERRORF(r, "Unable to decode '%s'", path);
        return;
    }
    SkImageInfo info = gen->getInfo();
    REPORTER_ASSERT(r, info.dimensions() == size);
    SkBitmap bm;
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);
    SkImageGenerator::Result result =
        gen->getPixels(info, bm.getPixels(), bm.rowBytes(), NULL, NULL, NULL);
    REPORTER_ASSERT(r, result == SkImageGenerator::kSuccess);

    SkMD5::Digest digest1, digest2;
    md5(bm, &digest1);

    bm.eraseColor(SK_ColorYELLOW);

    result =
        gen->getPixels(info, bm.getPixels(), bm.rowBytes(), NULL, NULL, NULL);

    // All ImageGenerators should support re-decoding the pixels.
    // It is a known bug that some can not.
    if (canRewind) {
        REPORTER_ASSERT(r, result == SkImageGenerator::kSuccess);
        // verify that re-decoding gives the same result.
        md5(bm, &digest2);
        REPORTER_ASSERT(r, digest1 == digest2);
    }
}

DEF_TEST(Codec, r) {
    // WBMP
    check(r, "mandrill.wbmp", SkISize::Make(512, 512), true);

    // BMP
    // TODO (msarett): SkBmpCodec should be able to rewind.
    check(r, "randPixels.bmp", SkISize::Make(8, 8), false);

    // ICO
    // TODO (msarett): SkIcoCodec should be able to rewind.
    check(r, "color_wheel.ico", SkISize::Make(128, 128), false);

    // PNG
    // TODO (scroggo): SkPngCodec should be able to rewind.
    check(r, "arrow.png", SkISize::Make(187, 312), false);
    check(r, "baby_tux.png", SkISize::Make(240, 246), false);
    check(r, "color_wheel.png", SkISize::Make(128, 128), false);
    check(r, "half-transparent-white-pixel.png", SkISize::Make(1, 1), false);
    check(r, "mandrill_128.png", SkISize::Make(128, 128), false);
    check(r, "mandrill_16.png", SkISize::Make(16, 16), false);
    check(r, "mandrill_256.png", SkISize::Make(256, 256), false);
    check(r, "mandrill_32.png", SkISize::Make(32, 32), false);
    check(r, "mandrill_512.png", SkISize::Make(512, 512), false);
    check(r, "mandrill_64.png", SkISize::Make(64, 64), false);
    check(r, "plane.png", SkISize::Make(250, 126), false);
    check(r, "randPixels.png", SkISize::Make(8, 8), false);
    check(r, "yellow_rose.png", SkISize::Make(400, 301), false);
}
