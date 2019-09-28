/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSwizzle.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkOpts.h"
#include "tests/Test.h"

static void check_fill(skiatest::Reporter* r,
                       const SkImageInfo& imageInfo,
                       uint32_t startRow,
                       uint32_t endRow,
                       size_t rowBytes,
                       uint32_t offset) {

    // Calculate the total size of the image in bytes.  Use the smallest possible size.
    // The offset value tells us to adjust the pointer from the memory we allocate in order
    // to test on different memory alignments.  If offset is nonzero, we need to increase the
    // size of the memory we allocate in order to make sure that we have enough.  We are
    // still allocating the smallest possible size.
    const size_t totalBytes = imageInfo.computeByteSize(rowBytes) + offset;

    // Create fake image data where every byte has a value of 0
    std::unique_ptr<uint8_t[]> storage(new uint8_t[totalBytes]);
    memset(storage.get(), 0, totalBytes);
    // Adjust the pointer in order to test on different memory alignments
    uint8_t* imageData = storage.get() + offset;
    uint8_t* imageStart = imageData + rowBytes * startRow;
    const SkImageInfo fillInfo = imageInfo.makeWH(imageInfo.width(), endRow - startRow + 1);
    SkSampler::Fill(fillInfo, imageStart, rowBytes, SkCodec::kNo_ZeroInitialized);

    // Ensure that the pixels are filled properly
    // The bots should catch any memory corruption
    uint8_t* indexPtr = imageData + startRow * rowBytes;
    uint8_t* grayPtr = indexPtr;
    uint32_t* colorPtr = (uint32_t*) indexPtr;
    uint16_t* color565Ptr = (uint16_t*) indexPtr;
    for (uint32_t y = startRow; y <= endRow; y++) {
        for (int32_t x = 0; x < imageInfo.width(); x++) {
            switch (imageInfo.colorType()) {
                case kN32_SkColorType:
                    REPORTER_ASSERT(r, 0 == colorPtr[x]);
                    break;
                case kGray_8_SkColorType:
                    REPORTER_ASSERT(r, 0 == grayPtr[x]);
                    break;
                case kRGB_565_SkColorType:
                    REPORTER_ASSERT(r, 0 == color565Ptr[x]);
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
    // Test on an invalid width and representative widths
    const uint32_t widths[] = { 0, 10, 50 };

    // In order to call Fill(), there must be at least one row to fill
    // Test on the smallest possible height and representative heights
    const uint32_t heights[] = { 1, 5, 10 };

    // Test on interesting possibilities for row padding
    const uint32_t paddings[] = { 0, 4 };

    // Iterate over test dimensions
    for (uint32_t width : widths) {
        for (uint32_t height : heights) {

            // Create image info objects
            const SkImageInfo colorInfo = SkImageInfo::MakeN32(width, height, kUnknown_SkAlphaType);
            const SkImageInfo grayInfo = colorInfo.makeColorType(kGray_8_SkColorType);
            const SkImageInfo color565Info = colorInfo.makeColorType(kRGB_565_SkColorType);

            for (uint32_t padding : paddings) {

                // Calculate row bytes
                const size_t colorRowBytes = SkColorTypeBytesPerPixel(kN32_SkColorType) * width
                        + padding;
                const size_t indexRowBytes = width + padding;
                const size_t grayRowBytes = indexRowBytes;
                const size_t color565RowBytes =
                        SkColorTypeBytesPerPixel(kRGB_565_SkColorType) * width + padding;

                // If there is padding, we can invent an offset to change the memory alignment
                for (uint32_t offset = 0; offset <= padding; offset += 4) {

                    // Test all possible start rows with all possible end rows
                    for (uint32_t startRow = 0; startRow < height; startRow++) {
                        for (uint32_t endRow = startRow; endRow < height; endRow++) {

                            // Test fill with each color type
                            check_fill(r, colorInfo, startRow, endRow, colorRowBytes, offset);
                            check_fill(r, grayInfo, startRow, endRow, grayRowBytes, offset);
                            check_fill(r, color565Info, startRow, endRow, color565RowBytes, offset);
                        }
                    }
                }
            }
        }
    }
}

DEF_TEST(SwizzleOpts, r) {
    uint32_t dst, src;

    // forall c, c*255 == c, c*0 == 0
    for (int c = 0; c <= 255; c++) {
        src = (255<<24) | c;
        SkOpts::RGBA_to_rgbA(&dst, &src, 1);
        REPORTER_ASSERT(r, dst == src);
        SkOpts::RGBA_to_bgrA(&dst, &src, 1);
        REPORTER_ASSERT(r, dst == (uint32_t)((255<<24) | (c<<16)));

        src = (0<<24) | c;
        SkOpts::RGBA_to_rgbA(&dst, &src, 1);
        REPORTER_ASSERT(r, dst == 0);
        SkOpts::RGBA_to_bgrA(&dst, &src, 1);
        REPORTER_ASSERT(r, dst == 0);
    }

    // check a totally arbitrary color
    src = 0xFACEB004;
    SkOpts::RGBA_to_rgbA(&dst, &src, 1);
    REPORTER_ASSERT(r, dst == 0xFACAAD04);

    // swap red and blue
    SkOpts::RGBA_to_BGRA(&dst, &src, 1);
    REPORTER_ASSERT(r, dst == 0xFA04B0CE);

    // all together now
    SkOpts::RGBA_to_bgrA(&dst, &src, 1);
    REPORTER_ASSERT(r, dst == 0xFA04ADCA);
}

DEF_TEST(PublicSwizzleOpts, r) {
    uint32_t dst, src;

    // check a totally arbitrary color
    src = 0xFACEB004;
    SkSwapRB(&dst, &src, 1);
    REPORTER_ASSERT(r, dst == 0xFA04B0CE);
}
