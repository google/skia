/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkExif.h"

#include "include/core/SkData.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkTiffUtility.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <utility>

constexpr uint16_t kSubIFDOffsetTag = 0x8769;
constexpr uint16_t kOriginTag = 0x112;
constexpr uint16_t kMarkerNoteTag = 0x927c;

// Physical resolution.
constexpr uint16_t kXResolutionTag = 0x011a;
constexpr uint16_t kYResolutionTag = 0x011b;
constexpr uint16_t kResolutionUnitTag = 0x0128;

// Size in pixels. Also sometimes called ImageWidth and ImageHeight.
constexpr uint16_t kPixelXDimensionTag = 0xa002;
constexpr uint16_t kPixelYDimensionTag = 0xa003;

static bool get_maker_note_hdr_headroom(sk_sp<SkData> data, float* hdrHeadroom) {
    // No little endian images that specify this data have been observed. Do not add speculative
    // support.
    const bool kLittleEndian = false;
    const uint8_t kSig[] = {
            'A', 'p', 'p', 'l', 'e', ' ', 'i', 'O', 'S', 0, 0, 1, 'M', 'M',  //
    };
    if (!data || data->size() < sizeof(kSig)) {
        return false;
    }
    if (memcmp(data->data(), kSig, sizeof(kSig)) != 0) {
        return false;
    }
    auto ifd =
            SkTiffImageFileDirectory::MakeFromOffset(std::move(data), kLittleEndian, sizeof(kSig));
    if (!ifd) {
        return false;
    }

    // See documentation at:
    // https://developer.apple.com/documentation/appkit/images_and_pdf/applying_apple_hdr_effect_to_your_photos
    bool hasMaker33 = false;
    bool hasMaker48 = false;
    float maker33 = 0.f;
    float maker48 = 0.f;
    for (uint32_t i = 0; i < ifd->getNumEntries(); ++i) {
        switch (ifd->getEntryTag(i)) {
            case 33:
                if (!hasMaker33) {
                    hasMaker33 = ifd->getEntrySignedRational(i, 1, &maker33);
                }
                break;
            case 48:
                if (!hasMaker48) {
                    hasMaker48 = ifd->getEntrySignedRational(i, 1, &maker48);
                }
                break;
            default:
                break;
        }
    }
    // Many images have a maker33 but not a maker48. Treat them as having maker48 of 0.
    if (!hasMaker33) {
        return false;
    }
    float stops = 0.f;
    if (maker33 < 1.0f) {
        if (maker48 <= 0.01f) {
            stops = -20.0f * maker48 + 1.8f;
        } else {
            stops = -0.101f * maker48 + 1.601f;
        }
    } else {
        if (maker48 <= 0.01f) {
            stops = -70.0f * maker48 + 3.0f;
        } else {
            stops = -0.303f * maker48 + 2.303f;
        }
    }
    *hdrHeadroom = std::pow(2.f, std::max(stops, 0.f));
    return true;
}

SkExifMetadata::SkExifMetadata(sk_sp<SkData> data) : fData(std::move(data)) {
    if (!fData) {
        return;
    }
    bool littleEndian = false;
    uint32_t ifdOffset = 0;
    if (!SkTiffImageFileDirectory::ParseHeader(fData.get(), &littleEndian, &ifdOffset)) {
        SkCodecPrintf("Failed to parse Exif header.\n");
        return;
    }
    parseIfd(ifdOffset, littleEndian, /*isRoot=*/true);
}

void SkExifMetadata::parseIfd(uint32_t ifdOffset, bool littleEndian, bool isRoot) {
    auto ifd = SkTiffImageFileDirectory::MakeFromOffset(fData, littleEndian, ifdOffset);
    if (!ifd) {
        SkCodecPrintf("Failed to make IFD\n");
        return;
    }
    for (uint32_t i = 0; i < ifd->getNumEntries(); ++i) {
        switch (ifd->getEntryTag(i)) {
            case kOriginTag: {
                uint16_t value = 0;
                if (!fOriginPresent && ifd->getEntryUnsignedShort(i, 1, &value)) {
                    if (0 < value && value <= kLast_SkEncodedOrigin) {
                        fOriginValue = static_cast<SkEncodedOrigin>(value);
                        fOriginPresent = true;
                    }
                }
                break;
            }
            case kMarkerNoteTag:
                if (!fHdrHeadroomPresent) {
                    if (auto makerNoteData = ifd->getEntryUndefinedData(i)) {
                        fHdrHeadroomPresent = get_maker_note_hdr_headroom(std::move(makerNoteData),
                                                                          &fHdrHeadroomValue);
                    }
                }
                break;
            case kSubIFDOffsetTag: {
                uint32_t subIfdOffset = 0;
                if (isRoot && ifd->getEntryUnsignedLong(i, 1, &subIfdOffset)) {
                    parseIfd(subIfdOffset, littleEndian, /*isRoot=*/false);
                }
                break;
            }
            case kXResolutionTag:
                if (!fXResolutionPresent) {
                    fXResolutionPresent = ifd->getEntryUnsignedRational(i, 1, &fXResolutionValue);
                }
                break;
            case kYResolutionTag:
                if (!fYResolutionPresent) {
                    fYResolutionPresent = ifd->getEntryUnsignedRational(i, 1, &fYResolutionValue);
                }
                break;
            case kResolutionUnitTag:
                if (!fResolutionUnitPresent) {
                    fResolutionUnitPresent =
                            ifd->getEntryUnsignedShort(i, 1, &fResolutionUnitValue);
                }
                break;
            case kPixelXDimensionTag:
                // The type for this tag can be unsigned short or unsigned long (as per the Exif 2.3
                // spec, aka CIPA DC-008-2012). Support for unsigned long was added in
                // https://crrev.com/817600.
                if (!fPixelXDimensionPresent) {
                    uint16_t value16 = 0;
                    if (ifd->getEntryUnsignedShort(i, 1, &value16)) {
                        fPixelXDimensionValue = value16;
                        fPixelXDimensionPresent = true;
                    }
                }
                if (!fPixelXDimensionPresent) {
                    fPixelXDimensionPresent =
                            ifd->getEntryUnsignedLong(i, 1, &fPixelXDimensionValue);
                }
                break;
            case kPixelYDimensionTag:
                if (!fPixelYDimensionPresent) {
                    uint16_t value16 = 0;
                    if (ifd->getEntryUnsignedShort(i, 1, &value16)) {
                        fPixelYDimensionValue = value16;
                        fPixelYDimensionPresent = true;
                    }
                }
                if (!fPixelYDimensionPresent) {
                    fPixelYDimensionPresent =
                            ifd->getEntryUnsignedLong(i, 1, &fPixelYDimensionValue);
                }
                break;
            default:
                break;
        }
    }
}
