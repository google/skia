/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkHeifDecoder_DEFINED
#define SkHeifDecoder_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkRefCnt.h"

class SkData;
class SkStream;

#include <memory>

namespace SkHeifDecoder {

/** Returns true if this data claims to be a HEIF image. */
SK_API bool IsHeif(const void*, size_t);

/**
 *  Attempts to decode the given bytes as a HEIF.
 *
 *  If the bytes are not a HEIF, returns nullptr.
 *
 *  DecodeContext is treated as a SkCodec::SelectionPolicy*
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<SkData>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline SkCodecs::Decoder Decoder() {
    return { "heif", IsHeif, Decode };
}

}  // namespace SkHeifDecoder

#endif  // SkHeifDecoder_DEFINED
