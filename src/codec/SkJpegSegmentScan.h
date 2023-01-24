/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegSegmentScan_codec_DEFINED
#define SkJpegSegmentScan_codec_DEFINED

#include "include/core/SkRefCnt.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class SkData;
class SkStream;

/*
 * A JPEG segment.
 */
struct SkJpegSegment {
    // The offset in bytes from the initial position to where this segment starts.
    size_t offset = 0;
    // The second byte of the marker code, which determines the segment type.
    uint8_t marker = 0;
    // The length of the parameters for this segment (including the two bytes used to specify
    // the length).
    uint16_t parameterLength = 0;
};

/*
 * Class for scanning JPEG data. The JPEG format consists of a sequence of segments that begin with
 * a marker, with entropy-coded data in between.
 */
class SkJpegSegmentScanner {
public:
    SkJpegSegmentScanner(uint8_t stopMarker = kMarkerEndOfImage);

    bool isDone() const { return fState == State::kDone; }
    bool hadError() const { return fState == State::kError; }

    // Provide more bytes of data to the state machine.
    void onBytes(const void* data, size_t size);

    // Return the number of bytes that have been received via OnBytes since the marker that caused
    // the transition to kDone was read. This will assert that the state is kDone.
    size_t bytesSinceDone() const;

    // Return the segments that have been retrieved so far. This will assert that no errors have
    // been encountered.
    const std::vector<SkJpegSegment>& getSegments() const;

    // Convenient markers to know.
    static constexpr uint8_t kMarkerStartOfImage = 0xD8;
    static constexpr uint8_t kMarkerEndOfImage = 0xD9;
    static constexpr uint8_t kMarkerStartOfScan = 0xDA;

    // The number of bytes in a marker code is two.
    static constexpr size_t kMarkerCodeSize = 2;

    // The number of bytes used to specify the length of a segment's parameters is two.
    static constexpr size_t kParameterLengthSize = 2;

private:
    // The scanner is a state machine. State transitions happen when a byte is read.
    enum class State {
        // The initial state, before we read the 0xFF,0xD8,0xFF JPEG signature.
        kStartOfImageByte0,
        // We have read the 0xFF of the JPEG signature.
        kStartOfImageByte1,
        // We have read the 0xFF,0xD8 of the JPEG signature. The next byte should be the 0xFF that
        // completes the JPEG signature and starts the second marker (the one after StartOfImage).
        kSecondMarkerByte0,
        // We have read the full JPEG signature. The next byte should be the the second byte of the
        // second marker.
        kSecondMarkerByte1,
        // We have read a marker that does not stand alone. The next byte is the first byte of the
        // length of the parameters,
        kSegmentParamLengthByte0,
        // We have read the first byte of the length of the parameters (and it is stored in
        // |fSegmentParamLengthByte0|). The next byte we read is the second byte of the length of
        // the parameters.
        kSegmentParamLengthByte1,
        // We have read the full length of the parameters. The next |fSegmentParamBytesRemaining|
        // bytes are the parameters.
        kSegmentParam,
        // We have read a marker and (optionally) its parameters, and we are now reading entropy-
        // coded data. All subsequent bytes until we reach 0xFF are entropy-coded data.
        kEntropyCodedData,
        // We reached an 0xFF in entropy-coded data. If the next byte is 0x00 then we continue
        // reading entropy-coded data. If the next byte is 0xFF then we are reading fill data.
        // If the next byte is anything else then it is the second byte of a marker.
        kEntropyCodedDataSentinel,
        // We are reading fill data. If the next byte is 0xFF then we are still reading fill data,
        // otherwise the next byte is the second byte of a marker.
        kPostEntropyCodedDataFill,
        // We reached |fStopMarker| and have stopped tracking our state.
        kDone,
        // We hit an error somewhere and have given up.
        kError,
    };
    State fState = State::kStartOfImageByte0;

    // Update state transition when a single byte is read.
    void onByte(uint8_t byte);

    // Perform the appropriate state transition for when a marker is read. This will set
    // |fCurrentSegmentMarker| and |fCurrentSegmentOffset|, and potentially call saveCurrentSegment.
    void onMarkerSecondByte(uint8_t byte);

    // Add a new entry in |segments| for |fCurrentSegmentMarker| and offset |fCurrentSegmentOffset|
    // and the specified length.
    void saveCurrentSegment(uint16_t length);

    static bool MarkerStandsAlone(uint8_t marker) {
        // These markers are TEM (0x01), RSTm (0xD0 through 0xD7), SOI (0xD8), and
        // EOI (0xD9). See section B.1.1.3, Marker assignments.
        return marker == 0x01 || (marker >= 0xD0 && marker <= 0xD9);
    }

    // Stop tracking state when we hit this marker. If this is 0x00, then never stop.
    const uint8_t fStopMarker;

    // The number of bytes that have been processed so far.
    size_t fOffset = 0;

    // If |fState| is kSegmentParamLengthByte1, then this is the value of the the previous byte.
    uint8_t fSegmentParamLengthByte0 = 0;

    // If |fState| is kSegmentParam, then this is the number of bytes reamining in the current
    // segment.
    size_t fSegmentParamBytesRemaining = 0;

    // The offset and marker for the segment started by the previous call to OnMarkerSecondByte.
    // These are re-set when SaveCurrentSegment is called.
    size_t fCurrentSegmentOffset = 0;
    uint8_t fCurrentSegmentMarker = 0;

    std::vector<SkJpegSegment> fSegments;
};

/*
 * This class will return the segment structure for a JPEG file represented by a seekable SkStream.
 * It can then be used to extract the parameters for any segment, as long as the original SkStream
 * is still valid.
 */
class SkJpegSeekableScan {
public:
    // Scan the stream, starting at its current position (not rewinding first), and stopping when
    // the specified stop marker is reached. This will return nullptr if the stop marker is not
    // reached.
    static std::unique_ptr<SkJpegSeekableScan> Create(
            SkStream* stream, uint8_t stopMarker = SkJpegSegmentScanner::kMarkerStartOfScan);

    // Return the list of segments from a scan.
    const std::vector<SkJpegSegment>& segments() { return fSegments; }

    // Copy the parameters from a segment. Return nullptr if the initial bytes of the parameters
    // section do not match the specified signature. Return the parameters starting from end of the
    // signature (so, kParameterLengthSize + signatureLength bytes into the parameter data).
    sk_sp<SkData> copyParameters(const SkJpegSegment& segment,
                                 const void* signature,
                                 const size_t signatureLength);

    // Return a stream for a subset of the original stream, starting at the specified offset, and
    // with the specified length.
    std::unique_ptr<SkStream> getSubsetStream(size_t offset, size_t size);

private:
    SkJpegSeekableScan(SkStream* stream, size_t initialPosition, std::vector<SkJpegSegment>&&);

    SkStream* const fStream;
    const size_t fInitialPosition;
    const std::vector<SkJpegSegment> fSegments;
};

#endif
