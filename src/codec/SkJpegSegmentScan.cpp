/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegSegmentScan.h"

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkAssert.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegPriv.h"

#include <cstring>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegSegmentScanner

SkJpegSegmentScanner::SkJpegSegmentScanner(uint8_t stopMarker) : fStopMarker(stopMarker) {}

size_t SkJpegSegmentScanner::bytesSinceDone() const {
    SkASSERT(fState == State::kDone);
    return fOffset - fSegments.back().offset - kMarkerCodeSize;
}

const std::vector<SkJpegSegment>& SkJpegSegmentScanner::getSegments() const {
    SkASSERT(fState == State::kDone);
    return fSegments;
}

void SkJpegSegmentScanner::onBytes(const void* data, size_t size) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
    size_t bytesRemaining = size;

    while (bytesRemaining > 0) {
        // Process the data byte-by-byte, unless we are in kSegmentParam or kEntropyCodedData, in
        // which case, perform some optimizations to avoid examining every byte.
        size_t bytesToMoveForward = 0;
        switch (fState) {
            case State::kSegmentParam: {
                // Skip forward through payloads.
                SkASSERT(fSegmentParamBytesRemaining > 0);
                bytesToMoveForward = std::min(fSegmentParamBytesRemaining, bytesRemaining);
                fSegmentParamBytesRemaining -= bytesToMoveForward;
                if (fSegmentParamBytesRemaining == 0) {
                    fState = State::kEntropyCodedData;
                }
                break;
            }
            case State::kEntropyCodedData: {
                // Skip through entropy-coded data, only looking at sentinel characters.
                const uint8_t* sentinel =
                        reinterpret_cast<const uint8_t*>(memchr(bytes, 0xFF, bytesRemaining));
                if (sentinel) {
                    bytesToMoveForward = (sentinel - bytes) + 1;
                    fState = State::kEntropyCodedDataSentinel;
                } else {
                    bytesToMoveForward = bytesRemaining;
                }
                break;
            }
            case State::kDone:
                // Skip all data after we have hit our stop marker.
                bytesToMoveForward = bytesRemaining;
                break;
            default: {
                onByte(*bytes);
                bytesToMoveForward = 1;
                break;
            }
        }
        SkASSERT(bytesToMoveForward > 0);
        fOffset += bytesToMoveForward;
        bytes += bytesToMoveForward;
        bytesRemaining -= bytesToMoveForward;
    }
}

void SkJpegSegmentScanner::saveCurrentSegment(uint16_t length) {
    SkJpegSegment s = {fCurrentSegmentOffset, fCurrentSegmentMarker, length};
    fSegments.push_back(s);

    fCurrentSegmentMarker = 0;
    fCurrentSegmentOffset = 0;
}

void SkJpegSegmentScanner::onMarkerSecondByte(uint8_t byte) {
    SkASSERT(fState == State::kStartOfImageByte1 || fState == State::kSecondMarkerByte1 ||
             fState == State::kEntropyCodedDataSentinel ||
             fState == State::kPostEntropyCodedDataFill);

    fCurrentSegmentMarker = byte;
    fCurrentSegmentOffset = fOffset - 1;

    if (byte == fStopMarker) {
        saveCurrentSegment(0);
        fState = State::kDone;
    } else if (byte == kMarkerStartOfImage) {
        saveCurrentSegment(0);
        fState = State::kSecondMarkerByte0;
    } else if (MarkerStandsAlone(byte)) {
        saveCurrentSegment(0);
        fState = State::kEntropyCodedData;
    } else {
        fCurrentSegmentMarker = byte;
        fState = State::kSegmentParamLengthByte0;
    }
}

void SkJpegSegmentScanner::onByte(uint8_t byte) {
    switch (fState) {
        case State::kStartOfImageByte0:
            if (byte != 0xFF) {
                SkCodecPrintf("First byte was %02x, not 0xFF", byte);
                fState = State::kError;
                return;
            }
            fState = State::kStartOfImageByte1;
            break;
        case State::kStartOfImageByte1:
            if (byte != kMarkerStartOfImage) {
                SkCodecPrintf("Second byte was %02x, not %02x", byte, kMarkerStartOfImage);
                fState = State::kError;
                return;
            }
            onMarkerSecondByte(byte);
            break;
        case State::kSecondMarkerByte0:
            if (byte != 0xFF) {
                SkCodecPrintf("Third byte was %02x, not 0xFF", byte);
                fState = State::kError;
                return;
            }
            fState = State::kSecondMarkerByte1;
            break;
        case State::kSecondMarkerByte1:
            // See section B.1.1.3: All markers are assigned two-byte codes: a 0xFF byte followed by
            // a byte which is not equal to 0x00 or 0xFF.
            if (byte == 0xFF || byte == 0x00) {
                SkCodecPrintf("SkJpegSegment marker was 0xFF,0xFF or 0xFF,0x00");
                fState = State::kError;
                return;
            }
            onMarkerSecondByte(byte);
            break;
        case State::kSegmentParamLengthByte0:
            fSegmentParamLengthByte0 = byte;
            fState = State::kSegmentParamLengthByte1;
            break;
        case State::kSegmentParamLengthByte1: {
            uint16_t paramLength = 256u * fSegmentParamLengthByte0 + byte;
            fSegmentParamLengthByte0 = 0;

            // See section B.1.1.4: A marker segment consists of a marker followed by a sequence
            // of related parameters. The first parameter in a marker segment is the two-byte length
            // parameter. This length parameter encodes the number of bytes in the marker segment,
            // including the length parameter and excluding the two-byte marker.
            if (paramLength < kParameterLengthSize) {
                SkCodecPrintf("SkJpegSegment payload length was %u < 2 bytes", paramLength);
                fState = State::kError;
                return;
            }
            saveCurrentSegment(paramLength);
            fSegmentParamBytesRemaining = paramLength - kParameterLengthSize;
            if (fSegmentParamBytesRemaining > 0) {
                fState = State::kSegmentParam;
            } else {
                fState = State::kEntropyCodedData;
            }
            break;
        }
        case State::kSegmentParam:
            SkASSERT(fSegmentParamBytesRemaining > 0);
            fSegmentParamBytesRemaining -= 1;
            if (fSegmentParamBytesRemaining == 0) {
                fState = State::kEntropyCodedData;
            }
            break;
        case State::kEntropyCodedData:
            if (byte == 0xFF) {
                fState = State::kEntropyCodedDataSentinel;
            }
            break;
        case State::kEntropyCodedDataSentinel:
            if (byte == 0x00) {
                fState = State::kEntropyCodedData;
            } else if (byte == 0xFF) {
                fState = State::kPostEntropyCodedDataFill;
            } else {
                onMarkerSecondByte(byte);
            }
            break;
        case State::kPostEntropyCodedDataFill:
            // See section B.1.1.3: Any marker may optionally be preceded by any number of fill
            // bytes, which are bytes assigned code 0xFF. Skip past any 0xFF fill bytes that may be
            // present at the end of the entropy-coded data.
            if (byte == 0xFF) {
                fState = State::kPostEntropyCodedDataFill;
            } else if (byte == 0x00) {
                SkCodecPrintf("Post entropy coded data had 0xFF,0x00");
                fState = State::kError;
                return;
            } else {
                onMarkerSecondByte(byte);
            }
            break;
        case State::kDone:
            break;
        case State::kError:
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegSeekableScan

SkJpegSeekableScan::SkJpegSeekableScan(SkStream* stream,
                                       size_t initialPosition,
                                       std::vector<SkJpegSegment>&& segments)
        : fStream(stream), fInitialPosition(initialPosition), fSegments(segments) {}

std::unique_ptr<SkJpegSeekableScan> SkJpegSeekableScan::Create(SkStream* stream,
                                                               uint8_t stopMarker) {
    size_t initialPosition = stream->getPosition();

    // This implementation relies on using getPosition. It is possible to implement this with just
    // rewind.
    if (!stream->hasPosition()) {
        SkCodecPrintf("Cannot scan stream that doesn't have a position.\n");
        return nullptr;
    }

    SkJpegSegmentScanner scanner(stopMarker);
    if (stream->hasLength() && stream->getMemoryBase()) {
        // If this stream is in-memory, scan it in one go.
        scanner.onBytes(stream->getMemoryBase(), stream->getLength());
    } else {
        // Otherise, read it in 1k chunks.
        constexpr size_t kBufferSize = 1024;
        uint8_t buffer[kBufferSize];
        while (!scanner.isDone() && !scanner.hadError()) {
            size_t bytesRead = stream->read(buffer, kBufferSize);
            if (bytesRead == 0) {
                SkCodecPrintf("Unexpected EOF.\n");
            }
            scanner.onBytes(buffer, bytesRead);
        }
    }
    if (scanner.hadError()) {
        return nullptr;
    }
    std::vector<SkJpegSegment> segments = scanner.getSegments();

    return std::unique_ptr<SkJpegSeekableScan>(
            new SkJpegSeekableScan(stream, initialPosition, std::move(segments)));
}

sk_sp<SkData> SkJpegSeekableScan::copyParameters(const SkJpegSegment& segment,
                                                 const void* signature,
                                                 const size_t signatureLength) {
    constexpr size_t kParameterLengthSize = SkJpegSegmentScanner::kParameterLengthSize;
    constexpr size_t kMarkerCodeSize = SkJpegSegmentScanner::kMarkerCodeSize;
    // If the segment's parameter length isn't long enough for the signature and the length,
    // early-out.
    if (segment.parameterLength < signatureLength + kParameterLengthSize) {
        return nullptr;
    }
    size_t sizeToRead = segment.parameterLength - signatureLength - kParameterLengthSize;

    // Seek to the start of the segment.
    if (!fStream->seek(fInitialPosition + segment.offset)) {
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

std::unique_ptr<SkStream> SkJpegSeekableScan::getSubsetStream(size_t offset, size_t size) {
    // Read the image's data. It would be better to fork `stream` and limit its position and size.
    if (!fStream->seek(fInitialPosition + offset)) {
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
