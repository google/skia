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
 * Return a scan of the segments of a JPEG image. The JPEG format consists of a sequence of segments
 * that begin with a marker, with entropy-coded data in between. This class will return the segment
 * structure for a JPEG file represented by a seekable SkStream. It can then be used to extract the
 * parameters for any segment, as long as the original SkStream is still valid.
 */
class SkJpegSegmentScan {
public:
    struct Segment {
        // The offset in bytes from the stream's initial position where this segment starts.
        size_t offset = 0;
        // The second byte of the marker code, which determines the segment type.
        uint8_t marker = 0;
        // The length of the parameters for this segment (including the two bytes used to specify
        // the length).
        uint16_t parameterLength = 0;
    };

    struct Options {
        // If true, then stop the scan when the first StartOfScan marker is read.
        bool stopOnStartOfScan = true;
        // Stop the scan when the specified number of EndOfImage markers are read.
        size_t stopOnEndOfImageCount = 1;
    };

    // Scan the stream, starting at its current position (not rewinding first), with the termination
    // conditions specified in the indicated options. This will return an object only when the scan
    // reached its termination conditions without encountering any errors.
    static std::unique_ptr<SkJpegSegmentScan> Create(SkStream* stream, const Options& options);

    // Return the list of segments from a scan.
    const std::vector<Segment>& segments() { return fSegments; }

    // Copy the parameters from a segment. Return nullptr if the initial bytes of the parameters
    // section do not match the specified signature. Return the parameters starting from end of the
    // signature (so, kParameterLengthSize + signatureLength bytes into the parameter data).
    sk_sp<SkData> copyParameters(const Segment& segment,
                                 const void* signature,
                                 const size_t signatureLength);

    // The number of bytes in a marker code is two.
    static constexpr size_t kMarkerCodeSize = 2;

    // The number of bytes used to specify the length of a segment's parameters is two.
    static constexpr size_t kParameterLengthSize = 2;

private:
    SkJpegSegmentScan(SkStream* stream, size_t initialPosition, std::vector<Segment>&&);

    // Skip past any entropy-coded data. If this function returns true, then the stream is
    // positioned to read the next (potentially valid) marker code. If the function returns false,
    // then there was a read failure.
    static bool SkipPastEntropyCodedData(SkStream* stream);

    // Returns true if this is a valid marker code (starts with 0xFF and is not 0xFF,0x00 or
    // 0xFF,0xFF).
    static bool MarkerIsValid(const uint8_t markerCode[kMarkerCodeSize]);

    // Returns true if this marker stand alone, which means that it does not have parameters.
    static bool MarkerStandsAlone(uint8_t marker);

    static constexpr uint8_t kMarkerStartOfImage = 0xD8;
    static constexpr uint8_t kMarkerEndOfImage = 0xD9;
    static constexpr uint8_t kMarkerStartOfScan = 0xDA;

    SkStream* const fStream;
    const size_t fInitialPosition;
    const std::vector<Segment> fSegments;
};

#endif
