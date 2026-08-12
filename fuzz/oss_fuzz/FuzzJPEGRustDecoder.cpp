/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/decoder/SkJpegRustDecoder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"

bool FuzzJPEGRustDecoder(const uint8_t* data, size_t size) {
    if (size == 0) {
        return false;
    }

    auto stream = std::make_unique<SkMemoryStream>(data, size, false);
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkJpegRustDecoder::Decode(std::move(stream), &result);

    if (!codec || result != SkCodec::kSuccess) {
        return false;
    }

    SkImageInfo info = codec->getInfo();
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(info)) {
        return false;
    }

    // We don't care if the decode succeeds or fails — we just want to make sure
    // it doesn't crash or trigger undefined behavior.
    (void)codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes());

    // Exercise the incremental state machine on malformed and truncated inputs
    // accepted by the metadata parser. A second call without new input checks
    // that retrying a recoverable EOF remains safe and idempotent.
    if (codec->startIncrementalDecode(
                info, bitmap.getPixels(), bitmap.rowBytes()) == SkCodec::kSuccess) {
        int rowsDecoded = 0;
        if (codec->incrementalDecode(&rowsDecoded) == SkCodec::kIncompleteInput) {
            (void)codec->incrementalDecode(&rowsDecoded);
        }
    }

    return true;
}

#if defined(SK_BUILD_FOR_LIBFUZZER)
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // Limit input size to prevent timeouts on heavily-compressed streams.
    if (size > 65536) {
        return 0;
    }

    FuzzJPEGRustDecoder(data, size);

    return 0;
}
#endif
