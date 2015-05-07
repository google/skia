/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkJpegInfo.h"

namespace {
class JpegSegment {
public:
    JpegSegment(const SkData* skdata)
        : fData(static_cast<const char*>(skdata->data()))
        , fSize(skdata->size())
        , fOffset(0)
        , fLength(0) {}
    bool read() {
        if (!this->readBigendianUint16(&fMarker)) {
            return false;
        }
        if (JpegSegment::StandAloneMarker(fMarker)) {
            fLength = 0;
            fBuffer = NULL;
            return true;
        }
        if (!this->readBigendianUint16(&fLength) || fLength < 2) {
            return false;
        }
        fLength -= 2;  // Length includes itself for some reason.
        if (fOffset + fLength > fSize) {
            return false;  // Segment too long.
        }
        fBuffer = &fData[fOffset];
        fOffset += fLength;
        return true;
    }

    bool isSOF() {
        return (fMarker & 0xFFF0) == 0xFFC0 && fMarker != 0xFFC4 &&
               fMarker != 0xFFC8 && fMarker != 0xFFCC;
    }
    uint16_t marker() { return fMarker; }
    uint16_t length() { return fLength; }
    const char* data() { return fBuffer; }

    static uint16_t GetBigendianUint16(const char* ptr) {
        // "the most significant byte shall come first"
        return (static_cast<uint8_t>(ptr[0]) << 8) |
            static_cast<uint8_t>(ptr[1]);
    }

private:
    const char* const fData;
    const size_t fSize;
    size_t fOffset;
    const char* fBuffer;
    uint16_t fMarker;
    uint16_t fLength;

    bool readBigendianUint16(uint16_t* value) {
        if (fOffset + 2 > fSize) {
            return false;
        }
        *value = JpegSegment::GetBigendianUint16(&fData[fOffset]);
        fOffset += 2;
        return true;
    }
    static bool StandAloneMarker(uint16_t marker) {
        // RST[m] markers or SOI, EOI, TEM
        return (marker & 0xFFF8) == 0xFFD0 || marker == 0xFFD8 ||
               marker == 0xFFD9 || marker == 0xFF01;
    }
};
}  // namespace

bool SkIsJFIF(const SkData* skdata, SkJFIFInfo* info) {
    static const uint16_t kSOI = 0xFFD8;
    static const uint16_t kAPP0 = 0xFFE0;
    JpegSegment segment(skdata);
    if (!segment.read() || segment.marker() != kSOI) {
        return false;  // not a JPEG
    }
    if (!segment.read() || segment.marker() != kAPP0) {
        return false;  // not an APP0 segment
    }
    static const char kJfif[] = {'J', 'F', 'I', 'F', '\0'};
    SkASSERT(segment.data());
    if (SkToSizeT(segment.length()) < sizeof(kJfif) ||
        0 != memcmp(segment.data(), kJfif, sizeof(kJfif))) {
        return false;  // Not JFIF JPEG
    }
    do {
        if (!segment.read()) {
            return false;  // malformed JPEG
        }
    } while (!segment.isSOF());
    if (segment.length() < 6) {
        return false;  // SOF segment is short
    }
    if (8 != segment.data()[0]) {
        return false;  // Only support 8-bit precision
    }
    int numberOfComponents = segment.data()[5];
    if (numberOfComponents != 1 && numberOfComponents != 3) {
        return false;  // Invalid JFIF
    }
    if (info) {
        info->fHeight = JpegSegment::GetBigendianUint16(&segment.data()[1]);
        info->fWidth = JpegSegment::GetBigendianUint16(&segment.data()[3]);
        if (numberOfComponents == 3) {
            info->fType = SkJFIFInfo::kYCbCr;
        } else {
            info->fType = SkJFIFInfo::kGrayscale;
        }
    }
    return true;
}


