/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/decoder/SkJpegRustDecoder.h"

#include <utility>

#include "experimental/rust_jpeg/ffi/FFI.rs.h"
#include "experimental/rust_jpeg/decoder/impl/SkJpegRustCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/core/SkStreamPriv.h"

namespace SkJpegRustDecoder {

bool IsJpeg(const void* buff, size_t bytesRead) {
    const rust::Slice<const uint8_t> data_slice{
        static_cast<const uint8_t*>(buff),
        bytesRead
    };

    return rust_jpeg::is_jpeg_data(data_slice);
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    return SkJpegRustCodec::MakeFromStream(std::move(stream), result);
}

std::unique_ptr<SkCodec> Decode(sk_sp<const SkData> data,
                                SkCodec::Result* result,
                                SkCodecs::DecodeContext) {
    return Decode(SkMemoryStream::Make(std::move(data)), result);
}

}  // namespace SkJpegRustDecoder
