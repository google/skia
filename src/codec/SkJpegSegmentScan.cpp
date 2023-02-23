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
#include "src/codec/SkJpegConstants.h"

#include <cstring>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegSegmentScanner

SkJpegSegmentScanner::SkJpegSegmentScanner(uint8_t stopMarker) : fStopMarker(stopMarker) {}

const std::vector<SkJpegSegment>& SkJpegSegmentScanner::getSegments() const { return fSegments; }

sk_sp<SkData> SkJpegSegmentScanner::GetParameters(const SkData* scannedData,
                                                  const SkJpegSegment& segment) {
    return SkData::MakeSubset(
            scannedData,
            segment.offset + kJpegMarkerCodeSize + kJpegSegmentParameterLengthSize,
            segment.parameterLength - kJpegSegmentParameterLengthSize);
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
    } else if (byte == kJpegMarkerStartOfImage) {
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
            if (byte != kJpegMarkerStartOfImage) {
                SkCodecPrintf("Second byte was %02x, not %02x", byte, kJpegMarkerStartOfImage);
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
            if (paramLength < kJpegSegmentParameterLengthSize) {
                SkCodecPrintf("SkJpegSegment payload length was %u < 2 bytes", paramLength);
                fState = State::kError;
                return;
            }
            saveCurrentSegment(paramLength);
            fSegmentParamBytesRemaining = paramLength - kJpegSegmentParameterLengthSize;
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
