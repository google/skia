/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/encoder/SkPngRustEncoder.h"

#include <memory>

#include "experimental/rust_png/encoder/impl/SkPngRustEncoderImpl.h"
#include "include/encode/SkEncoder.h"

namespace SkPngRustEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src) {
    std::unique_ptr<SkEncoder> encoder = Make(dst, src);
    return encoder && encoder->encodeRows(src.height());
}

SK_API std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src) {
    return SkPngRustEncoderImpl::Make(dst, src);
}

}  // namespace SkPngRustEncoder
