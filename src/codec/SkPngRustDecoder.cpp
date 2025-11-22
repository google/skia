/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkPngRustDecoder.h"

#include <utility>

#include "include/core/SkStream.h"
#include "src/codec/SkPngRustCodec.h"

namespace SkPngRustDecoder {

bool IsPng(const void* buff, size_t bytesRead) {
    static constexpr unsigned char pngSignature[] = {
            0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    if (bytesRead < sizeof(pngSignature)) {
        return false;
    }

    return memcmp(buff, pngSignature, sizeof(pngSignature)) == 0;
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    SkCodec::Result resultStorage;
    if (!result) {
        result = &resultStorage;
    }
    return SkPngRustCodec::MakeFromStream(std::move(stream), result);
}

}  // namespace SkPngRustDecoder
