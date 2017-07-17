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
        uint64_t colorOrIndex, SkCodec::ZeroInitialized zeroInit) {
    SkASSERT(dst != nullptr);

    // Calculate bytes to fill.  We use getSafeSize since the last row may not be padded.
    const size_t bytesToFill = info.getSafeSize(rowBytes);
    const int width = info.width();
    const int numRows = info.height();

    // Use the proper memset routine to fill the remaining bytes
    switch (info.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType: {
            // If memory is zero initialized, we may not need to fill
            uint32_t color = (uint32_t) colorOrIndex;
            if (SkCodec::kYes_ZeroInitialized == zeroInit && 0 == color) {
                return;
            }

            uint32_t* dstRow = (uint32_t*) dst;
            for (int row = 0; row < numRows; row++) {
                sk_memset32((uint32_t*) dstRow, color, width);
                dstRow = SkTAddOffset<uint32_t>(dstRow, rowBytes);
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

            uint16_t* dstRow = (uint16_t*) dst;
            for (int row = 0; row < numRows; row++) {
                sk_memset16((uint16_t*) dstRow, color, width);
                dstRow = SkTAddOffset<uint16_t>(dstRow, rowBytes);
            }
            break;
        }
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
        case kRGBA_F16_SkColorType: {
            uint64_t color = colorOrIndex;
            if (SkCodec::kYes_ZeroInitialized == zeroInit && 0 == color) {
                return;
            }

            uint64_t* dstRow = (uint64_t*) dst;
            for (int row = 0; row < numRows; row++) {
                sk_memset64((uint64_t*) dstRow, color, width);
                dstRow = SkTAddOffset<uint64_t>(dstRow, rowBytes);
            }
            break;
        }
        default:
            SkCodecPrintf("Error: Unsupported dst color type for fill().  Doing nothing.\n");
            SkASSERT(false);
            break;
    }
}
