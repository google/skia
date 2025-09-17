/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/encode/SkPngRustEncoder.h"

#include <memory>

#include "include/encode/SkEncoder.h"
#include "src/encode/SkPngRustEncoderImpl.h"

namespace SkPngRustEncoder {

bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options) {
    std::unique_ptr<SkEncoder> encoder = Make(dst, src, options);
    return encoder && encoder->encodeRows(src.height());
}

SK_API std::unique_ptr<SkEncoder> Make(SkWStream* dst,
                                       const SkPixmap& src,
                                       const Options& options) {
    return SkPngRustEncoderImpl::Make(dst, src, options);
}

}  // namespace SkPngRustEncoder
