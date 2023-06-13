/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegXmp.h"

#include "include/private/SkGainmapInfo.h"
#include "include/utils/SkParse.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegConstants.h"
#include "src/core/SkMD5.h"
#include "src/xml/SkDOM.h"

#include <string>
#include <tuple>

constexpr size_t kGuidAsciiSize = 32;

/*
 * Extract standard XMP metadata. The decoderApp1Params must outlive the returned SkData.
 *
 * See XMP Specification Part 3: Storage in files, Section 1.1.3: JPEG.
 */
static sk_sp<SkData> read_xmp_standard(const std::vector<sk_sp<SkData>>& decoderApp1Params) {
    constexpr size_t kSigSize = sizeof(kXMPStandardSig);
    // Iterate through the image's segments.
    for (const auto& params : decoderApp1Params) {
        // Skip segments that don't have the right marker, signature, or are too small.
        if (params->size() <= kSigSize) {
            continue;
        }
        if (memcmp(params->bytes(), kXMPStandardSig, kSigSize) != 0) {
            continue;
        }
        return SkData::MakeWithoutCopy(params->bytes() + kSigSize, params->size() - kSigSize);
    }
    return nullptr;
}

/*
 * Extract and validate extended XMP metadata.
 *
 * See XMP Specification Part 3: Storage in files, Section 1.1.3.1: Extended XMP in JPEG:
 * Each chunk is written into the JPEG file within a separate APP1 marker segment. Each ExtendedXMP
 * marker segment contains:
 *   - A null-terminated signature string
 *   - A 128-bit GUID stored as a 32-byte ASCII hex string, capital A-F, no null termination. The
 *     GUID is a 128-bit MD5 digest of the full ExtendedXMP serialization.
 *   - The full length of the ExtendedXMP serialization as a 32-bit unsigned integer.
 *   - The offset of this portion as a 32-bit unsigned integer.
 *   - The portion of the ExtendedXMP
 */
static sk_sp<SkData> read_xmp_extended(const std::vector<sk_sp<SkData>>& decoderApp1Params,
                                       const char* guidAscii) {
    constexpr size_t kSigSize = sizeof(kXMPExtendedSig);
    constexpr size_t kFullLengthSize = 4;
    constexpr size_t kOffsetSize = 4;
    constexpr size_t kHeaderSize = kSigSize + kGuidAsciiSize + kFullLengthSize + kOffsetSize;

    // Validate the provided ASCII guid.
    if (strlen(guidAscii) != kGuidAsciiSize) {
        SkCodecPrintf("Invalid ASCII GUID size.\n");
        return nullptr;
    }
    SkMD5::Digest guidAsDigest;
    for (size_t i = 0; i < kGuidAsciiSize; ++i) {
        uint8_t digit = 0;
        if (guidAscii[i] >= '0' && guidAscii[i] <= '9') {
            digit = guidAscii[i] - '0';
        } else if (guidAscii[i] >= 'A' && guidAscii[i] <= 'F') {
            digit = guidAscii[i] - 'A' + 10;
        } else {
            SkCodecPrintf("GUID is not upper-case hex.\n");
            return nullptr;
        }
        if (i % 2 == 0) {
            guidAsDigest.data[i / 2] = 16 * digit;
        } else {
            guidAsDigest.data[i / 2] += digit;
        }
    }

    // Iterate through the image's segments.
    uint32_t fullLength = 0;
    using Part = std::tuple<uint32_t, sk_sp<SkData>>;
    std::vector<Part> parts;
    for (const auto& params : decoderApp1Params) {
        // Skip segments that don't have the right marker, signature, or are too small.
        if (params->size() <= kHeaderSize) {
            continue;
        }
        if (memcmp(params->bytes(), kXMPExtendedSig, kSigSize) != 0) {
            continue;
        }

        // Ignore parts that do not match the expected GUID.
        const uint8_t* partGuidAscii = params->bytes() + kSigSize;
        if (memcmp(guidAscii, partGuidAscii, kGuidAsciiSize) != 0) {
            SkCodecPrintf("Ignoring unexpected GUID.\n");
            continue;
        }

        // Read the full length and the offset for this part.
        uint32_t partFullLength = 0;
        uint32_t partOffset = 0;
        const uint8_t* partFullLengthBytes = params->bytes() + kSigSize + kGuidAsciiSize;
        const uint8_t* partOffsetBytes =
                params->bytes() + kSigSize + kGuidAsciiSize + kFullLengthSize;
        for (size_t i = 0; i < 4; ++i) {
            partFullLength *= 256;
            partOffset *= 256;
            partFullLength += partFullLengthBytes[i];
            partOffset += partOffsetBytes[i];
        }

        // If this is the first part, set our global full length size.
        if (parts.empty()) {
            fullLength = partFullLength;
        }

        // Ensure all parts agree on the full length.
        if (partFullLength != fullLength) {
            SkCodecPrintf("Multiple parts had different total lengths.\n");
            return nullptr;
        }

        // Add it to the list.
        auto partData = SkData::MakeWithoutCopy(params->bytes() + kHeaderSize,
                                                params->size() - kHeaderSize);
        parts.push_back({partOffset, partData});
    }
    if (parts.empty() || fullLength == 0) {
        return nullptr;
    }

    // Sort the list of parts by offset.
    std::sort(parts.begin(), parts.end(), [](const Part& a, const Part& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    // Stitch the parts together. Fail if we find that they are not contiguous.
    auto xmpExtendedData = SkData::MakeUninitialized(fullLength);
    uint8_t* xmpExtendedBase = reinterpret_cast<uint8_t*>(xmpExtendedData->writable_data());
    uint8_t* xmpExtendedCurrent = xmpExtendedBase;
    SkMD5 md5;
    for (const auto& part : parts) {
        uint32_t currentOffset = static_cast<uint32_t>(xmpExtendedCurrent - xmpExtendedBase);
        uint32_t partOffset = std::get<0>(part);
        const sk_sp<SkData>& partData = std::get<1>(part);
        // Make sure the data is contiguous and doesn't overflow the buffer.
        if (partOffset != currentOffset) {
            SkCodecPrintf("XMP extension parts not contiguous\n");
            return nullptr;
        }
        if (partData->size() > fullLength - currentOffset) {
            SkCodecPrintf("XMP extension parts overflow\n");
            return nullptr;
        }
        memcpy(xmpExtendedCurrent, partData->data(), partData->size());
        xmpExtendedCurrent += partData->size();
    }
    // Make sure we wrote the full buffer.
    if (static_cast<uint32_t>(xmpExtendedCurrent - xmpExtendedBase) != fullLength) {
        SkCodecPrintf("XMP extension did not match full length.\n");
        return nullptr;
    }

    // Make sure the MD5 hash of the extended data matched the GUID.
    md5.write(xmpExtendedData->data(), xmpExtendedData->size());
    if (md5.finish() != guidAsDigest) {
        SkCodecPrintf("XMP extension did not hash to GUID.\n");
        return nullptr;
    }

    return xmpExtendedData;
}

std::unique_ptr<SkXmp> SkJpegMakeXmp(const std::vector<sk_sp<SkData>>& decoderApp1Params) {
    auto xmpStandard = read_xmp_standard(decoderApp1Params);
    if (!xmpStandard) {
        return nullptr;
    }

    std::unique_ptr<SkXmp> xmp = SkXmp::Make(xmpStandard);
    if (!xmp) {
        return nullptr;
    }

    // Extract the GUID (the MD5 hash) of the extended metadata.
    const char* extendedGuid = xmp->getExtendedXmpGuid();
    if (!extendedGuid) {
        return xmp;
    }

    // Extract and validate the extended metadata from the JPEG structure.
    auto xmpExtended = read_xmp_extended(decoderApp1Params, extendedGuid);
    if (!xmpExtended) {
        SkCodecPrintf("Extended XMP was indicated but failed to read or validate.\n");
        return xmp;
    }

    return SkXmp::Make(xmpStandard, xmpExtended);
}
