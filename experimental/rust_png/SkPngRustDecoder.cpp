/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/SkPngRustDecoder.h"

#include <utility>

#include "experimental/rust_png/impl/SkPngRustCodec.h"
#include "include/core/SkStream.h"

namespace SkPngRustDecoder {

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream, SkCodec::Result* result) {
    return SkPngRustCodec::MakeFromStream(std::move(stream), result);
}

}  // namespace SkPngRustDecoder
