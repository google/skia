/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegConstants_codec_DEFINED
#define SkJpegConstants_codec_DEFINED

#include <cstddef>
#include <cstdint>

// The first marker of all JPEG files is StartOfImage.
static constexpr uint8_t kJpegMarkerStartOfImage = 0xD8;

// The last marker of all JPEG files (excluding auxiliary data, e.g, MPF images) is EndOfImage.
static constexpr uint8_t kJpegMarkerEndOfImage = 0xD9;

// The header of a JPEG file is the data in all segments before the first StartOfScan.
static constexpr uint8_t kJpegMarkerStartOfScan = 0xDA;

// Metadata and auxiliary images are stored in the APP1 through APP15 markers.
static constexpr uint8_t kJpegMarkerAPP0 = 0xE0;

// The number of bytes in a marker code is two. The first byte is all marker codes is 0xFF.
static constexpr size_t kJpegMarkerCodeSize = 2;

// The number of bytes used to specify the length of a segment's parameters is two. This length
// value includes these two bytes.
static constexpr size_t kJpegSegmentParameterLengthSize = 2;

// The first three bytes of all JPEG files is a StartOfImage marker (two bytes) followed by the
// first byte of the next marker.
static constexpr uint8_t kJpegSig[] = {0xFF, kJpegMarkerStartOfImage, 0xFF};

// ICC profile segment marker and signature.
static constexpr uint32_t kICCMarker = kJpegMarkerAPP0 + 2;
static constexpr uint32_t kICCMarkerHeaderSize = 14;
static constexpr uint32_t kICCMarkerIndexSize = 1;
static constexpr uint8_t kICCSig[] = {
        'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', '\0',
};

// XMP segment marker and signature.
static constexpr uint32_t kXMPMarker = kJpegMarkerAPP0 + 1;
static constexpr uint8_t kXMPStandardSig[] = {
        'h', 't', 't', 'p', ':', '/', '/', 'n', 's', '.', 'a', 'd', 'o', 'b', 'e', '.', 'c', 'o',
        'm', '/', 'x', 'a', 'p', '/', '1', '.', '0', '/', '\0'};
static constexpr uint8_t kXMPExtendedSig[] = {
        'h', 't', 't', 'p', ':', '/', '/', 'n', 's', '.', 'a', 'd', 'o', 'b', 'e', '.', 'c', 'o',
        'm', '/', 'x', 'm', 'p', '/', 'e', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', '/', '\0'};

// EXIF segment marker and signature.
static constexpr uint32_t kExifMarker = kJpegMarkerAPP0 + 1;
constexpr uint8_t kExifSig[] = {'E', 'x', 'i', 'f', '\0'};

// MPF segment marker and signature.
static constexpr uint32_t kMpfMarker = kJpegMarkerAPP0 + 2;
static constexpr uint8_t kMpfSig[] = {'M', 'P', 'F', '\0'};

#endif
