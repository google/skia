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
    , fHasLengthAndPosition(stream->hasLength() && stream->hasPosition())
    , fTrulyBuffered(0)
{}

const char* SkStreamBuffer::get() const {
    SkASSERT(fBytesBuffered >= 1);
    if (fHasLengthAndPosition && fTrulyBuffered < fBytesBuffered) {
        const size_t bytesToBuffer = fBytesBuffered - fTrulyBuffered;
        char* dst = SkTAddOffset<char>(const_cast<char*>(fBuffer), fTrulyBuffered);
        SkDEBUGCODE(const size_t bytesRead =)
        // This stream is rewindable, so it should be safe to call the non-const
        // read()
        const_cast<SkStream*>(fStream.get())->read(dst, bytesToBuffer);
        SkASSERT(bytesRead == bytesToBuffer);
        fTrulyBuffered = fBytesBuffered;
    }
    return fBuffer;
}

bool SkStreamBuffer::buffer(size_t totalBytesToBuffer) {
    // FIXME (scroggo): What should we do if the client tries to read too much?
    // Should not be a problem in GIF.
    SkASSERT(totalBytesToBuffer <= kMaxSize);

    if (totalBytesToBuffer <= fBytesBuffered) {
        return true;
    }

    if (fHasLengthAndPosition) {
        const size_t remaining = fStream->getLength() - fStream->getPosition() + fTrulyBuffered;
        fBytesBuffered = SkTMin(remaining, totalBytesToBuffer);
    } else {
        const size_t extraBytes = totalBytesToBuffer - fBytesBuffered;
        const size_t bytesBuffered = fStream->read(fBuffer + fBytesBuffered, extraBytes);
        fBytesBuffered += bytesBuffered;
    }
    return fBytesBuffered == totalBytesToBuffer;
}

bool SkStreamBuffer::readFromPosition(void* dst, size_t offset, size_t length) {
    SkASSERT(fHasLengthAndPosition);

    if (offset + length > fStream->getLength()) {
        return false;
    }

    const size_t oldPosition = fStream->getPosition();
    if (!fStream->seek(offset)) {
        return false;
    }

    const bool success = fStream->read(dst, length) == length;
    fStream->seek(oldPosition);
    return success;
}

size_t SkStreamBuffer::getPosition() const {
    SkASSERT(fHasLengthAndPosition);
    return fStream->getPosition() - fTrulyBuffered;
}
