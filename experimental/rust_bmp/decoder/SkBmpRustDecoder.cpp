/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/SkBmpRustDecoder.h"

#include <utility>

#include "experimental/rust_bmp/ffi/FFI.rs.h"
#include "experimental/rust_bmp/decoder/impl/SkBmpRustCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/core/SkStreamPriv.h"

namespace SkBmpRustDecoder {

bool IsBmp(const void* buff, size_t bytesRead) {
    const rust::Slice<const uint8_t> data_slice{
        static_cast<const uint8_t*>(buff),
        bytesRead
    };

    return rust_bmp::is_bmp_data(data_slice);
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    return SkBmpRustCodec::MakeFromStream(std::move(stream), result);
}

std::unique_ptr<SkCodec> Decode(sk_sp<const SkData> data,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    return Decode(SkMemoryStream::Make(std::move(data)), result);
}

}  // namespace SkBmpRustDecoder
