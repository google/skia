/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/encoder/SkPngRustEncoder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"

DEF_TEST(Encode_png_Rust_smoke_test, r) {
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
    success = SkPngRustEncoder::Encode(&dst, src);

    // TODO(https://crbug.com/379312510): Expect success once encoding via Rust
    // `png` crate actually works.  See also
    // http://review.skia.org/923337/8/tests/SkPngRustEncoderTest.cpp
    REPORTER_ASSERT(r, !success);
}
