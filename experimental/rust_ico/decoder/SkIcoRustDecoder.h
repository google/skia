/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkIcoRustDecoder_DEFINED
#define SkIcoRustDecoder_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"

class SkData;
class SkStream;

#include <memory>
#include <optional>

namespace SkIcoRustDecoder {

/** Returns true if this data claims to be an ICO or CUR image. */
SK_API bool IsIco(const void*, size_t);

/**
 *  Attempts to decode the given bytes as an ICO using Rust-based
 *  PNG and BMP decoders for the embedded images.
 *
 *  If the bytes are not an ICO, returns nullptr.
 *
 *  DecodeContext is ignored.
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<const SkData>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

/**
 * Returns the raw, unclamped hotspot from a CUR directory entry, in reported
 * frame coordinates.
 *
 * codec must have been created by SkIcoRustDecoder::Decode().
 */
SK_API bool GetHotSpot(const SkCodec* codec, int frameIndex, SkIPoint* hotSpot);

struct DefaultImageInfo {
    SkISize dimensions;
    std::optional<SkIPoint> hotSpot;
    bool hasCompatibleFrame;
};

/**
 * Describes the largest directory entry, preferring higher bit depth on ties.
 * These display dimensions can differ from SkCodec::dimensions(), which bound
 * all entries. hotSpot is present only for cursors. hasCompatibleFrame indicates
 * whether a currently available representation fits the default dimensions.
 * Returns std::nullopt for a null codec.
 *
 * codec must have been created by SkIcoRustDecoder::Decode().
 */
SK_API std::optional<DefaultImageInfo> GetDefaultImageInfo(const SkCodec* codec);

inline constexpr SkCodecs::Decoder Decoder() { return {"ico", IsIco, Decode}; }

}  // namespace SkIcoRustDecoder

#endif  // SkIcoRustDecoder_DEFINED
