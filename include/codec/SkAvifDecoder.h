/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkAvifDecoder_DEFINED
#define SkAvifDecoder_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SkData;
class SkStream;

#include <memory>

namespace SkAvifDecoder {

namespace LibAvif {

/** Returns true if this data claims to be an AVIF image. */
SK_API bool IsAvif(const void*, size_t);

/**
 *  Attempts to decode the given bytes as an AVIF.
 *
 *  If the bytes are not an AVIF, returns nullptr.
 *
 *  DecodeContext is ignored
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<SkData>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline constexpr SkCodecs::Decoder Decoder() {
    return { "avif", IsAvif, Decode };
}

}  // namespace LibAvif

// This function is in the root SkAvifDecoder namespace. It simply routes
// everything through the LibAvif namespace and exists for backwards
// compatibility.
inline constexpr SkCodecs::Decoder Decoder() {
    return { "avif", LibAvif::IsAvif, LibAvif::Decode };
}

namespace CrabbyAvif {

/** Returns true if this data claims to be an AVIF image. */
SK_API bool IsAvif(const void*, size_t);

/**
 *  Attempts to decode the given bytes as an AVIF.
 *
 *  If the bytes are not an AVIF, returns nullptr.
 *
 *  DecodeContext is ignored
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<SkData>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline constexpr SkCodecs::Decoder Decoder() {
    return { "avif", IsAvif, Decode };
}

}  // namespace CrabbyAvif

}  // namespace SkAvifDecoder

#endif  // SkAvifDecoder_DEFINED
