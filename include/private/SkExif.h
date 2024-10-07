/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkExif_DEFINED
#define SkExif_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

#include <cstdint>
#include <optional>

class SkData;

namespace SkExif {

// Tag values that are parsed by Parse and stored in Metadata.
static constexpr uint16_t kOriginTag = 0x112;
static constexpr uint16_t kResolutionUnitTag = 0x0128;
static constexpr uint16_t kXResolutionTag = 0x011a;
static constexpr uint16_t kYResolutionTag = 0x011b;
static constexpr uint16_t kPixelXDimensionTag = 0xa002;
static constexpr uint16_t kPixelYDimensionTag = 0xa003;

struct Metadata {
    // The image orientation.
    std::optional<SkEncodedOrigin> fOrigin;

    // The HDR headroom property.
    // https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
    std::optional<float> fHdrHeadroom;

    // Resolution.
    std::optional<uint16_t> fResolutionUnit;
    std::optional<float> fXResolution;
    std::optional<float> fYResolution;

    // Size in pixels.
    std::optional<uint32_t> fPixelXDimension;
    std::optional<uint32_t> fPixelYDimension;
};

/*
 * Parse the metadata specified in |data| and write them to |metadata|. Stop only at an
 * unrecoverable error (allow truncated input).
 */
void SK_API Parse(Metadata& metadata, const SkData* data);

/*
 * Write exif data that includes the values in |metadata| and returns it as SkData
 * untruncated. Return nullptr if a write to the data stream fails or if
 * |metadata.fHdrHeadroom| has value.
 * This function cannot write an IFD entry based on the HdrHeadroom value because
 * the information of maker33 and maker48 are lost in decoding.
 * For metadata that belongs in a subIFD the function will write it to one and
 * store it after the root IFD.
 * Data that does not fit within kSizeEntry, is appended to the end of the data,
 * after the subIFD if it exists.
 */
sk_sp<SkData> WriteExif(Metadata& metadata);

}  // namespace SkExif

#endif
