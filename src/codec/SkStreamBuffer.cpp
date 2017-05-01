/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStreamBuffer.h"

SkStreamBuffer::SkStreamBuffer(SkStream* stream, size_t bufferSize)
    : fStream(stream)
    , fBufferSize(bufferSize)
    , fBuffer(new char[bufferSize])
    , fPosition(0)
    , fBytesBuffered(0)
    , fHasLengthAndPosition(stream->hasLength() && stream->hasPosition())
    , fTrulyBuffered(0)
{}

SkStreamBuffer::~SkStreamBuffer() {
    fMarkedData.foreach([](size_t, SkData** data) { (*data)->unref(); });
}

const char* SkStreamBuffer::get() const {
    SkASSERT(fBytesBuffered >= 1);
    if (fHasLengthAndPosition && fTrulyBuffered < fBytesBuffered) {
        const size_t bytesToBuffer = fBytesBuffered - fTrulyBuffered;
        char* dst = SkTAddOffset<char>(const_cast<char*>(fBuffer.get()), fTrulyBuffered);
        SkDEBUGCODE(const size_t bytesRead =)
        // This stream is rewindable, so it should be safe to call the non-const
        // read()
        const_cast<SkStream*>(fStream.get())->read(dst, bytesToBuffer);
        SkASSERT(bytesRead == bytesToBuffer);
        fTrulyBuffered = fBytesBuffered;
    }
    return fBuffer.get();
}

bool SkStreamBuffer::buffer(size_t totalBytesToBuffer) {
    // Client should have allocated with a larger size if they needed a larger
    // buffer.
    SkASSERT(totalBytesToBuffer <= fBufferSize);

    if (totalBytesToBuffer <= fBytesBuffered) {
        return true;
    }

    if (fHasLengthAndPosition) {
        const size_t remaining = fStream->getLength() - fStream->getPosition() + fTrulyBuffered;
        fBytesBuffered = SkTMin(remaining, totalBytesToBuffer);
    } else {
        const size_t extraBytes = totalBytesToBuffer - fBytesBuffered;
        char* dst = SkTAddOffset<char>(fBuffer.get(), fBytesBuffered);
        const size_t bytesBuffered = fStream->read(dst, extraBytes);
        fBytesBuffered += bytesBuffered;
    }
    return fBytesBuffered == totalBytesToBuffer;
}

size_t SkStreamBuffer::markPosition() {
    SkASSERT(fBytesBuffered >= 1);
    if (!fHasLengthAndPosition) {
        sk_sp<SkData> data(SkData::MakeWithCopy(fBuffer.get(), fBytesBuffered));
        SkASSERT(nullptr == fMarkedData.find(fPosition));
        fMarkedData.set(fPosition, data.release());
    }
    return fPosition;
}

sk_sp<SkData> SkStreamBuffer::getDataAtPosition(size_t position, size_t length) {
    if (!fHasLengthAndPosition) {
        SkData** data = fMarkedData.find(position);
        SkASSERT(data);
        SkASSERT((*data)->size() == length);
        return sk_ref_sp<SkData>(*data);
    }

    SkASSERT(position + length <= fStream->getLength());

    const size_t oldPosition = fStream->getPosition();
    if (!fStream->seek(position)) {
        return nullptr;
    }

    sk_sp<SkData> data(SkData::MakeUninitialized(length));
    void* dst = data->writable_data();
    const bool success = fStream->read(dst, length) == length;
    fStream->seek(oldPosition);
    return success ? data : nullptr;
}
