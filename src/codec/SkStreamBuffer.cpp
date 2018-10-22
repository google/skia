/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStreamBuffer.h"

SkStreamBuffer::SkStreamBuffer(std::unique_ptr<SkStream> stream)
    : fStream(std::move(stream))
    , fPosition(0)
    , fBytesBuffered(0)
    , fHasLengthAndPosition(fStream->hasLength() && fStream->hasPosition())
    , fTrulyBuffered(0)
{}

SkStreamBuffer::~SkStreamBuffer() {
    fMarkedData.foreach([](size_t, SkData** data) { (*data)->unref(); });
}

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

size_t SkStreamBuffer::markPosition() {
    SkASSERT(fBytesBuffered >= 1);
    if (!fHasLengthAndPosition) {
        sk_sp<SkData> data(SkData::MakeWithCopy(fBuffer, fBytesBuffered));
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

    SkASSERT(length <= fStream->getLength() &&
             position <= fStream->getLength() - length);

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
