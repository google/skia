/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/GrSlug.h"

#include "include/core/SkCanvas.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <atomic>

GrTextReferenceFrame::~GrTextReferenceFrame() = default;

GrSlug::~GrSlug() = default;
sk_sp<GrSlug> GrSlug::ConvertBlob(
        SkCanvas* canvas, const SkTextBlob& blob, SkPoint origin, const SkPaint& paint) {
    return canvas->convertBlobToSlug(blob, origin, paint);
}

sk_sp<SkData> GrSlug::serialize() const {
    SkBinaryWriteBuffer buffer;
    this->doFlatten(buffer);
    return buffer.snapshotAsData();
}

size_t GrSlug::serialize(void* buffer, size_t size) const {
    SkBinaryWriteBuffer writeBuffer{buffer, size};
    this->doFlatten(writeBuffer);

    // If we overflow the given buffer, then SkWriteBuffer allocates a new larger buffer. Check
    // to see if an additional buffer was allocated, if it wasn't then everything fit, else
    // return 0 signaling the buffer overflowed.
    // N.B. This is the idiom from SkTextBlob.
    return writeBuffer.usingInitialStorage() ? writeBuffer.bytesWritten() : 0u;
}

sk_sp<GrSlug> SkMakeSlugFromBuffer(SkReadBuffer& buffer, const SkStrikeClient* client);
sk_sp<GrSlug> GrSlug::MakeFromBuffer(SkReadBuffer& buffer) {
    return SkMakeSlugFromBuffer(buffer, nullptr);
}

sk_sp<GrSlug> GrSlug::Deserialize(const void* data, size_t size, const SkStrikeClient* client) {
    SkReadBuffer buffer{data, size};
    return SkMakeSlugFromBuffer(buffer, client);
}

void GrSlug::draw(SkCanvas* canvas) const {
    canvas->drawSlug(this);
}

uint32_t GrSlug::NextUniqueID() {
    static std::atomic<uint32_t> nextUnique = 1;
    return nextUnique++;
}

// Most of GrSlug's implementation is in GrTextBlob.cpp to share common code.



