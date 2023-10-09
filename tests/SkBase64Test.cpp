/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkBase64.h"
#include "tests/Test.h"

#include <cstring>

using namespace skia_private;

DEF_TEST(SkBase64, reporter) {
    char all[256];
    for (int index = 0; index < 255; ++index) {
        all[index] = (signed char) (index + 1);
    }
    all[255] = 0;

    for (int offset = 0; offset < 6; ++offset) {
        size_t length = 256 - offset;

        // Encode
        size_t predictedEncodeLength = SkBase64::EncodedSize(length);
        size_t actualEncodeLength = SkBase64::Encode(all + offset, length, nullptr);

        REPORTER_ASSERT(reporter, actualEncodeLength == predictedEncodeLength,
                        "input size %zu; output size %zu != %zu", length,
                        actualEncodeLength, predictedEncodeLength);
        AutoTMalloc<char> src(actualEncodeLength + 1);
        size_t n = SkBase64::Encode(all + offset, length, src.get());
        REPORTER_ASSERT(reporter, n == predictedEncodeLength);

        src[SkToInt(actualEncodeLength)] = '\0';

        // Decode
        SkBase64::Error err;

        size_t decodeLength;
        err = SkBase64::Decode(src.get(), actualEncodeLength, nullptr, &decodeLength);
        if (err != SkBase64::kNoError) {
            REPORT_FAILURE(reporter, "err", SkString("SkBase64::Decode failed!"));
            continue;
        }
        REPORTER_ASSERT(reporter, decodeLength == length);

        AutoTMalloc<char> dst(decodeLength);
        err = SkBase64::Decode(src.get(), actualEncodeLength, dst, &decodeLength);
        if (err != SkBase64::kNoError) {
            REPORT_FAILURE(reporter, "err", SkString("SkBase64::Decode failed!"));
            continue;
        }
        REPORTER_ASSERT(reporter, decodeLength == length);

        REPORTER_ASSERT(reporter, (strcmp((const char*) (all + offset), dst.get()) == 0));
    }
}
