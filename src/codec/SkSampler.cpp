/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkSampler.h"
#include "SkUtils.h"

void SkSampler::Fill(const SkImageInfo& info, void* dst, size_t rowBytes,
        uint32_t colorOrIndex, SkCodec::ZeroInitialized zeroInit) {
    SkASSERT(dst != nullptr);

    // Calculate bytes to fill.  We use getSafeSize since the last row may not be padded.
    const size_t bytesToFill = info.getSafeSize(rowBytes);
    const int width = info.width();
    const int numRows = info.height();

    // Use the proper memset routine to fill the remaining bytes
    switch (info.colorType()) {
        case kN32_SkColorType: {
            // If memory is zero initialized, we may not need to fill
            uint32_t color = colorOrIndex;
            if (SkCodec::kYes_ZeroInitialized == zeroInit && 0 == color) {
                return;
            }

            // We must fill row by row in the case of unaligned row bytes
            if (SkIsAlign4((size_t) dst) && SkIsAlign4(rowBytes)) {
                sk_memset32((uint32_t*) dst, color,
                        (uint32_t) bytesToFill / sizeof(SkPMColor));
            } else {
                // We must fill row by row in the case of unaligned row bytes.  This is an
                // unlikely, slow case.
                SkCodecPrintf("Warning: Strange number of row bytes, fill will be slow.\n");
                uint32_t* dstRow = (uint32_t*) dst;
                for (int row = 0; row < numRows; row++) {
                    for (int col = 0; col < width; col++) {
                        dstRow[col] = color;
                    }
                    dstRow = SkTAddOffset<uint32_t>(dstRow, rowBytes);
                }
            }
            break;
        }
        case kRGB_565_SkColorType: {
            // If the destination is k565, the caller passes in a 16-bit color.
            // We will not assert that the high bits of colorOrIndex must be zeroed.
            // This allows us to take advantage of the fact that the low 16 bits of an
            // SKPMColor may be a valid a 565 color.  For example, the low 16
            // bits of SK_ColorBLACK are identical to the 565 representation
            // for black.

            // If memory is zero initialized, we may not need to fill
            uint16_t color = (uint16_t) colorOrIndex;
            if (SkCodec::kYes_ZeroInitialized == zeroInit && 0 == color) {
                return;
            }

            if (SkIsAlign2((size_t) dst) && SkIsAlign2(rowBytes)) {
                sk_memset16((uint16_t*) dst, color, (uint32_t) bytesToFill / sizeof(uint16_t));
            } else {
                // We must fill row by row in the case of unaligned row bytes.  This is an
                // unlikely, slow case.
                SkCodecPrintf("Warning: Strange number of row bytes, fill will be slow.\n");
                uint16_t* dstRow = (uint16_t*) dst;
                for (int row = 0; row < numRows; row++) {
                    for (int col = 0; col < width; col++) {
                        dstRow[col] = color;
                    }
                    dstRow = SkTAddOffset<uint16_t>(dstRow, rowBytes);
                }
            }
            break;
        }
        case kIndex_8_SkColorType:
            // On an index destination color type, always assume the input is an index.
            // Fall through
        case kGray_8_SkColorType:
            // If the destination is kGray, the caller passes in an 8-bit color.
            // We will not assert that the high bits of colorOrIndex must be zeroed.
            // This allows us to take advantage of the fact that the low 8 bits of an
            // SKPMColor may be a valid a grayscale color.  For example, the low 8
            // bits of SK_ColorBLACK are identical to the grayscale representation
            // for black.

            // If memory is zero initialized, we may not need to fill
            if (SkCodec::kYes_ZeroInitialized == zeroInit && 0 == (uint8_t) colorOrIndex) {
                return;
            }

            memset(dst, (uint8_t) colorOrIndex, bytesToFill);
            break;
        default:
            SkCodecPrintf("Error: Unsupported dst color type for fill().  Doing nothing.\n");
            SkASSERT(false);
            break;
    }
}
