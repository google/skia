/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/private/SkTemplates.h"
#include "include/utils/SkFrontBufferedStream.h"

class FrontBufferedStream : public SkStreamRewindable {
public:
    // Called by Make.
    FrontBufferedStream(std::unique_ptr<SkStream>, size_t bufferSize);

    size_t read(void* buffer, size_t size) override;

    size_t peek(void* buffer, size_t size) const override;

    bool isAtEnd() const override;

    bool rewind() override;

    bool hasLength() const override { return fHasLength; }

    size_t getLength() const override { return fLength; }

private:
    SkStreamRewindable* onDuplicate() const override { return nullptr; }

    std::unique_ptr<SkStream> fStream;
    const bool                fHasLength;
    const size_t              fLength;
    // Current offset into the stream. Always >= 0.
    size_t                    fOffset;
    // Amount that has been buffered by calls to read. Will always be less than
    // fBufferSize.
    size_t                    fBufferedSoFar;
    // Total size of the buffer.
    const size_t              fBufferSize;
    // FIXME: SkAutoTMalloc throws on failure. Instead, Create should return a
    // nullptr stream.
    SkAutoTMalloc<char>       fBuffer;

    // Read up to size bytes from already buffered data, and copy to
    // dst, if non-nullptr. Updates fOffset. Assumes that fOffset is less
    // than fBufferedSoFar.
    size_t readFromBuffer(char* dst, size_t size);

    // Buffer up to size bytes from the stream, and copy to dst if non-
    // nullptr. Updates fOffset and fBufferedSoFar. Assumes that fOffset is
    // less than fBufferedSoFar, and size is greater than 0.
    size_t bufferAndWriteTo(char* dst, size_t size);

    // Read up to size bytes directly from the stream and into dst if non-
    // nullptr. Updates fOffset. Assumes fOffset is at or beyond the buffered
    // data, and size is greater than 0.
    size_t readDirectlyFromStream(char* dst, size_t size);

    typedef SkStream INHERITED;
};

std::unique_ptr<SkStreamRewindable> SkFrontBufferedStream::Make(std::unique_ptr<SkStream> stream,
                                                                size_t bufferSize) {
    if (!stream) {
        return nullptr;
    }
    return std::unique_ptr<SkStreamRewindable>(new FrontBufferedStream(std::move(stream),
                                                                       bufferSize));
}

FrontBufferedStream::FrontBufferedStream(std::unique_ptr<SkStream> stream, size_t bufferSize)
    : fStream(std::move(stream))
    , fHasLength(fStream->hasPosition() && fStream->hasLength())
    , fLength(fStream->getLength() - fStream->getPosition())
    , fOffset(0)
    , fBufferedSoFar(0)
    , fBufferSize(bufferSize)
    , fBuffer(bufferSize) {}

bool FrontBufferedStream::isAtEnd() const {
    if (fOffset < fBufferedSoFar) {
        // Even if the underlying stream is at the end, this stream has been
        // rewound after buffering, so it is not at the end.
        return false;
    }

    return fStream->isAtEnd();
}

bool FrontBufferedStream::rewind() {
    // Only allow a rewind if we have not exceeded the buffer.
    if (fOffset <= fBufferSize) {
        fOffset = 0;
        return true;
    }
    return false;
}

size_t FrontBufferedStream::readFromBuffer(char* dst, size_t size) {
    SkASSERT(fOffset < fBufferedSoFar);
    // Some data has already been copied to fBuffer. Read up to the
    // lesser of the size requested and the remainder of the buffered
    // data.
    const size_t bytesToCopy = SkTMin(size, fBufferedSoFar - fOffset);
    if (dst != nullptr) {
        memcpy(dst, fBuffer + fOffset, bytesToCopy);
    }

    // Update fOffset to the new position. It is guaranteed to be
    // within the buffered data.
    fOffset += bytesToCopy;
    SkASSERT(fOffset <= fBufferedSoFar);

    return bytesToCopy;
}

size_t FrontBufferedStream::bufferAndWriteTo(char* dst, size_t size) {
    SkASSERT(size > 0);
    SkASSERT(fOffset >= fBufferedSoFar);
    SkASSERT(fBuffer);
    // Data needs to be buffered. Buffer up to the lesser of the size requested
    // and the remainder of the max buffer size.
    const size_t bytesToBuffer = SkTMin(size, fBufferSize - fBufferedSoFar);
    char* buffer = fBuffer + fOffset;
    const size_t buffered = fStream->read(buffer, bytesToBuffer);

    fBufferedSoFar += buffered;
    fOffset = fBufferedSoFar;
    SkASSERT(fBufferedSoFar <= fBufferSize);

    // Copy the buffer to the destination buffer and update the amount read.
    if (dst != nullptr) {
        memcpy(dst, buffer, buffered);
    }

    return buffered;
}

size_t FrontBufferedStream::readDirectlyFromStream(char* dst, size_t size) {
    SkASSERT(size > 0);
    // If we get here, we have buffered all that can be buffered.
    SkASSERT(fBufferSize == fBufferedSoFar && fOffset >= fBufferSize);

    const size_t bytesReadDirectly = fStream->read(dst, size);
    fOffset += bytesReadDirectly;

    // If we have read past the end of the buffer, rewinding is no longer
    // supported, so we can go ahead and free the memory.
    if (bytesReadDirectly > 0) {
        sk_free(fBuffer.release());
    }

    return bytesReadDirectly;
}

size_t FrontBufferedStream::peek(void* dst, size_t size) const {
    // Keep track of the offset so we can return to it.
    const size_t start = fOffset;

    if (start >= fBufferSize) {
        // This stream is not able to buffer.
        return 0;
    }

    size = SkTMin(size, fBufferSize - start);
    FrontBufferedStream* nonConstThis = const_cast<FrontBufferedStream*>(this);
    const size_t bytesRead = nonConstThis->read(dst, size);
    nonConstThis->fOffset = start;
    return bytesRead;
}

size_t FrontBufferedStream::read(void* voidDst, size_t size) {
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
        if (dst != nullptr) {
            dst += bytesCopied;
        }
    }

    // Buffer any more data that should be buffered, and copy it to the
    // destination.
    if (size > 0 && fBufferedSoFar < fBufferSize && !fStream->isAtEnd()) {
        const size_t buffered = this->bufferAndWriteTo(dst, size);

        // Update the remaining number of bytes needed to read
        // and the destination buffer.
        size -= buffered;
        SkASSERT(size + (fOffset - start) == totalSize);
        if (dst != nullptr) {
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
