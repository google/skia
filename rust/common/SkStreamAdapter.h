// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SkStreamAdapter_DEFINED
#define SkStreamAdapter_DEFINED

#include "include/core/SkStream.h"
#include "include/private/base/SkAssert.h"
#include "rust/common/SpanUtils.h"

namespace rust {
namespace stream {

// Adapter class for reading and seeking Skia SkStreams from Rust.
// The caller needs to guarantee that `stream` will be alive for at least as
// long as the `SkStreamAdapter`.
class SkStreamAdapter {
public:
    // The caller needs to guarantee that `stream` will be alive for at least as
    // long as the `StreamAdapter`.
    explicit SkStreamAdapter(SkStream* stream) : fStream(stream) {
        SkASSERT_RELEASE(fStream);
    }

    ~SkStreamAdapter() = default;

    // Non-copyable and non-movable (we want a stable `this` pointer)
    SkStreamAdapter(const SkStreamAdapter&) = delete;
    SkStreamAdapter& operator=(const SkStreamAdapter&) = delete;
    SkStreamAdapter(SkStreamAdapter&&) = delete;
    SkStreamAdapter& operator=(SkStreamAdapter&&) = delete;

    // Read from stream into provided buffer. Returns number of bytes actually read.
    size_t read(rust::Slice<uint8_t> buffer);

    // Seek operations. Return true on success and set finalPos to actual position.
    bool seek_from_start(uint64_t requestedPos, uint64_t& finalPos);
    bool seek_from_end(int64_t requestedOffset, uint64_t& finalPos);
    bool seek_relative(int64_t requestedOffset, uint64_t& finalPos);

private:
    // TODO: Consider avoiding holding a raw / unowned pointer here:
    // * Replace `fStream` field with an abstract method declaration:
    //   `virtual SkStream& stream() = 0`.
    // * Remove the constructor.
    // * Derive-from/implement this in a child class in `src/codec/SkPngRustCodec.cpp`
    SkStream* fStream;
};

}  // namespace stream
}  // namespace rust

#endif  // SkStreamAdapter_DEFINED
