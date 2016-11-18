/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStreamBuffer.h"

SkStreamBuffer::SkStreamBuffer(SkStream* stream)
    : fStream(stream)
    , fBytesBuffered(0)
{}

size_t SkStreamBuffer::buffer(size_t bytesToBuffer) {
    // FIXME (scroggo): What should we do if the client tries to read too much?
    // Should not be a problem in GIF.
    SkASSERT(fBytesBuffered + bytesToBuffer <= kMaxSize);

    const size_t bytesBuffered = fStream->read(fBuffer + fBytesBuffered, bytesToBuffer);
    fBytesBuffered += bytesBuffered;
    return bytesBuffered;
}
