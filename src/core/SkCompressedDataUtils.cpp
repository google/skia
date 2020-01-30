/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCompressedDataUtils.h"

#include "include/core/SkColorPriv.h"
#include "include/core/SkData.h"
#include "include/private/SkColorData.h"
#include "src/core/SkMipMap.h"

struct ETC1Block {
    uint32_t fHigh;
    uint32_t fLow;
};

constexpr uint32_t kFlipBit = 0x1; // set -> T/B sub-blocks; not-set -> L/R sub-blocks
constexpr uint32_t kDiffBit = 0x2; // set -> differential; not-set -> individual

static inline int extend_4To8bits(int b) {
    int c = b & 0xf;
    return (c << 4) | c;
}

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

static int num_4x4_blocks(int size) {
    return ((size + 3) & ~3) >> 2;
}

static bool decompress_etc1(SkISize dimensions, const uint8_t* srcData,
                            SkBitmap* dst) {
    const ETC1Block* srcBlocks = reinterpret_cast<const ETC1Block*>(srcData);

    int numXBlocks = num_4x4_blocks(dimensions.width());
    int numYBlocks = num_4x4_blocks(dimensions.height());

    for (int y = 0; y < numYBlocks; ++y) {
        for (int x = 0; x < numXBlocks; ++x) {
            const ETC1Block* curBlock = &srcBlocks[y * numXBlocks + x];

            bool flipped = SkToBool(curBlock->fHigh & kFlipBit);
            bool differential = SkToBool(curBlock->fHigh & kDiffBit);

            int r1, r2, g1, g2, b1, b2;
            if (differential) {
                r1 = extend_5To8bits(curBlock->fHigh >> 27);
                g1 = extend_5To8bits(curBlock->fHigh >> 19);
                b1 = extend_5To8bits(curBlock->fHigh >> 11);
            } else {
                r1 = extend_4To8bits(curBlock->fHigh >> 28);
                r2 = extend_4To8bits(curBlock->fHigh >> 24);
                g1 = extend_4To8bits(curBlock->fHigh >> 20);
                g2 = extend_4To8bits(curBlock->fHigh >> 16);
                b1 = extend_4To8bits(curBlock->fHigh >> 12);
                b2 = extend_4To8bits(curBlock->fHigh >> 8);
            }

            int tableIndexA = (curBlock->fHigh >> 5) & 0x7;
            int tableIndexB = (curBlock->fHigh >> 2) & 0x7;
            const int* tableA = kETC1ModifierTables[tableIndexA];
            const int* tableB = kETC1ModifierTables[tableIndexB];

            int shift = 0;
            int offsetX = 4 * x, offsetY = 4 * y;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j, shift += 1) {
                    if (offsetX + j >= dst->width() || offsetY + i >= dst->height()) {
                        // This can happen for the topmost levels of a mipmap and for
                        // non-multiple of 4 textures
                        continue;
                    }

                    int index = (curBlock->fLow >> shift) & 0x1 |
                                (curBlock->fLow >> (shift + 15) & 0x2);
                    int delta = tableA[index];
                    *dst->getAddr32(offsetX + j, offsetY + i) = 0x0;
                }
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------
struct BC1Block {
    uint16_t fColor0;
    uint16_t fColor1;
    uint32_t fIndices;
};

static SkPMColor from565(uint16_t rgb565) {
    uint8_t r8 = SkR16ToR32((rgb565 >> 11) & 0x1F);
    uint8_t g8 = SkG16ToG32((rgb565 >> 5) & 0x3F);
    uint8_t b8 = SkB16ToB32(rgb565 & 0x1F);

    return SkPackARGB32(0xFF, r8, g8, b8);
}

// return t*col0 + (1-t)*col1
static SkPMColor lerp(float t, SkPMColor col0, SkPMColor col1) {
    SkASSERT(SkGetPackedA32(col0) == 0xFF && SkGetPackedA32(col1) == 0xFF);

    // TODO: given 't' is only either 1/3 or 2/3 this could be done faster
    uint8_t r8 = SkScalarRoundToInt(t * SkGetPackedR32(col0) + (1.0f - t) * SkGetPackedR32(col1));
    uint8_t g8 = SkScalarRoundToInt(t * SkGetPackedG32(col0) + (1.0f - t) * SkGetPackedG32(col1));
    uint8_t b8 = SkScalarRoundToInt(t * SkGetPackedB32(col0) + (1.0f - t) * SkGetPackedB32(col1));
    return SkPackARGB32(0xFF, r8, g8, b8);
}

static bool decompress_bc1(SkISize dimensions, const uint8_t* srcData,
                           bool isOpaque, SkBitmap* dst) {
    const BC1Block* srcBlocks = reinterpret_cast<const BC1Block*>(srcData);

    int numXBlocks = num_4x4_blocks(dimensions.width());
    int numYBlocks = num_4x4_blocks(dimensions.height());

    SkPMColor colors[4];

    for (int y = 0; y < numYBlocks; ++y) {
        for (int x = 0; x < numXBlocks; ++x) {
            const BC1Block* curBlock = &srcBlocks[y * numXBlocks + x];

            colors[0] = from565(curBlock->fColor0);
            colors[1] = from565(curBlock->fColor1);
            if (isOpaque) {
                colors[2] = lerp(2.0f/3.0f, colors[0], colors[1]);
                colors[3] = lerp(1.0f/3.0f, colors[0], colors[1]);
            } else {
                colors[2] = SkPackARGB32(
                                0xFF,
                                (SkGetPackedR32(colors[0]) + SkGetPackedR32(colors[1])) >> 1,
                                (SkGetPackedG32(colors[0]) + SkGetPackedG32(colors[1])) >> 1,
                                (SkGetPackedB32(colors[0]) + SkGetPackedB32(colors[1])) >> 1);
                colors[3] = SkPackARGB32(0, 0, 0, 0);
            }

            int shift = 0;
            int offsetX = 4 * x, offsetY = 4 * y;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j, shift += 2) {
                    if (offsetX + j >= dst->width() || offsetY + i >= dst->height()) {
                        // This can happen for the topmost levels of a mipmap and for
                        // non-multiple of 4 textures
                        continue;
                    }

                    int index = (curBlock->fIndices >> shift) & 0x3;
                    *dst->getAddr32(offsetX + j, offsetY + i) = colors[index];
                }
            }
        }
    }

    return true;
}

bool SkDecompress(sk_sp<SkData> data,
                  SkISize dimensions,
                  SkImage::CompressionType compressionType,
                  SkBitmap* dst) {
    using Type = SkImage::CompressionType;

    const uint8_t* bytes = data->bytes();
    switch (compressionType) {
        case Type::kNone:            return false;
        case Type::kETC2_RGB8_UNORM: return decompress_etc1(dimensions, bytes, dst);
        case Type::kBC1_RGB8_UNORM:  return decompress_bc1(dimensions, bytes, true, dst);
        case Type::kBC1_RGBA8_UNORM: return decompress_bc1(dimensions, bytes, false, dst);
    }

    SkUNREACHABLE;
    return false;
}

size_t SkCompressedDataSize(SkImage::CompressionType type, SkISize dimensions,
                            SkTArray<size_t>* individualMipOffsets, bool mipMapped) {
    SkASSERT(!individualMipOffsets || !individualMipOffsets->count());

    int numMipLevels = 1;
    if (mipMapped) {
        numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    size_t totalSize = 0;
    switch (type) {
        case SkImage::CompressionType::kNone:
            break;
        case SkImage::CompressionType::kETC2_RGB8_UNORM:
        case SkImage::CompressionType::kBC1_RGB8_UNORM:
        case SkImage::CompressionType::kBC1_RGBA8_UNORM: {
            for (int i = 0; i < numMipLevels; ++i) {
                int numBlocks = num_4x4_blocks(dimensions.width()) *
                                num_4x4_blocks(dimensions.height());

                if (individualMipOffsets) {
                    individualMipOffsets->push_back(totalSize);
                }

                static_assert(sizeof(ETC1Block) == sizeof(BC1Block));
                totalSize += numBlocks * sizeof(ETC1Block);

                dimensions = {SkTMax(1, dimensions.width()/2), SkTMax(1, dimensions.height()/2)};
            }
            break;
        }
    }

    return totalSize;
}

size_t SkCompressedFormatDataSize(SkImage::CompressionType compressionType,
                                  SkISize dimensions, bool mipMapped) {
    return SkCompressedDataSize(compressionType, dimensions, nullptr, mipMapped);
}
