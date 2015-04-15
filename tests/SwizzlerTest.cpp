/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSwizzler.h"
#include "Test.h"

// These are the values that we will look for to indicate that the fill was successful
static const uint8_t kFillIndex = 0x1;
static const uint32_t kFillColor = 0x22334455;

static void check_fill(skiatest::Reporter* r,
                       const SkImageInfo& imageInfo,
                       uint32_t startRow,
                       uint32_t endRow,
                       size_t rowBytes,
                       uint32_t offset,
                       uint32_t colorOrIndex,
                       SkPMColor* colorTable) {

    // Calculate the total size of the image in bytes.  Use the smallest possible size.
    // The offset value tells us to adjust the pointer from the memory we allocate in order
    // to test on different memory alignments.  If offset is nonzero, we need to increase the
    // size of the memory we allocate in order to make sure that we have enough.  We are
    // still allocating the smallest possible size.
    const size_t totalBytes = imageInfo.getSafeSize(rowBytes) + offset;

    // Create fake image data where every byte has a value of 0
    SkAutoTDeleteArray<uint8_t> storage(SkNEW_ARRAY(uint8_t, totalBytes));
    memset(storage.get(), 0, totalBytes);
    // Adjust the pointer in order to test on different memory alignments
    uint8_t* imageData = storage.get() + offset;
    uint8_t* imageStart = imageData + rowBytes * startRow;

    // Fill image with the fill value starting at the indicated row
    SkSwizzler::Fill(imageStart, imageInfo, rowBytes, endRow - startRow + 1, colorOrIndex,
            colorTable);

    // Ensure that the pixels are filled properly
    // The bots should catch any memory corruption
    uint8_t* indexPtr = imageData + startRow * rowBytes;
    uint8_t* grayPtr = indexPtr;
    uint32_t* colorPtr = (uint32_t*) indexPtr;
    for (uint32_t y = startRow; y <= endRow; y++) {
        for (int32_t x = 0; x < imageInfo.width(); x++) {
            switch (imageInfo.colorType()) {
                case kIndex_8_SkColorType:
                    REPORTER_ASSERT(r, kFillIndex == indexPtr[x]);
                    break;
                case kN32_SkColorType:
                    REPORTER_ASSERT(r, kFillColor == colorPtr[x]);
                    break;
                case kGray_8_SkColorType:
                    // We always fill kGray with black
                    REPORTER_ASSERT(r, (uint8_t) kFillColor == grayPtr[x]);
                    break;
                default:
                    REPORTER_ASSERT(r, false);
                    break;
            }
        }
        indexPtr += rowBytes;
        colorPtr = (uint32_t*) indexPtr;
    }
}

// Test Fill() with different combinations of dimensions, alignment, and padding
DEF_TEST(SwizzlerFill, r) {
    // Set up a color table
    SkPMColor colorTable[kFillIndex + 1];
    colorTable[kFillIndex] = kFillColor;
    // Apart from the fill index, we will leave the other colors in the color table uninitialized.
    // If we incorrectly try to fill with this uninitialized memory, the bots will catch it.

    // Test on an invalid width and representative widths
    const uint32_t widths[] = { 0, 10, 50 };

    // In order to call Fill(), there must be at least one row to fill
    // Test on the smallest possible height and representative heights
    const uint32_t heights[] = { 1, 5, 10 };

    // Test on interesting possibilities for row padding
    const uint32_t paddings[] = { 0, 1, 2, 3, 4 };

    // Iterate over test dimensions
    for (uint32_t width : widths) {
        for (uint32_t height : heights) {

            // Create image info objects
            const SkImageInfo colorInfo = SkImageInfo::MakeN32(width, height,
                kUnknown_SkAlphaType);
            const SkImageInfo indexInfo = colorInfo.makeColorType(kIndex_8_SkColorType);
            const SkImageInfo grayInfo = colorInfo.makeColorType(kGray_8_SkColorType);

            for (uint32_t padding : paddings) {

                // Calculate row bytes
                size_t colorRowBytes = SkColorTypeBytesPerPixel(kN32_SkColorType) * width +
                        padding;
                size_t indexRowBytes = width + padding;
                size_t grayRowBytes = indexRowBytes;

                // If there is padding, we can invent an offset to change the memory alignment
                for (uint32_t offset = 0; offset <= padding; offset++) {

                    // Test all possible start rows with all possible end rows
                    for (uint32_t startRow = 0; startRow < height; startRow++) {
                        for (uint32_t endRow = startRow; endRow < height; endRow++) {

                            // Fill with an index that we use to look up a color
                            check_fill(r, colorInfo, startRow, endRow, colorRowBytes, offset,
                                    kFillIndex, colorTable);

                            // Fill with a color
                            check_fill(r, colorInfo, startRow, endRow, colorRowBytes, offset,
                                    kFillColor, NULL);

                            // Fill with an index
                            check_fill(r, indexInfo, startRow, endRow, indexRowBytes, offset,
                                    kFillIndex, NULL);

                            // Fill a grayscale image
                            check_fill(r, grayInfo, startRow, endRow, grayRowBytes, offset,
                                    kFillColor, NULL);
                        }
                    }
                }
            }
        }
    }
}
