/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkPngRustEncoder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"
#include "tools/ToolUtils.h"

DEF_TEST(RustEncodePng_smoke_test, r) {
    SkBitmap bitmap;
    bool success = ToolUtils::GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
    if (!success) {
        return;
    }

    SkPixmap src;
    success = bitmap.peekPixels(&src);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkDynamicMemoryWStream dst;
    SkPngRustEncoder::Options options;
    success = SkPngRustEncoder::Encode(&dst, src, options);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkBitmap roundtrip;
    success = ToolUtils::DecodeDataToBitmap(dst.detachAsData(), &roundtrip);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    success = ToolUtils::equal_pixels(bitmap, roundtrip);
    REPORTER_ASSERT(r, success);
}
