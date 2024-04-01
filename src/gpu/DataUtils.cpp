/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/DataUtils.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTraceEvent.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

using namespace skia_private;

namespace skgpu {

struct ETC1Block {
    uint32_t fHigh;
    uint32_t fLow;
};

constexpr uint32_t kDiffBit = 0x2; // set -> differential; not-set -> individual

static inline int extend_5To8bits(int b) {
    int c = b & 0x1f;
    return (c << 3) | (c >> 2);
}

static const int kNumETC1ModifierTables = 8;
static const int kNumETC1PixelIndices = 4;

// The index of each row in this table is the ETC1 table codeword
// The index of each column in this table is the ETC1 pixel index value
static const int kETC1ModifierTables[kNumETC1ModifierTables][kNumETC1PixelIndices] = {
    /* 0 */ { 2,    8,  -2,   -8 },
    /* 1 */ { 5,   17,  -5,  -17 },
    /* 2 */ { 9,   29,  -9,  -29 },
    /* 3 */ { 13,  42, -13,  -42 },
    /* 4 */ { 18,  60, -18,  -60 },
    /* 5 */ { 24,  80, -24,  -80 },
    /* 6 */ { 33, 106, -33, -106 },
    /* 7 */ { 47, 183, -47, -183 }
};

// Evaluate one of the entries in 'kModifierTables' to see how close it can get (r8,g8,b8) to
// the original color (rOrig, gOrib, bOrig).
static int test_table_entry(int rOrig, int gOrig, int bOrig,
                            int r8, int g8, int b8,
                            int table, int offset) {
    SkASSERT(0 <= table && table < 8);
    SkASSERT(0 <= offset && offset < 4);

    r8 = SkTPin<int>(r8 + kETC1ModifierTables[table][offset], 0, 255);
    g8 = SkTPin<int>(g8 + kETC1ModifierTables[table][offset], 0, 255);
    b8 = SkTPin<int>(b8 + kETC1ModifierTables[table][offset], 0, 255);

    return SkTAbs(rOrig - r8) + SkTAbs(gOrig - g8) + SkTAbs(bOrig - b8);
}

// Create an ETC1 compressed block that is filled with 'col'
static void create_etc1_block(SkColor col, ETC1Block* block) {
    uint32_t high = 0;
    uint32_t low = 0;

    int rOrig = SkColorGetR(col);
    int gOrig = SkColorGetG(col);
    int bOrig = SkColorGetB(col);

    int r5 = SkMulDiv255Round(31, rOrig);
    int g5 = SkMulDiv255Round(31, gOrig);
    int b5 = SkMulDiv255Round(31, bOrig);

    int r8 = extend_5To8bits(r5);
    int g8 = extend_5To8bits(g5);
    int b8 = extend_5To8bits(b5);

    // We always encode solid color textures in differential mode (i.e., with a 555 base color) but
    // with zero diffs (i.e., bits 26-24, 18-16 and 10-8 are left 0).
    high |= (r5 << 27) | (g5 << 19) | (b5 << 11) | kDiffBit;

    int bestTableIndex = 0, bestPixelIndex = 0;
    int bestSoFar = 1024;
    for (int tableIndex = 0; tableIndex < kNumETC1ModifierTables; ++tableIndex) {
        for (int pixelIndex = 0; pixelIndex < kNumETC1PixelIndices; ++pixelIndex) {
            int score = test_table_entry(rOrig, gOrig, bOrig, r8, g8, b8,
                                         tableIndex, pixelIndex);

            if (bestSoFar > score) {
                bestSoFar = score;
                bestTableIndex = tableIndex;
                bestPixelIndex = pixelIndex;
            }
        }
    }

    high |= (bestTableIndex << 5) | (bestTableIndex << 2);

    if (bestPixelIndex & 0x1) {
        low |= 0xFFFF;
    }
    if (bestPixelIndex & 0x2) {
        low |= 0xFFFF0000;
    }

    block->fHigh = SkBSwap32(high);
    block->fLow = SkBSwap32(low);
}

static int num_4x4_blocks(int size) {
    return ((size + 3) & ~3) >> 2;
}

static int num_ETC1_blocks(int w, int h) {
    w = num_4x4_blocks(w);
    h = num_4x4_blocks(h);

    return w * h;
}

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
    SkASSERT(block->fColor0 <= block->fColor1); // we always assume transparent blocks

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

size_t NumCompressedBlocks(SkTextureCompressionType type, SkISize baseDimensions) {
    switch (type) {
        case SkTextureCompressionType::kNone:
            return baseDimensions.width() * baseDimensions.height();
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
        case SkTextureCompressionType::kBC1_RGB8_UNORM:
        case SkTextureCompressionType::kBC1_RGBA8_UNORM: {
            int numBlocksWidth = num_4x4_blocks(baseDimensions.width());
            int numBlocksHeight = num_4x4_blocks(baseDimensions.height());

            return numBlocksWidth * numBlocksHeight;
        }
    }
    SkUNREACHABLE;
}

size_t CompressedRowBytes(SkTextureCompressionType type, int width) {
    switch (type) {
        case SkTextureCompressionType::kNone:
            return 0;
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
        case SkTextureCompressionType::kBC1_RGB8_UNORM:
        case SkTextureCompressionType::kBC1_RGBA8_UNORM: {
            int numBlocksWidth = num_4x4_blocks(width);

            static_assert(sizeof(ETC1Block) == sizeof(BC1Block));
            return numBlocksWidth * sizeof(ETC1Block);
        }
    }
    SkUNREACHABLE;
}

SkISize CompressedDimensions(SkTextureCompressionType type, SkISize baseDimensions) {
    switch (type) {
        case SkTextureCompressionType::kNone:
            return baseDimensions;
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
        case SkTextureCompressionType::kBC1_RGB8_UNORM:
        case SkTextureCompressionType::kBC1_RGBA8_UNORM: {
            SkISize blockDims = CompressedDimensionsInBlocks(type, baseDimensions);
            // Each BC1_RGB8_UNORM and ETC1 block has 16 pixels
            return { 4 * blockDims.fWidth, 4 * blockDims.fHeight };
        }
    }
    SkUNREACHABLE;
}

SkISize CompressedDimensionsInBlocks(SkTextureCompressionType type, SkISize baseDimensions) {
    switch (type) {
        case SkTextureCompressionType::kNone:
            return baseDimensions;
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
        case SkTextureCompressionType::kBC1_RGB8_UNORM:
        case SkTextureCompressionType::kBC1_RGBA8_UNORM: {
            int numBlocksWidth = num_4x4_blocks(baseDimensions.width());
            int numBlocksHeight = num_4x4_blocks(baseDimensions.height());

            // Each BC1_RGB8_UNORM and ETC1 block has 16 pixels
            return { numBlocksWidth, numBlocksHeight };
        }
    }
    SkUNREACHABLE;
}

// Fill in 'dest' with ETC1 blocks derived from 'colorf'
static void fillin_ETC1_with_color(SkISize dimensions, const SkColor4f& colorf, char* dest) {
    SkColor color = colorf.toSkColor();

    ETC1Block block;
    create_etc1_block(color, &block);

    int numBlocks = num_ETC1_blocks(dimensions.width(), dimensions.height());

    for (int i = 0; i < numBlocks; ++i) {
        memcpy(dest, &block, sizeof(ETC1Block));
        dest += sizeof(ETC1Block);
    }
}

// Fill in 'dest' with BC1 blocks derived from 'colorf'
static void fillin_BC1_with_color(SkISize dimensions, const SkColor4f& colorf, char* dest) {
    SkColor color = colorf.toSkColor();

    BC1Block block;
    create_BC1_block(color, color, &block);

    int numBlocks = num_ETC1_blocks(dimensions.width(), dimensions.height());

    for (int i = 0; i < numBlocks; ++i) {
        memcpy(dest, &block, sizeof(BC1Block));
        dest += sizeof(BC1Block);
    }
}

void FillInCompressedData(SkTextureCompressionType type,
                          SkISize dimensions,
                          skgpu::Mipmapped mipmapped,
                          char* dstPixels,
                          const SkColor4f& colorf) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    int numMipLevels = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    size_t offset = 0;

    for (int i = 0; i < numMipLevels; ++i) {
        size_t levelSize = SkCompressedDataSize(type, dimensions, nullptr, false);

        if (SkTextureCompressionType::kETC2_RGB8_UNORM == type) {
            fillin_ETC1_with_color(dimensions, colorf, &dstPixels[offset]);
        } else {
            SkASSERT(type == SkTextureCompressionType::kBC1_RGB8_UNORM ||
                     type == SkTextureCompressionType::kBC1_RGBA8_UNORM);
            fillin_BC1_with_color(dimensions, colorf, &dstPixels[offset]);
        }

        offset += levelSize;
        dimensions = {std::max(1, dimensions.width()/2), std::max(1, dimensions.height()/2)};
    }
}

} // namespace skgpu
