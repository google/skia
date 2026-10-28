/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkJpegRustDecoder_DEFINED
#define SkJpegRustDecoder_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/SkAPI.h"

class SkData;
class SkStream;

#include <memory>

namespace SkJpegRustDecoder {

enum class InputState { kGrowing, kComplete };

struct Metadata {
    /** Encoded maximum sampling factors times eight, independent of output scale. */
    SkISize fMCUSize = SkISize::MakeEmpty();
};

/** Returns true if this data claims to be a JPEG image. */
SK_API bool IsJpeg(const void*, size_t);

/**
 *  Attempts to decode the given bytes as a JPEG using Rust implementation.
 *
 *  If the bytes are not a JPEG, returns nullptr.
 *  Unannotated streams use kGrowing for incremental, scanline, and raw decoding.
 *  Full-frame getPixels retains legacy non-strict best-effort decoding.
 *  All eighth-step reduced sizes support scanline decoding through buffered
 *  frame reconstruction, charged against the decode memory budget.
 *  Full-size scanline decoding retains its streaming implementation.
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);
/**
 *  InputState::kComplete promises that no more encoded bytes can arrive, including
 *  when the stream has no EOI marker. Stream length or temporary EOF alone does
 *  not make this promise. Input state is immutable, including across rewind.
 *  To finalize incremental or raw decoding of a growing source without EOI,
 *  construct a new complete-input codec and restart decoding.
 *  Full-frame getPixels is best-effort independently of this state.
 *  Incremental decoding can still report incomplete
 *  input and publish only the current buffered scan's valid row prefix.
 *  Metadata is written only on successful creation
 *  and describes source MCU geometry, independently of raw YUV eligibility.
 */
SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>,
                                       SkCodec::Result*,
                                       InputState,
                                       Metadata* = nullptr);

/** Immutable data is final input; non-strict decoding can render truncated scans. */
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<const SkData>,
                                       SkCodec::Result*,
                                       SkCodecs::DecodeContext = nullptr);

inline constexpr SkCodecs::Decoder Decoder() {
    return { "rust_jpeg", IsJpeg, Decode };
}

}  // namespace SkJpegRustDecoder

#endif  // SkJpegRustDecoder_DEFINED
