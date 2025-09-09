/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/CompressedTexture.h"

// The following four helpers are copied from src/gpu/DataUtils.cpp to support the test only
// GrTwoColorBC1Compress function. Ideally we would copy the test function into DataUtils.cpp
// instead, but we're currently trying to avoid using the GPU_TEST_UTILS define in src/gpu.

static int num_4x4_blocks(int size) { return ((size + 3) & ~3) >> 2; }

struct BC1Block {
    uint16_t fColor0;
    uint16_t fColor1;
    uint32_t fIndices;
};

static uint16_t to565(SkColor col) {
    int r5 = SkMulDiv255Round(31, SkColorGetR(col));
    int g6 = SkMulDiv255Round(63, SkColorGetG(col));
    int b5 = SkMulDiv255Round(31, SkColorGetB(col));

    return (r5 << 11) | (g6 << 5) | b5;
}

// Create a BC1 compressed block that has two colors but is initialized to 'col0'
static void create_BC1_block(SkColor col0, SkColor col1, BC1Block* block) {
    block->fColor0 = to565(col0);
    block->fColor1 = to565(col1);
    SkASSERT(block->fColor0 <= block->fColor1);  // we always assume transparent blocks

    if (col0 == SK_ColorTRANSPARENT) {
        // This sets all 16 pixels to just use color3 (under the assumption
        // that this is a kBC1_RGBA8_UNORM texture. Note that in this case
        // fColor0 will be opaque black.
        block->fIndices = 0xFFFFFFFF;
    } else {
        // This sets all 16 pixels to just use 'fColor0'
        block->fIndices = 0;
    }
}

namespace sk_gpu_test {
// Fill in 'dstPixels' with BC1 blocks derived from the 'pixmap'.
void TwoColorBC1Compress(const SkPixmap& pixmap, SkColor otherColor, char* dstPixels) {
    BC1Block* dstBlocks = reinterpret_cast<BC1Block*>(dstPixels);
    SkASSERT(pixmap.colorType() == SkColorType::kRGBA_8888_SkColorType);

    BC1Block block;

    // black -> fColor0, otherColor -> fColor1
    create_BC1_block(SK_ColorBLACK, otherColor, &block);

    int numXBlocks = num_4x4_blocks(pixmap.width());
    int numYBlocks = num_4x4_blocks(pixmap.height());

    for (int y = 0; y < numYBlocks; ++y) {
        for (int x = 0; x < numXBlocks; ++x) {
            int shift = 0;
            int offsetX = 4 * x, offsetY = 4 * y;
            block.fIndices = 0;  // init all the pixels to color0 (i.e., opaque black)
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j, shift += 2) {
                    if (offsetX + j >= pixmap.width() || offsetY + i >= pixmap.height()) {
                        // This can happen for the topmost levels of a mipmap and for
                        // non-multiple of 4 textures
                        continue;
                    }

                    SkColor tmp = pixmap.getColor(offsetX + j, offsetY + i);
                    if (tmp == SK_ColorTRANSPARENT) {
                        // For RGBA BC1 images color3 is set to transparent black
                        block.fIndices |= 3 << shift;
                    } else if (tmp != SK_ColorBLACK) {
                        block.fIndices |= 1 << shift;  // color1
                    }
                }
            }

            dstBlocks[y * numXBlocks + x] = block;
        }
    }
}

}  // namespace sk_gpu_test
