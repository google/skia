/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegSourceMgr.h"

#include "include/core/SkTypes.h"

#ifdef SK_CODEC_DECODES_JPEG
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "src/codec/SkCodecPriv.h"

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
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
    ScopedSkStreamRestorer(SkStream* stream)
            : fStream(stream), fPosition(stream->hasPosition() ? stream->getPosition() : 0) {
        if (canRestore()) {
            if (!fStream->rewind()) {
                SkCodecPrintf("Failed to rewind decoder stream.\n");
            }
        }
    }
    ~ScopedSkStreamRestorer() {
        if (canRestore()) {
            if (!fStream->seek(fPosition)) {
                SkCodecPrintf("Failed to restore decoder stream.\n");
            }
        }
    }
    bool canRestore() const { return fStream->hasPosition(); }

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
        fScanner = std::make_unique<SkJpegSegmentScanner>(SkJpegSegmentScanner::kMarkerEndOfImage);
        fScanner->onBytes(fStream->getMemoryBase(), fStream->getLength());
        return fScanner->getSegments();
    }
    std::unique_ptr<SkStream> getSubsetStream(size_t offset, size_t size) override {
        if (offset > fStream->getLength() || size > fStream->getLength() - offset) {
            return nullptr;
        }
        return SkMemoryStream::MakeCopy(
                reinterpret_cast<const uint8_t*>(fStream->getMemoryBase()) + offset, size);
    }
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegBufferedSourceMgr

class SkJpegBufferedSourceMgr : public SkJpegSourceMgr {
public:
    SkJpegBufferedSourceMgr(SkStream* stream) : SkJpegSourceMgr(stream) {}
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
#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
    const std::vector<SkJpegSegment>& getAllSegments() override {
        if (fScanner) {
            return fScanner->getSegments();
        }
        ScopedSkStreamRestorer streamRestorer(fStream);
        fScanner = std::make_unique<SkJpegSegmentScanner>(SkJpegSegmentScanner::kMarkerEndOfImage);
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
    std::unique_ptr<SkStream> getSubsetStream(size_t offset, size_t size) override {
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
        return SkMemoryStream::Make(data);
    }
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

private:
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

SkJpegSourceMgr::SkJpegSourceMgr(SkStream* stream) : fStream(stream) {}

SkJpegSourceMgr::~SkJpegSourceMgr() = default;

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
sk_sp<SkData> SkJpegSourceMgr::copyParameters(const SkJpegSegment& segment,
                                              const void* signature,
                                              const size_t signatureLength) {
    // This functionality is only available for seekable streams.
    if (!fStream->hasPosition()) {
        return nullptr;
    }

    constexpr size_t kParameterLengthSize = SkJpegSegmentScanner::kParameterLengthSize;
    constexpr size_t kMarkerCodeSize = SkJpegSegmentScanner::kMarkerCodeSize;
    // If the segment's parameter length isn't long enough for the signature and the length,
    // early-out.
    if (segment.parameterLength < signatureLength + kParameterLengthSize) {
        return nullptr;
    }
    size_t sizeToRead = segment.parameterLength - signatureLength - kParameterLengthSize;

    // Seek to the start of the segment.
    if (!fStream->seek(segment.offset)) {
        SkCodecPrintf("Failed to seek to segment\n");
        return nullptr;
    }

    // Read the marker and verify it matches `segment`.
    uint8_t markerCode[kMarkerCodeSize] = {0};
    if (fStream->read(markerCode, kMarkerCodeSize) != kMarkerCodeSize) {
        SkCodecPrintf("Failed to read segment marker code\n");
        return nullptr;
    }
    SkASSERT(markerCode[0] == 0xFF);
    SkASSERT(markerCode[1] == segment.marker);

    // Read the parameter length and verify it matches `segment`.
    uint8_t parameterLength[kParameterLengthSize] = {0};
    if (fStream->read(parameterLength, kParameterLengthSize) != kParameterLengthSize) {
        SkCodecPrintf("Failed to read parameter length\n");
        return nullptr;
    }
    SkASSERT(256 * parameterLength[0] + parameterLength[1] == segment.parameterLength);

    // Check the next bytes against `signature`.
    auto segmentSignature = SkData::MakeUninitialized(signatureLength);
    if (fStream->read(segmentSignature->writable_data(), segmentSignature->size()) !=
        segmentSignature->size()) {
        SkCodecPrintf("Failed to read parameters\n");
        return nullptr;
    }
    if (memcmp(segmentSignature->data(), signature, signatureLength) != 0) {
        return nullptr;
    }

    // Finally, read the remainder of the segment.
    auto result = SkData::MakeUninitialized(sizeToRead);
    if (fStream->read(result->writable_data(), sizeToRead) != sizeToRead) {
        return nullptr;
    }

    return result;
}
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS
#endif  // SK_CODEC_DECODES_JPEG
