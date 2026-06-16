/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkPngRustDecoder.h"

#include <utility>

#include "include/core/SkStream.h"
#include "src/codec/SkPngRustCodec.h"

namespace SkPngRustDecoder {

bool IsPng(const void* buff, size_t bytesRead) { return SkPngCodecBase::IsPng(buff, bytesRead); }

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
