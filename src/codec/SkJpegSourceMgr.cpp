/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegSourceMgr.h"

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/codec/SkCodecPriv.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegMemorySourceMgr

class SkJpegMemorySourceMgr : public SkJpegSourceMgr {
public:
    SkJpegMemorySourceMgr(SkStream* stream) : fStream(stream) {}
    ~SkJpegMemorySourceMgr() override {}

    void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        nextInputByte = reinterpret_cast<const uint8_t*>(fStream->getMemoryBase());
        bytesInBuffer = static_cast<size_t>(fStream->getLength());
    }
    bool fillInputBuffer(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        // The whole JPEG data is expected to reside in the supplied memory buffer, so any request
        // for more data beyond the given buffer size is treated as an error.
        SkCodecPrintf("Asked to re-fill a memory-mapped stream.\n");
        return false;
    }
    bool skipInputBytes(size_t bytesToSkip,
                        const uint8_t*& nextInputByte,
                        size_t& bytesInBuffer) override {
        if (bytesToSkip > bytesInBuffer) {
            SkCodecPrintf("Asked to read past end of a memory-mapped stream.\n");
            return false;
        }
        nextInputByte += bytesToSkip;
        bytesInBuffer -= bytesToSkip;
        return true;
    }

private:
    SkStream* const fStream;  // unowned.
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegBufferedSourceMgr

class SkJpegBufferedSourceMgr : public SkJpegSourceMgr {
public:
    SkJpegBufferedSourceMgr(SkStream* stream) : fStream(stream) {}
    ~SkJpegBufferedSourceMgr() override {}

    void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        constexpr size_t kBufferSize = 1024;
        fBuffer = SkData::MakeUninitialized(kBufferSize);
        nextInputByte = fBuffer->bytes();
        bytesInBuffer = 0;
    }
    bool fillInputBuffer(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        size_t bytesRead = fStream->read(fBuffer->writable_data(), fBuffer->size());
        if (bytesRead == 0) {
            // Fail if we read zero bytes (libjpeg will accept any non-zero number of bytes).
            SkCodecPrintf("Hit end of file reading a buffered stream.\n");
            return false;
        }
        nextInputByte = fBuffer->bytes();
        bytesInBuffer = bytesRead;
        return true;
    }
    bool skipInputBytes(size_t bytesToSkip,
                        const uint8_t*& nextInputByte,
                        size_t& bytesInBuffer) override {
        // Skip through the already-read (or already in memory) buffer.
        if (bytesToSkip <= bytesInBuffer) {
            nextInputByte += bytesToSkip;
            bytesInBuffer -= bytesToSkip;
            return true;
        }
        bytesToSkip -= bytesInBuffer;

        // Fail if we skip past the end of the stream.
        if (fStream->skip(bytesToSkip) != bytesToSkip) {
            SkCodecPrintf("Failed to skip through buffered stream.\n");
            return false;
        }

        bytesInBuffer = 0;
        nextInputByte = fBuffer->bytes();
        return true;
    }

private:
    SkStream* const fStream;  // unowned.
    sk_sp<SkData> fBuffer;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegSourceMgr

// static
std::unique_ptr<SkJpegSourceMgr> SkJpegSourceMgr::Make(SkStream* stream) {
    if (stream->hasLength() && stream->getMemoryBase()) {
        return std::make_unique<SkJpegMemorySourceMgr>(stream);
    }
    return std::make_unique<SkJpegBufferedSourceMgr>(stream);
}
