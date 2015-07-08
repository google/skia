/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "Test.h"
#include "SkBitmap.h"
#include "SkImageDecoder.h"
#include "SkOSFile.h"

DEF_TEST(BadIco, reporter) {
    const char* const badIcos [] = {
        "sigabort_favicon.ico",
        "sigsegv_favicon.ico",
        "sigsegv_favicon_2.ico",
        "ico_leak01.ico",
        "ico_fuzz0.ico",
        "ico_fuzz1.ico"
    };

    const char* badIcoFolder = "invalid_images";

    SkString resourcePath = GetResourcePath(badIcoFolder);

    SkBitmap bm;
    for (size_t i = 0; i < SK_ARRAY_COUNT(badIcos); ++i) {
        SkString fullPath = SkOSPath::Join(resourcePath.c_str(), badIcos[i]);
        bool success = SkImageDecoder::DecodeFile(fullPath.c_str(), &bm);
        // These files are invalid, and should not decode. More importantly,
        // though, we reached here without crashing.
        REPORTER_ASSERT(reporter, !success);
    }
}
