/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFrontBufferedStream.h"

SkStreamRewindable* SkFrontBufferedStream::Create(SkStream* stream, size_t bufferSize) {
    if (NULL == stream) {
        return NULL;
    }
    return SkNEW_ARGS(SkFrontBufferedStream, (stream, bufferSize));
}

SkFrontBufferedStream::SkFrontBufferedStream(SkStream* stream, size_t bufferSize)
    : fStream(SkRef(stream))
    , fOffset(0)
    , fBufferedSoFar(0)
    , fBufferSize(bufferSize)
    , fBuffer(bufferSize) {}

bool SkFrontBufferedStream::isAtEnd() const {
    if (fOffset < fBufferedSoFar) {
        // Even if the underlying stream is at the end, this stream has been
        // rewound after buffering, so it is not at the end.
        return false;
    }

    return fStream->isAtEnd();
}

bool SkFrontBufferedStream::rewind() {
    // Only allow a rewind if we have not exceeded the buffer.
    if (fOffset <= fBufferSize) {
        fOffset = 0;
        return true;
    }
    return false;
}

bool SkFrontBufferedStream::hasLength() const {
    return fStream->hasLength();
}

size_t SkFrontBufferedStream::getLength() const {
    return fStream->getLength();
}

size_t SkFrontBufferedStream::readFromBuffer(char* dst, size_t size) {
    SkASSERT(fOffset < fBufferedSoFar);
    // Some data has already been copied to fBuffer. Read up to the
    // lesser of the size requested and the remainder of the buffered
    // data.
    const size_t bytesToCopy = SkTMin(size, fBufferedSoFar - fOffset);
    if (dst != NULL) {
        memcpy(dst, fBuffer + fOffset, bytesToCopy);
    }

    // Update fOffset to the new position. It is guaranteed to be
    // within the buffered data.
    fOffset += bytesToCopy;
    SkASSERT(fOffset <= fBufferedSoFar);

    return bytesToCopy;
}

size_t SkFrontBufferedStream::bufferAndWriteTo(char* dst, size_t size) {
    SkASSERT(size > 0);
    SkASSERT(fOffset >= fBufferedSoFar);
    // Data needs to be buffered. Buffer up to the lesser of the size requested
    // and the remainder of the max buffer size.
    const size_t bytesToBuffer = SkTMin(size, fBufferSize - fBufferedSoFar);
    char* buffer = fBuffer + fOffset;
    const size_t buffered = fStream->read(buffer, bytesToBuffer);

    fBufferedSoFar += buffered;
    fOffset = fBufferedSoFar;
    SkASSERT(fBufferedSoFar <= fBufferSize);

    // Copy the buffer to the destination buffer and update the amount read.
    if (dst != NULL) {
        memcpy(dst, buffer, buffered);
    }

    return buffered;
}

size_t SkFrontBufferedStream::readDirectlyFromStream(char* dst, size_t size) {
    SkASSERT(size > 0);
    // If we get here, we have buffered all that can be buffered.
    SkASSERT(fBufferSize == fBufferedSoFar && fOffset >= fBufferSize);

    const size_t bytesReadDirectly = fStream->read(dst, size);
    fOffset += bytesReadDirectly;

    // If we have read past the end of the buffer, rewinding is no longer
    // supported, so we can go ahead and free the memory.
    if (bytesReadDirectly > 0) {
        fBuffer.reset(0);
    }

    return bytesReadDirectly;
}

size_t SkFrontBufferedStream::read(void* voidDst, size_t size) {
    // Cast voidDst to a char* for easy addition.
    char* dst = reinterpret_cast<char*>(voidDst);
    SkDEBUGCODE(const size_t totalSize = size;)
    const size_t start = fOffset;

    // First, read any data that was previously buffered.
    if (fOffset < fBufferedSoFar) {
        const size_t bytesCopied = this->readFromBuffer(dst, size);

        // Update the remaining number of bytes needed to read
        // and the destination buffer.
        size -= bytesCopied;
        SkASSERT(size + (fOffset - start) == totalSize);
        if (dst != NULL) {
            dst += bytesCopied;
        }
    }

    // Buffer any more data that should be buffered, and copy it to the
    // destination.
    if (size > 0 && fBufferedSoFar < fBufferSize) {
        const size_t buffered = this->bufferAndWriteTo(dst, size);

        // Update the remaining number of bytes needed to read
        // and the destination buffer.
        size -= buffered;
        SkASSERT(size + (fOffset - start) == totalSize);
        if (dst != NULL) {
            dst += buffered;
        }
    }

    if (size > 0 && !fStream->isAtEnd()) {
        SkDEBUGCODE(const size_t bytesReadDirectly =) this->readDirectlyFromStream(dst, size);
        SkDEBUGCODE(size -= bytesReadDirectly;)
        SkASSERT(size + (fOffset - start) == totalSize);
    }

    return fOffset - start;
}
