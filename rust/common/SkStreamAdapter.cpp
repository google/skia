// Copyright 2024 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Common implementation of SkStreamAdapter for Rust codec FFI.
// This is a reusable component that wraps SkStream for Rust Read + Seek traits.

#include "rust/common/SkStreamAdapter.h"

#include <limits>

#include "include/private/base/SkAssert.h"
#include "rust/common/SpanUtils.h"
#include "src/base/SkSafeMath.h"
#include "third_party/rust/cxx/v1/cxx.h"

namespace rust {
namespace stream {
size_t SkStreamAdapter::read(rust::Slice<uint8_t> buffer) {
    SkASSERT_RELEASE(fStream);
    SkSpan<uint8_t> span = ToSkSpan(buffer);
    return fStream->read(span.data(), span.size());
}

bool SkStreamAdapter::seek_from_start(uint64_t requestedPos, uint64_t& finalPos) {
    SkASSERT_RELEASE(fStream);

    SkSafeMath safe;
    size_t pos = safe.castTo<size_t>(requestedPos);
    if (!safe.ok()) {
        return false;
    }

    if (!fStream->seek(pos)) {
        return false;
    }
    SkASSERT_RELEASE(!fStream->hasPosition() || fStream->getPosition() == requestedPos);

    // Assigning `size_t` to `uint64_t` doesn't need to go through
    // `SkSafeMath`, because `uint64_t` is never smaller than `size_t`.
    static_assert(sizeof(uint64_t) >= sizeof(size_t));
    finalPos = requestedPos;

    return true;
}

bool SkStreamAdapter::seek_from_end(int64_t requestedOffset, uint64_t& finalPos) {
    SkASSERT_RELEASE(fStream);

    if (!fStream->hasLength()) {
        return false;
    }
    size_t length = fStream->getLength();

    SkSafeMath safe;
    uint64_t endPos = safe.castTo<uint64_t>(length);
    if (requestedOffset > 0) {
        // IIUC `SkStream` doesn't support reading beyond the current length.
        return false;
    }
    if (requestedOffset == std::numeric_limits<int64_t>::min()) {
        // `-requestedOffset` below wouldn't work.
        return false;
    }
    uint64_t offset = safe.castTo<uint64_t>(-requestedOffset);
    if (!safe.ok()) {
        return false;
    }
    if (offset > endPos) {
        // `endPos - offset` below wouldn't work.
        return false;
    }

    return this->seek_from_start(endPos - offset, finalPos);
}

bool SkStreamAdapter::seek_relative(int64_t requestedOffset, uint64_t& finalPos) {
    SkASSERT_RELEASE(fStream);

    if (!fStream->hasPosition()) {
        return false;
    }

    SkSafeMath safe;
    long offset = safe.castTo<long>(requestedOffset);
    if (!safe.ok()) {
        return false;
    }

    // Only move the stream if the offset is non-zero, as zero offset should be no-op.
    if (offset != 0 && !fStream->move(offset)) {
        return false;
    }

    finalPos = safe.castTo<uint64_t>(fStream->getPosition());
    if (!safe.ok()) {
        return false;
    }
    return true;
}

}  // namespace stream
}  // namespace rust
