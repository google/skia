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
                  bool supportsScanlineDecoding) {
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.detach()));
    if (!codec) {
        ERRORF(r, "Unable to decode '%s'", path);
        return;
    }
    SkImageInfo info = codec->getInfo();
    REPORTER_ASSERT(r, info.dimensions() == size);
    SkBitmap bm;
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);
    SkImageGenerator::Result result =
        codec->getPixels(info, bm.getPixels(), bm.rowBytes(), NULL, NULL, NULL);
    REPORTER_ASSERT(r, result == SkImageGenerator::kSuccess);

    SkMD5::Digest digest1, digest2;
    md5(bm, &digest1);

    bm.eraseColor(SK_ColorYELLOW);

    result =
        codec->getPixels(info, bm.getPixels(), bm.rowBytes(), NULL, NULL, NULL);

    REPORTER_ASSERT(r, result == SkImageGenerator::kSuccess);
    // verify that re-decoding gives the same result.
    md5(bm, &digest2);
    REPORTER_ASSERT(r, digest1 == digest2);

    SkScanlineDecoder* scanlineDecoder = codec->getScanlineDecoder(info);
    if (supportsScanlineDecoding) {
        bm.eraseColor(SK_ColorYELLOW);
        REPORTER_ASSERT(r, scanlineDecoder);
        for (int y = 0; y < info.height(); y++) {
            result = scanlineDecoder->getScanlines(bm.getAddr(0, y), 1, 0);
            REPORTER_ASSERT(r, result == SkImageGenerator::kSuccess);
        }
        // verify that scanline decoding gives the same result.
        SkMD5::Digest digest3;
        md5(bm, &digest3);
        REPORTER_ASSERT(r, digest3 == digest1);
    } else {
        REPORTER_ASSERT(r, !scanlineDecoder);
    }
}

DEF_TEST(Codec, r) {
    // WBMP
    check(r, "mandrill.wbmp", SkISize::Make(512, 512), false);

    // BMP
    check(r, "randPixels.bmp", SkISize::Make(8, 8), false);

    // ICO
    // These two tests examine interestingly different behavior:
    // Decodes an embedded BMP image
    check(r, "color_wheel.ico", SkISize::Make(128, 128), false);
    // Decodes an embedded PNG image
    check(r, "google_chrome.ico", SkISize::Make(256, 256), false);

    // PNG
    check(r, "arrow.png", SkISize::Make(187, 312), true);
    check(r, "baby_tux.png", SkISize::Make(240, 246), true);
    check(r, "color_wheel.png", SkISize::Make(128, 128), true);
    check(r, "half-transparent-white-pixel.png", SkISize::Make(1, 1), true);
    check(r, "mandrill_128.png", SkISize::Make(128, 128), true);
    check(r, "mandrill_16.png", SkISize::Make(16, 16), true);
    check(r, "mandrill_256.png", SkISize::Make(256, 256), true);
    check(r, "mandrill_32.png", SkISize::Make(32, 32), true);
    check(r, "mandrill_512.png", SkISize::Make(512, 512), true);
    check(r, "mandrill_64.png", SkISize::Make(64, 64), true);
    check(r, "plane.png", SkISize::Make(250, 126), true);
    check(r, "randPixels.png", SkISize::Make(8, 8), true);
    check(r, "yellow_rose.png", SkISize::Make(400, 301), true);
}
