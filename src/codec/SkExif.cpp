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

#include <memory>
#include <utility>

SkExifMetadata::SkExifMetadata(sk_sp<SkData> data) : fData(std::move(data)) {
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
            case kSubIFDOffsetTag: {
                uint32_t subIfdOffset = 0;
                if (isRoot && ifd->getEntryUnsignedLong(i, 1, &subIfdOffset)) {
                    parseIfd(subIfdOffset, littleEndian, /*isRoot=*/false);
                }
                break;
            }
            default:
                break;
        }
    }
}
