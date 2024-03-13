/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/chromium/Slug.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSerialProcs.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

class SkData;

namespace sktext::gpu {

sk_sp<Slug> Slug::ConvertBlob(
        SkCanvas* canvas, const SkTextBlob& blob, SkPoint origin, const SkPaint& paint) {
    return canvas->convertBlobToSlug(blob, origin, paint);
}

sk_sp<SkData> Slug::serialize() const {
    SkBinaryWriteBuffer buffer({});
    this->doFlatten(buffer);
    return buffer.snapshotAsData();
}

size_t Slug::serialize(void* buffer, size_t size) const {
    SkBinaryWriteBuffer writeBuffer{buffer, size, {}};
    this->doFlatten(writeBuffer);

    // If we overflow the given buffer, then SkWriteBuffer allocates a new larger buffer. Check
    // to see if an additional buffer was allocated, if it wasn't then everything fit, else
    // return 0 signaling the buffer overflowed.
    // N.B. This is the idiom from SkTextBlob.
    return writeBuffer.usingInitialStorage() ? writeBuffer.bytesWritten() : 0u;
}

sk_sp<Slug> Slug::Deserialize(const void* data, size_t size, const SkStrikeClient* client) {
    SkReadBuffer buffer{data, size};
    SkDeserialProcs procs;
    Slug::AddDeserialProcs(&procs, client);
    buffer.setDeserialProcs(procs);
    return MakeFromBuffer(buffer);
}

void Slug::draw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawSlug(this, paint);
}

}  // namespace sktext::gpu

