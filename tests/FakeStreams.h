/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkStream.h"

#ifndef FakeStreams_DEFINED
#define FakeStreams_DEFINED

// Stream that is not an asset stream (!hasPosition() or !hasLength())
class NotAssetMemStream : public SkStream {
public:
    NotAssetMemStream(sk_sp<SkData> data) : fStream(std::move(data)) {}

    bool hasPosition() const override {
        return false;
    }

    bool hasLength() const override {
        return false;
    }

    size_t peek(void* buf, size_t bytes) const override {
        return fStream.peek(buf, bytes);
    }
    size_t read(void* buf, size_t bytes) override {
        return fStream.read(buf, bytes);
    }
    bool rewind() override {
        return fStream.rewind();
    }
    bool isAtEnd() const override {
        return fStream.isAtEnd();
    }
private:
    SkMemoryStream fStream;
};

/*
 *  Represents a stream without all of its data.
 */
class HaltingStream : public SkStream {
public:
    HaltingStream(sk_sp<SkData> data, size_t initialLimit)
        : fTotalSize(data->size())
        , fLimit(initialLimit)
        , fStream(std::move(data))
    {}

    void addNewData(size_t extra) {
        fLimit = SkTMin(fTotalSize, fLimit + extra);
    }

    size_t read(void* buffer, size_t size) override {
        if (fStream.getPosition() + size > fLimit) {
            size = fLimit - fStream.getPosition();
        }

        return fStream.read(buffer, size);
    }

    bool isAtEnd() const override {
        return fStream.isAtEnd();
    }

    bool hasLength() const override { return true; }
    size_t getLength() const override { return fLimit; }

    bool hasPosition() const override { return true; }
    size_t getPosition() const override { return fStream.getPosition(); }
    bool rewind() override { return fStream.rewind(); }
    bool move(long offset) override { return fStream.move(offset); }
    bool seek(size_t position) override { return fStream.seek(position); }

    bool isAllDataReceived() const { return fLimit == fTotalSize; }

private:
    const size_t    fTotalSize;
    size_t          fLimit;
    SkMemoryStream  fStream;
};
#endif // FakeStreams_DEFINED
