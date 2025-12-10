/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkBmpRustDecoder_DEFINED
#define SkBmpRustDecoder_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SkData;
class SkStream;

#include <memory>

namespace SkBmpRustDecoder {

/** Returns true if this data claims to be a BMP image. */
SK_API bool IsBmp(const void*, size_t);

/**
 *  Attempts to decode the given bytes as a BMP using Rust implementation.
 *
 *  If the bytes are not a BMP, returns nullptr.
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<const SkData>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline constexpr SkCodecs::Decoder Decoder() {
    return { "rust_bmp", IsBmp, Decode };
}

}  // namespace SkBmpRustDecoder

#endif  // SkBmpRustDecoder_DEFINED
