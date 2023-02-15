/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegConstants_codec_DEFINED
#define SkJpegConstants_codec_DEFINED

#include <cstdint>

static constexpr uint8_t kJpegSig[] = {0xFF, 0xD8, 0xFF};

static constexpr uint32_t kICCMarker = 0xE2;
static constexpr uint32_t kICCMarkerHeaderSize = 14;
static constexpr uint32_t kICCMarkerIndexSize = 1;
static constexpr uint8_t kICCSig[] = {
        'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', '\0',
};

static constexpr uint32_t kGainmapMarker = 0xEF;
static constexpr uint32_t kGainmapMarkerIndexSize = 2;
static constexpr uint8_t kGainmapSig[] = {
        'H', 'D', 'R', '_', 'G', 'A', 'I', 'N', '_', 'M', 'A', 'P', '\0',
};

static constexpr uint32_t kXMPMarker = 0xE1;
static constexpr uint8_t kXMPStandardSig[] = {
        'h', 't', 't', 'p', ':', '/', '/', 'n', 's', '.', 'a', 'd', 'o', 'b', 'e', '.', 'c', 'o',
        'm', '/', 'x', 'a', 'p', '/', '1', '.', '0', '/', '\0'};
static constexpr uint8_t kXMPExtendedSig[] = {
        'h', 't', 't', 'p', ':', '/', '/', 'n', 's', '.', 'a', 'd', 'o', 'b', 'e', '.', 'c', 'o',
        'm', '/', 'x', 'm', 'p', '/', 'e', 'x', 't', 'e', 'n', 's', 'i', 'o', 'n', '/', '\0'};

static constexpr uint32_t kExifMarker = 0xE1;
static constexpr uint32_t kExifHeaderSize = 14;
constexpr uint8_t kExifSig[] = {'E', 'x', 'i', 'f', '\0'};

static constexpr uint32_t kMpfMarker = 0xE2;
static constexpr uint8_t kMpfSig[] = {'M', 'P', 'F', '\0'};

#endif
