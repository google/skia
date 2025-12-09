/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"

#include "experimental/rust_bmp/decoder/SkBmpRustDecoder.h"

bool FuzzBMPRustDecoder(const uint8_t* data, size_t size) {
    if (size == 0) {
        return false;
    }

    auto stream = std::make_unique<SkMemoryStream>(data, size, false);
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkBmpRustDecoder::Decode(std::move(stream), &result);

    if (!codec || result != SkCodec::kSuccess) {
        return false;
    }

    SkImageInfo info = codec->getInfo();
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(info)) {
        return false;
    }

    // Attempt to decode the image
    (void)codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes());

    // We don't care if the decode succeeds or fails - we just want to make sure it doesn't crash
    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Limit input size to prevent timeouts
    if (size > 10240) {
        return 0;
    }

    FuzzBMPRustDecoder(data, size);

    return 0;
}
#endif
