/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/core/SkData.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "tests/Test.h"
#include "tools/Resources.h"

DEF_TEST(CodecImageGenerator_null, r) {
    REPORTER_ASSERT(r, !SkCodecImageGenerator::MakeFromEncodedCodec(nullptr));
    REPORTER_ASSERT(r, !SkCodecImageGenerator::MakeFromEncodedCodec(SkData::MakeEmpty()));
    REPORTER_ASSERT(r, !SkCodecImageGenerator::MakeFromCodec(nullptr, nullptr));

    const char* path = "images/flightAnim.gif";
    auto data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "missing %s", path);
        return;
    }

    // This SkData contains an encoded image, so the first call succeeds, but the second method
    // requires a non-null SkCodec.
    REPORTER_ASSERT(r,  SkCodecImageGenerator::MakeFromEncodedCodec(data));
    REPORTER_ASSERT(r, !SkCodecImageGenerator::MakeFromCodec(nullptr, data));
}
