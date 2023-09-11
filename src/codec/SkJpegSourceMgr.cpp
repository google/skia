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
#include "include/core/SkTypes.h"
#include "src/codec/SkCodecPriv.h"

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegSegmentScan.h"
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkStream helpers.

/*
 * Class that will will rewind an SkStream, and then restore it to its original position when it
 * goes out of scope. If the SkStream is not seekable, then the stream will not be altered at all,
 * and will return false from canRestore.
 */

class ScopedSkStreamRestorer {
public:
    ScopedSkStreamRestorer(SkStream* stream) : fStream(stream), fPosition(stream->getPosition()) {
        if (!fStream->rewind()) {
            SkCodecPrintf("Failed to rewind decoder stream.\n");
        }
    }
    ~ScopedSkStreamRestorer() {
        if (!fStream->seek(fPosition)) {
            SkCodecPrintf("Failed to restore decoder stream.\n");
        }
    }

private:
    SkStream* const fStream;
    const size_t fPosition;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegMemorySourceMgr

class SkJpegMemorySourceMgr : public SkJpegSourceMgr {
public:
    SkJpegMemorySourceMgr(SkStream* stream) : SkJpegSourceMgr(stream) {}
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
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    const std::vector<SkJpegSegment>& getAllSegments() override {
        if (fScanner) {
            return fScanner->getSegments();
        }
        fScanner = std::make_unique<SkJpegSegmentScanner>(kJpegMarkerEndOfImage);
        fScanner->onBytes(fStream->getMemoryBase(), fStream->getLength());
        return fScanner->getSegments();
    }
    sk_sp<SkData> getSubsetData(size_t offset, size_t size, bool* wasCopied) override {
        if (offset > fStream->getLength() || size > fStream->getLength() - offset) {
            return nullptr;
        }
        if (wasCopied) {
            *wasCopied = false;
        }
        return SkData::MakeWithoutCopy(
                reinterpret_cast<const uint8_t*>(fStream->getMemoryBase()) + offset, size);
    }
    sk_sp<SkData> getSegmentParameters(const SkJpegSegment& segment) override {
        const uint8_t* base =
                reinterpret_cast<const uint8_t*>(fStream->getMemoryBase()) + segment.offset;
        SkASSERT(segment.offset < fStream->getLength());
        SkASSERT(kJpegMarkerCodeSize + segment.parameterLength <=
                 fStream->getLength() - segment.offset);

        // Read the marker and verify it matches `segment`.
        SkASSERT(base[0] == 0xFF);
        SkASSERT(base[1] == segment.marker);

        // Read the parameter length and verify it matches `segment`.
        SkASSERT(256 * base[2] + base[3] == segment.parameterLength);
        if (segment.parameterLength <= kJpegSegmentParameterLengthSize) {
            return nullptr;
        }

        // Read the remainder of the segment.
        return SkData::MakeWithoutCopy(base + kJpegMarkerCodeSize + kJpegSegmentParameterLengthSize,
                                       segment.parameterLength - kJpegSegmentParameterLengthSize);
    }
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegBufferedSourceMgr

class SkJpegBufferedSourceMgr : public SkJpegSourceMgr {
public:
    SkJpegBufferedSourceMgr(SkStream* stream, size_t bufferSize) : SkJpegSourceMgr(stream) {
        fBuffer = SkData::MakeUninitialized(bufferSize);
    }
    ~SkJpegBufferedSourceMgr() override {}

    void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
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
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    const std::vector<SkJpegSegment>& getAllSegments() override {
        if (fScanner) {
            return fScanner->getSegments();
        }
        ScopedSkStreamRestorer streamRestorer(fStream);
        fScanner = std::make_unique<SkJpegSegmentScanner>(kJpegMarkerEndOfImage);
        while (!fScanner->isDone() && !fScanner->hadError()) {
            constexpr size_t kBufferSize = 1024;
            uint8_t buffer[kBufferSize];
            size_t bytesRead = fStream->read(buffer, kBufferSize);
            if (bytesRead == 0) {
                SkCodecPrintf("Unexpected EOF.\n");
                break;
            }
            fScanner->onBytes(buffer, bytesRead);
        }
        return fScanner->getSegments();
    }
    sk_sp<SkData> getSubsetData(size_t offset, size_t size, bool* wasCopied) override {
        ScopedSkStreamRestorer streamRestorer(fStream);
        if (!fStream->seek(offset)) {
            SkCodecPrintf("Failed to seek to subset stream position.\n");
            return nullptr;
        }
        sk_sp<SkData> data = SkData::MakeUninitialized(size);
        if (fStream->read(data->writable_data(), size) != size) {
            SkCodecPrintf("Failed to read subset stream data.\n");
            return nullptr;
        }
        if (wasCopied) {
            *wasCopied = true;
        }
        return data;
    }
    sk_sp<SkData> getSegmentParameters(const SkJpegSegment& segment) override {
        // If the segment's parameter length isn't longer than the two bytes for the length,
        // early-out early-out.
        if (segment.parameterLength <= kJpegSegmentParameterLengthSize) {
            return nullptr;
        }

        // Seek to the start of the segment.
        ScopedSkStreamRestorer streamRestorer(fStream);
        if (!fStream->seek(segment.offset)) {
            SkCodecPrintf("Failed to seek to segment\n");
            return nullptr;
        }

        // Read the marker and verify it matches `segment`.
        uint8_t markerCode[kJpegMarkerCodeSize] = {0};
        if (fStream->read(markerCode, kJpegMarkerCodeSize) != kJpegMarkerCodeSize) {
            SkCodecPrintf("Failed to read segment marker code\n");
            return nullptr;
        }
        SkASSERT(markerCode[0] == 0xFF);
        SkASSERT(markerCode[1] == segment.marker);

        // Read the parameter length and verify it matches `segment`.
        uint8_t parameterLength[kJpegSegmentParameterLengthSize] = {0};
        if (fStream->read(parameterLength, kJpegSegmentParameterLengthSize) !=
            kJpegSegmentParameterLengthSize) {
            SkCodecPrintf("Failed to read parameter length\n");
            return nullptr;
        }
        SkASSERT(256 * parameterLength[0] + parameterLength[1] == segment.parameterLength);

        // Read the remainder of the segment.
        size_t sizeToRead = segment.parameterLength - kJpegSegmentParameterLengthSize;
        auto result = SkData::MakeUninitialized(sizeToRead);
        if (fStream->read(result->writable_data(), sizeToRead) != sizeToRead) {
            return nullptr;
        }

        return result;
    }
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

private:
    sk_sp<SkData> fBuffer;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegUnseekableSourceMgr

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
/*
 * This class implements SkJpegSourceMgr for a stream that cannot seek or rewind. It scans the data
 * as it is presented to the decoder. This allows it to track the position of segments, so that it
 * can extract subsets at a specific offset (e.g, relative to the EndOfImage segment for JpegR or
 * relative to an MPF segment for MPF).
 */
class SkJpegUnseekableSourceMgr : public SkJpegSourceMgr {
public:
    SkJpegUnseekableSourceMgr(SkStream* stream, size_t bufferSize) : SkJpegSourceMgr(stream) {
        fBuffer = SkData::MakeUninitialized(bufferSize);
        fScanner = std::make_unique<SkJpegSegmentScanner>(kJpegMarkerEndOfImage);
    }
    ~SkJpegUnseekableSourceMgr() override {}

    void initSource(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        nextInputByte = fBuffer->bytes();
        bytesInBuffer = 0;
    }
    bool fillInputBuffer(const uint8_t*& nextInputByte, size_t& bytesInBuffer) override {
        if (!readToBufferAndScan(fBuffer->size())) {
            SkCodecPrintf("Failure filling unseekable input buffer.\n");
            return false;
        }
        nextInputByte = fBuffer->bytes();
        bytesInBuffer = fLastReadSize;
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

        // Read the remaining bytes to skip into fBuffer and feed them into fScanner.
        while (bytesToSkip > 0) {
            if (!readToBufferAndScan(std::min(bytesToSkip, fBuffer->size()))) {
                SkCodecPrintf("Failure filling unseekable input buffer.\n");
                return false;
            }
            bytesToSkip -= fLastReadSize;
        }

        // Indicate to libjpeg that it it needs to call fillInputBuffer.
        bytesInBuffer = 0;
        nextInputByte = fBuffer->bytes();
        return true;
    }
    const std::vector<SkJpegSegment>& getAllSegments() override {
        while (!fScanner->isDone() && !fScanner->hadError()) {
            if (!readToBufferAndScan(fBuffer->size())) {
                SkCodecPrintf("Failure finishing unseekable input buffer.\n");
                break;
            }
        }
        return fScanner->getSegments();
    }
    sk_sp<SkData> getSubsetData(size_t offset, size_t size, bool* wasCopied) override {
        // If we haven't reached the EndOfImage, then we are throwing away the base image before
        // decoding it. This is only reasonable for tests.
        if (!fScanner->isDone()) {
            SkCodecPrintf("getSubsetData is prematurely terminating scan.\n");
        }

        // If we have read past offset, we can never get that data back again.
        if (offset < fLastReadOffset) {
            SkCodecPrintf("Requested that is gone.\n");
            return nullptr;
        }

        // Allocate the memory to return, and indicate that the result is a copy.
        sk_sp<SkData> subsetData = SkData::MakeUninitialized(size);
        uint8_t* subsetDataCurrent = reinterpret_cast<uint8_t*>(subsetData->writable_data());

        // Determine the relationship between the offset we're reading from and |fBuffer|.
        size_t offsetIntoBuffer = offset - fLastReadOffset;
        if (offsetIntoBuffer >= fLastReadSize) {
            // We have to skip past |fBuffer| to get to |offset|.
            fLastReadOffset += fLastReadSize;
            fLastReadSize = 0;

            // Skip any additional bytes needed to get to |offset|.
            size_t bytesToSkip = offset - fLastReadOffset;
            while (bytesToSkip > 0) {
                size_t bytesSkipped = fStream->skip(bytesToSkip);
                if (bytesSkipped == 0) {
                    SkCodecPrintf("Failed to skip bytes before subset.\n");
                    return nullptr;
                }
                bytesToSkip -= bytesSkipped;
                fLastReadOffset += bytesSkipped;
            }
        } else {
            // This assert is to emphatically document the side of the branch we're on.
            SkASSERT(offsetIntoBuffer < fLastReadSize);

            // Some of the data we want to copy has already been read into |fBuffer|. Copy that data
            // to |subsetData|
            size_t bytesToReadFromBuffer = std::min(fLastReadSize - offsetIntoBuffer, size);
            memcpy(subsetDataCurrent, fBuffer->bytes() + offsetIntoBuffer, bytesToReadFromBuffer);
            size -= bytesToReadFromBuffer;
            subsetDataCurrent += bytesToReadFromBuffer;

            // If all of the data that we needed was in |fBuffer|, then return early.
            if (size == 0) {
                if (wasCopied) {
                    *wasCopied = true;
                }
                return subsetData;
            }
            // We will now have to read beyond |fBuffer|, so reset it.
            fLastReadOffset += fLastReadSize;
            fLastReadSize = 0;
        }

        // Read the remaining data from |fStream|.
        while (size > 0) {
            size_t bytesRead = fStream->read(subsetDataCurrent, size);
            if (bytesRead == 0) {
                SkCodecPrintf("Failed to read subset stream data.\n");
                return nullptr;
            }
            size -= bytesRead;
            subsetDataCurrent += bytesRead;
            fLastReadOffset += bytesRead;
        }

        if (wasCopied) {
            *wasCopied = true;
        }
        return subsetData;
    }
    sk_sp<SkData> getSegmentParameters(const SkJpegSegment& segment) override {
        // The only way to implement this for an unseekable stream is to record the parameters as
        // they are scanned.
        return nullptr;
    }

private:
    // Read the specified number of bytes into fBuffer and feed them to fScanner. The number of
    // bytes must not be larger than fBuffer's size.
    bool readToBufferAndScan(size_t bytesToRead) {
        SkASSERT(bytesToRead <= fBuffer->size());
        fLastReadOffset += fLastReadSize;
        fLastReadSize = fStream->read(fBuffer->writable_data(), bytesToRead);
        if (fLastReadSize == 0) {
            SkCodecPrintf("Hit end of file reading an unseekable stream.\n");
            return false;
        }
        fScanner->onBytes(fBuffer->bytes(), fLastReadSize);
        return true;
    }

    sk_sp<SkData> fBuffer;

    // The number of bytes that were most recently read into fBuffer (this can be less than the size
    // of fBuffer).
    size_t fLastReadSize = 0;

    // The offset into the stream (total number of bytes read) at the time of our most recent read
    // into fBuffer.
    size_t fLastReadOffset = 0;
};
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegSourceMgr

// static
std::unique_ptr<SkJpegSourceMgr> SkJpegSourceMgr::Make(SkStream* stream, size_t bufferSize) {
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    if (!stream->hasPosition()) {
        return std::make_unique<SkJpegUnseekableSourceMgr>(stream, bufferSize);
    }
#endif
    if (stream->hasLength() && stream->getMemoryBase()) {
        return std::make_unique<SkJpegMemorySourceMgr>(stream);
    }
    return std::make_unique<SkJpegBufferedSourceMgr>(stream, bufferSize);
}

SkJpegSourceMgr::SkJpegSourceMgr(SkStream* stream) : fStream(stream) {}

SkJpegSourceMgr::~SkJpegSourceMgr() = default;
