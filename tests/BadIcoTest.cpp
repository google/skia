/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "Test.h"

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkOSFile.h"
#include "SkOSPath.h"
#include "SkStream.h"

DEF_TEST(BadImage, reporter) {
    const char* const badImages [] = {
        "sigabort_favicon.ico",
        "sigsegv_favicon.ico",
        "sigsegv_favicon_2.ico",
        "ico_leak01.ico",
        "ico_fuzz0.ico",
        "ico_fuzz1.ico",
        "skbug3442.webp",
        "skbug3429.webp",
        "b38116746.ico",
    };

    const char* badImagesFolder = "invalid_images";

    for (size_t i = 0; i < SK_ARRAY_COUNT(badImages); ++i) {
        SkString resourcePath = SkOSPath::Join(badImagesFolder, badImages[i]);
        std::unique_ptr<SkStream> stream(GetResourceAsStream(resourcePath.c_str()));
        std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(stream.release()));

        // These images are corrupt.  It's not important whether we succeed/fail in codec
        // creation or decoding.  We just want to make sure that we don't crash.
        if (codec) {
            SkBitmap bm;
            bm.allocPixels(codec->getInfo());
            codec->getPixels(codec->getInfo(), bm.getPixels(),
                    bm.rowBytes());
        }
    }
}
