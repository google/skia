/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDataUtils.h"

#include "include/private/SkTPin.h"
#include "include/third_party/skcms/skcms.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrImageInfo.h"

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

size_t GrNumBlocks(SkImage::CompressionType type, SkISize baseDimensions) {
    switch (type) {
        case SkImage::CompressionType::kNone:
            return baseDimensions.width() * baseDimensions.height();
        case SkImage::CompressionType::kETC2_RGB8_UNORM:
        case SkImage::CompressionType::kBC1_RGB8_UNORM:
        case SkImage::CompressionType::kBC1_RGBA8_UNORM: {
            int numBlocksWidth = num_4x4_blocks(baseDimensions.width());
            int numBlocksHeight = num_4x4_blocks(baseDimensions.height());

            return numBlocksWidth * numBlocksHeight;
        }
    }
    SkUNREACHABLE;
}

size_t GrCompressedRowBytes(SkImage::CompressionType type, int width) {
    switch (type) {
        case SkImage::CompressionType::kNone:
            return 0;
        case SkImage::CompressionType::kETC2_RGB8_UNORM:
        case SkImage::CompressionType::kBC1_RGB8_UNORM:
        case SkImage::CompressionType::kBC1_RGBA8_UNORM: {
            int numBlocksWidth = num_4x4_blocks(width);

            static_assert(sizeof(ETC1Block) == sizeof(BC1Block));
            return numBlocksWidth * sizeof(ETC1Block);
        }
    }
    SkUNREACHABLE;
}

SkISize GrCompressedDimensions(SkImage::CompressionType type, SkISize baseDimensions) {
    switch (type) {
        case SkImage::CompressionType::kNone:
            return baseDimensions;
        case SkImage::CompressionType::kETC2_RGB8_UNORM:
        case SkImage::CompressionType::kBC1_RGB8_UNORM:
        case SkImage::CompressionType::kBC1_RGBA8_UNORM: {
            int numBlocksWidth = num_4x4_blocks(baseDimensions.width());
            int numBlocksHeight = num_4x4_blocks(baseDimensions.height());

            // Each BC1_RGB8_UNORM and ETC1 block has 16 pixels
            return { 4 * numBlocksWidth, 4 * numBlocksHeight };
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

#if GR_TEST_UTILS

// Fill in 'dstPixels' with BC1 blocks derived from the 'pixmap'.
void GrTwoColorBC1Compress(const SkPixmap& pixmap, SkColor otherColor, char* dstPixels) {
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
                        block.fIndices |= 1 << shift; // color1
                    }
                }
            }

            dstBlocks[y*numXBlocks + x] = block;
        }
    }
}

#endif

size_t GrComputeTightCombinedBufferSize(size_t bytesPerPixel, SkISize baseDimensions,
                                        SkTArray<size_t>* individualMipOffsets, int mipLevelCount) {
    SkASSERT(individualMipOffsets && !individualMipOffsets->count());
    SkASSERT(mipLevelCount >= 1);

    individualMipOffsets->push_back(0);

    size_t combinedBufferSize = baseDimensions.width() * bytesPerPixel * baseDimensions.height();
    SkISize levelDimensions = baseDimensions;

    // The Vulkan spec for copying a buffer to an image requires that the alignment must be at
    // least 4 bytes and a multiple of the bytes per pixel of the image config.
    SkASSERT(bytesPerPixel == 1 || bytesPerPixel == 2 || bytesPerPixel == 3 ||
             bytesPerPixel == 4 || bytesPerPixel == 8 || bytesPerPixel == 16);
    int desiredAlignment = (bytesPerPixel == 3) ? 12 : (bytesPerPixel > 4 ? bytesPerPixel : 4);

    for (int currentMipLevel = 1; currentMipLevel < mipLevelCount; ++currentMipLevel) {
        levelDimensions = {std::max(1, levelDimensions.width() /2),
                           std::max(1, levelDimensions.height()/2)};

        size_t trimmedSize = levelDimensions.area() * bytesPerPixel;
        const size_t alignmentDiff = combinedBufferSize % desiredAlignment;
        if (alignmentDiff != 0) {
            combinedBufferSize += desiredAlignment - alignmentDiff;
        }
        SkASSERT((0 == combinedBufferSize % 4) && (0 == combinedBufferSize % bytesPerPixel));

        individualMipOffsets->push_back(combinedBufferSize);
        combinedBufferSize += trimmedSize;
    }

    SkASSERT(individualMipOffsets->count() == mipLevelCount);
    return combinedBufferSize;
}

void GrFillInCompressedData(SkImage::CompressionType type, SkISize dimensions,
                            GrMipmapped mipMapped, char* dstPixels, const SkColor4f& colorf) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    int numMipLevels = 1;
    if (mipMapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    size_t offset = 0;

    for (int i = 0; i < numMipLevels; ++i) {
        size_t levelSize = SkCompressedDataSize(type, dimensions, nullptr, false);

        if (SkImage::CompressionType::kETC2_RGB8_UNORM == type) {
            fillin_ETC1_with_color(dimensions, colorf, &dstPixels[offset]);
        } else {
            SkASSERT(type == SkImage::CompressionType::kBC1_RGB8_UNORM ||
                     type == SkImage::CompressionType::kBC1_RGBA8_UNORM);
            fillin_BC1_with_color(dimensions, colorf, &dstPixels[offset]);
        }

        offset += levelSize;
        dimensions = {std::max(1, dimensions.width()/2), std::max(1, dimensions.height()/2)};
    }
}

static GrSwizzle get_load_and_src_swizzle(GrColorType ct, SkRasterPipeline::StockStage* load,
                                          bool* isNormalized, bool* isSRGB) {
    GrSwizzle swizzle("rgba");
    *isNormalized = true;
    *isSRGB = false;
    switch (ct) {
        case GrColorType::kAlpha_8:          *load = SkRasterPipeline::load_a8;       break;
        case GrColorType::kAlpha_16:         *load = SkRasterPipeline::load_a16;      break;
        case GrColorType::kBGR_565:          *load = SkRasterPipeline::load_565;      break;
        case GrColorType::kABGR_4444:        *load = SkRasterPipeline::load_4444;     break;
        case GrColorType::kARGB_4444:        swizzle = GrSwizzle("bgra");
                                             *load = SkRasterPipeline::load_4444;     break;
        case GrColorType::kBGRA_4444:        swizzle = GrSwizzle("gbar");
                                             *load = SkRasterPipeline::load_4444;     break;
        case GrColorType::kRGBA_8888:        *load = SkRasterPipeline::load_8888;     break;
        case GrColorType::kRG_88:            *load = SkRasterPipeline::load_rg88;     break;
        case GrColorType::kRGBA_1010102:     *load = SkRasterPipeline::load_1010102;  break;
        case GrColorType::kBGRA_1010102:     *load = SkRasterPipeline::load_1010102;
                                             swizzle = GrSwizzle("bgra");
                                             break;
        case GrColorType::kAlpha_F16:        *load = SkRasterPipeline::load_af16;     break;
        case GrColorType::kRGBA_F16_Clamped: *load = SkRasterPipeline::load_f16;      break;
        case GrColorType::kRG_1616:          *load = SkRasterPipeline::load_rg1616;   break;
        case GrColorType::kRGBA_16161616:    *load = SkRasterPipeline::load_16161616; break;

        case GrColorType::kRGBA_8888_SRGB:   *load = SkRasterPipeline::load_8888;
                                             *isSRGB = true;
                                             break;
        case GrColorType::kRG_F16:           *load = SkRasterPipeline::load_rgf16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F16:         *load = SkRasterPipeline::load_f16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F32:         *load = SkRasterPipeline::load_f32;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_8xxx:       *load = SkRasterPipeline::load_8888;
                                             swizzle = GrSwizzle("000r");
                                             break;
        case GrColorType::kAlpha_F32xxx:     *load = SkRasterPipeline::load_f32;
                                             swizzle = GrSwizzle("000r");
                                             break;
        case GrColorType::kGray_8xxx:       *load = SkRasterPipeline::load_8888;
                                             swizzle = GrSwizzle("rrr1");
                                             break;
        case GrColorType::kGray_8:           *load = SkRasterPipeline::load_a8;
                                             swizzle = GrSwizzle("aaa1");
                                             break;
        case GrColorType::kGrayAlpha_88:    *load = SkRasterPipeline::load_rg88;
                                             swizzle = GrSwizzle("rrrg");
                                             break;
        case GrColorType::kBGRA_8888:        *load = SkRasterPipeline::load_8888;
                                             swizzle = GrSwizzle("bgra");
                                             break;
        case GrColorType::kRGB_888x:         *load = SkRasterPipeline::load_8888;
                                             swizzle = GrSwizzle("rgb1");
                                             break;

        // These are color types we don't expect to ever have to load.
        case GrColorType::kRGB_888:
        case GrColorType::kR_8:
        case GrColorType::kR_16:
        case GrColorType::kR_F16:
        case GrColorType::kGray_F16:
        case GrColorType::kUnknown:
            SK_ABORT("unexpected CT");
    }
    return swizzle;
}

enum class LumMode {
    kNone,
    kToRGB,
    kToAlpha
};

static GrSwizzle get_dst_swizzle_and_store(GrColorType ct, SkRasterPipeline::StockStage* store,
                                           LumMode* lumMode, bool* isNormalized, bool* isSRGB) {
    GrSwizzle swizzle("rgba");
    *isNormalized = true;
    *isSRGB = false;
    *lumMode = LumMode::kNone;
    switch (ct) {
        case GrColorType::kAlpha_8:          *store = SkRasterPipeline::store_a8;       break;
        case GrColorType::kAlpha_16:         *store = SkRasterPipeline::store_a16;      break;
        case GrColorType::kBGR_565:          *store = SkRasterPipeline::store_565;      break;
        case GrColorType::kABGR_4444:        *store = SkRasterPipeline::store_4444;     break;
        case GrColorType::kARGB_4444:        swizzle = GrSwizzle("bgra");
                                             *store = SkRasterPipeline::store_4444;     break;
        case GrColorType::kBGRA_4444:        swizzle = GrSwizzle("argb");
                                             *store = SkRasterPipeline::store_4444;     break;
        case GrColorType::kRGBA_8888:        *store = SkRasterPipeline::store_8888;     break;
        case GrColorType::kRG_88:            *store = SkRasterPipeline::store_rg88;     break;
        case GrColorType::kRGBA_1010102:     *store = SkRasterPipeline::store_1010102;  break;
        case GrColorType::kBGRA_1010102:     swizzle = GrSwizzle("bgra");
                                             *store = SkRasterPipeline::store_1010102;
                                             break;
        case GrColorType::kRGBA_F16_Clamped: *store = SkRasterPipeline::store_f16;      break;
        case GrColorType::kRG_1616:          *store = SkRasterPipeline::store_rg1616;   break;
        case GrColorType::kRGBA_16161616:    *store = SkRasterPipeline::store_16161616; break;

        case GrColorType::kRGBA_8888_SRGB:   *store = SkRasterPipeline::store_8888;
                                             *isSRGB = true;
                                             break;
        case GrColorType::kRG_F16:           *store = SkRasterPipeline::store_rgf16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_F16:        *store = SkRasterPipeline::store_af16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F16:         *store = SkRasterPipeline::store_f16;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kRGBA_F32:         *store = SkRasterPipeline::store_f32;
                                             *isNormalized = false;
                                             break;
        case GrColorType::kAlpha_8xxx:       *store = SkRasterPipeline::store_8888;
                                             swizzle = GrSwizzle("a000");
                                             break;
        case GrColorType::kAlpha_F32xxx:     *store = SkRasterPipeline::store_f32;
                                             swizzle = GrSwizzle("a000");
                                             break;
        case GrColorType::kBGRA_8888:        swizzle = GrSwizzle("bgra");
                                             *store = SkRasterPipeline::store_8888;
                                             break;
        case GrColorType::kRGB_888x:         swizzle = GrSwizzle("rgb1");
                                             *store = SkRasterPipeline::store_8888;
                                             break;
        case GrColorType::kR_8:              swizzle = GrSwizzle("agbr");
                                             *store = SkRasterPipeline::store_a8;
                                             break;
        case GrColorType::kR_16:             swizzle = GrSwizzle("agbr");
                                             *store = SkRasterPipeline::store_a16;
                                             break;
        case GrColorType::kR_F16:            swizzle = GrSwizzle("agbr");
                                             *store = SkRasterPipeline::store_af16;
                                             break;
        case GrColorType::kGray_F16:         *lumMode = LumMode::kToAlpha;
                                             *store = SkRasterPipeline::store_af16;
                                             break;
        case GrColorType::kGray_8:           *lumMode = LumMode::kToAlpha;
                                             *store = SkRasterPipeline::store_a8;
                                             break;
        case GrColorType::kGrayAlpha_88:     *lumMode = LumMode::kToRGB;
                                             swizzle = GrSwizzle("ragb");
                                             *store = SkRasterPipeline::store_rg88;
                                             break;
        case GrColorType::kGray_8xxx:        *lumMode = LumMode::kToRGB;
                                             *store = SkRasterPipeline::store_8888;
                                             swizzle = GrSwizzle("r000");
                                             break;

        // These are color types we don't expect to ever have to store.
        case GrColorType::kRGB_888:  // This is handled specially in GrConvertPixels.
        case GrColorType::kUnknown:
            SK_ABORT("unexpected CT");
    }
    return swizzle;
}

static inline void append_clamp_gamut(SkRasterPipeline* pipeline) {
    // SkRasterPipeline may not know our color type and also doesn't like caller to directly
    // append clamp_gamut. Fake it out.
    static SkImageInfo fakeII = SkImageInfo::MakeN32Premul(1, 1);
    pipeline->append_gamut_clamp_if_normalized(fakeII);
}

bool GrConvertPixels(const GrImageInfo& dstInfo,       void* dst, size_t dstRB,
                     const GrImageInfo& srcInfo, const void* src, size_t srcRB,
                     bool flipY) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (srcInfo.colorType() == GrColorType::kRGB_888) {
        // We don't expect to have to convert from this format.
        return false;
    }
    if (srcInfo.dimensions().isEmpty() || dstInfo.dimensions().isEmpty()) {
        return false;
    }
    if (srcInfo.colorType() == GrColorType::kUnknown ||
        dstInfo.colorType() == GrColorType::kUnknown) {
        return false;
    }
    if (!src || !dst) {
        return false;
    }
    if (dstInfo.dimensions() != srcInfo.dimensions()) {
        return false;
    }
    if (dstRB < dstInfo.minRowBytes() || srcRB < srcInfo.minRowBytes()) {
        return false;
    }
    if (dstInfo.colorType() == GrColorType::kRGB_888) {
        // SkRasterPipeline doesn't handle writing to RGB_888. So we have it write to RGB_888x and
        // then do another conversion that does the 24bit packing.
        auto tempDstInfo = dstInfo.makeColorType(GrColorType::kRGB_888x);
        auto tempRB = tempDstInfo.minRowBytes();
        std::unique_ptr<char[]> tempDst(new char[tempRB * tempDstInfo.height()]);
        if (!GrConvertPixels(tempDstInfo, tempDst.get(), tempRB, srcInfo, src, srcRB, flipY)) {
            return false;
        }
        auto* tRow = reinterpret_cast<const char*>(tempDst.get());
        auto* dRow = reinterpret_cast<char*>(dst);
        for (int y = 0; y < dstInfo.height(); ++y, tRow += tempRB, dRow += dstRB) {
            for (int x = 0; x < dstInfo.width(); ++x) {
                auto t = reinterpret_cast<const uint32_t*>(tRow + x * sizeof(uint32_t));
                auto d = reinterpret_cast<uint32_t*>(dRow + x * 3);
                memcpy(d, t, 3);
            }
        }
        return true;
    }

    size_t srcBpp = srcInfo.bpp();
    size_t dstBpp = dstInfo.bpp();

    // SkRasterPipeline operates on row-pixels not row-bytes.
    SkASSERT(dstRB % dstBpp == 0);
    SkASSERT(srcRB % srcBpp == 0);

    bool premul   = srcInfo.alphaType() == kUnpremul_SkAlphaType &&
                    dstInfo.alphaType() == kPremul_SkAlphaType;
    bool unpremul = srcInfo.alphaType() == kPremul_SkAlphaType &&
                    dstInfo.alphaType() == kUnpremul_SkAlphaType;
    bool alphaOrCSConversion =
            premul || unpremul || !SkColorSpace::Equals(srcInfo.colorSpace(), dstInfo.colorSpace());

    if (srcInfo.colorType() == dstInfo.colorType() && !alphaOrCSConversion) {
        size_t tightRB = dstBpp * dstInfo.width();
        if (flipY) {
            dst = static_cast<char*>(dst) + dstRB * (dstInfo.height() - 1);
            for (int y = 0; y < dstInfo.height(); ++y) {
                memcpy(dst, src, tightRB);
                src = static_cast<const char*>(src) + srcRB;
                dst = static_cast<      char*>(dst) - dstRB;
            }
        } else {
            SkRectMemcpy(dst, dstRB, src, srcRB, tightRB, srcInfo.height());
        }
        return true;
    }

    SkRasterPipeline::StockStage load;
    bool srcIsNormalized;
    bool srcIsSRGB;
    auto loadSwizzle =
            get_load_and_src_swizzle(srcInfo.colorType(), &load, &srcIsNormalized, &srcIsSRGB);

    SkRasterPipeline::StockStage store;
    LumMode lumMode;
    bool dstIsNormalized;
    bool dstIsSRGB;
    auto storeSwizzle = get_dst_swizzle_and_store(dstInfo.colorType(), &store, &lumMode,
                                                  &dstIsNormalized, &dstIsSRGB);

    bool clampGamut;
    SkTLazy<SkColorSpaceXformSteps> steps;
    GrSwizzle loadStoreSwizzle;
    if (alphaOrCSConversion) {
        steps.init(srcInfo.colorSpace(), srcInfo.alphaType(),
                   dstInfo.colorSpace(), dstInfo.alphaType());
        clampGamut = dstIsNormalized && dstInfo.alphaType() == kPremul_SkAlphaType;
    } else {
        clampGamut =
                dstIsNormalized && !srcIsNormalized && dstInfo.alphaType() == kPremul_SkAlphaType;
        if (!clampGamut) {
            loadStoreSwizzle = GrSwizzle::Concat(loadSwizzle, storeSwizzle);
        }
    }
    int cnt = 1;
    int height = srcInfo.height();
    SkRasterPipeline_MemoryCtx srcCtx{const_cast<void*>(src), SkToInt(srcRB / srcBpp)},
                               dstCtx{                  dst , SkToInt(dstRB / dstBpp)};

    if (flipY) {
        // It *almost* works to point the src at the last row and negate the stride and run the
        // whole rectangle. However, SkRasterPipeline::run()'s control loop uses size_t loop
        // variables so it winds up relying on unsigned overflow math. It works out in practice
        // but UBSAN says "no!" as it's technically undefined and in theory a compiler could emit
        // code that didn't do what is intended. So we go one row at a time. :(
        srcCtx.pixels = static_cast<char*>(srcCtx.pixels) + srcRB * (height - 1);
        std::swap(cnt, height);
    }

    bool hasConversion = alphaOrCSConversion || clampGamut || lumMode != LumMode::kNone;

    if (srcIsSRGB && dstIsSRGB && !hasConversion) {
        // No need to convert from srgb if we are just going to immediately convert it back.
        srcIsSRGB = dstIsSRGB = false;
    }

    hasConversion = hasConversion || srcIsSRGB || dstIsSRGB;

    for (int i = 0; i < cnt; ++i) {
        SkRasterPipeline_<256> pipeline;
        pipeline.append(load, &srcCtx);
        if (hasConversion) {
            loadSwizzle.apply(&pipeline);
            if (srcIsSRGB) {
                pipeline.append_transfer_function(*skcms_sRGB_TransferFunction());
            }
            if (alphaOrCSConversion) {
                steps->apply(&pipeline);
            }
            if (clampGamut) {
                append_clamp_gamut(&pipeline);
            }
            switch (lumMode) {
                case LumMode::kNone:
                    break;
                case LumMode::kToRGB:
                    pipeline.append(SkRasterPipeline::StockStage::bt709_luminance_or_luma_to_rgb);
                    break;
                case LumMode::kToAlpha:
                    pipeline.append(SkRasterPipeline::StockStage::bt709_luminance_or_luma_to_alpha);
                    // If we ever need to store srgb-encoded gray (e.g. GL_SLUMINANCE8) then we
                    // should use ToRGB and then a swizzle stage rather than ToAlpha. The subsequent
                    // transfer function stage ignores the alpha channel (where we just stashed the
                    // gray).
                    SkASSERT(!dstIsSRGB);
                    break;
            }
            if (dstIsSRGB) {
                pipeline.append_transfer_function(*skcms_sRGB_Inverse_TransferFunction());
            }
            storeSwizzle.apply(&pipeline);
        } else {
            loadStoreSwizzle.apply(&pipeline);
        }
        pipeline.append(store, &dstCtx);
        pipeline.run(0, 0, srcInfo.width(), height);
        srcCtx.pixels = static_cast<char*>(srcCtx.pixels) - srcRB;
        dstCtx.pixels = static_cast<char*>(dstCtx.pixels) + dstRB;
    }
    return true;
}

bool GrClearImage(const GrImageInfo& dstInfo, void* dst, size_t dstRB, SkColor4f color) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    if (!dstInfo.isValid()) {
        return false;
    }
    if (!dst) {
        return false;
    }
    if (dstRB < dstInfo.minRowBytes()) {
        return false;
    }
    if (dstInfo.colorType() == GrColorType::kRGB_888) {
        // SkRasterPipeline doesn't handle writing to RGB_888. So we handle that specially here.
        uint32_t rgba = color.toBytes_RGBA();
        for (int y = 0; y < dstInfo.height(); ++y) {
            char* d = static_cast<char*>(dst) + y * dstRB;
            for (int x = 0; x < dstInfo.width(); ++x, d += 3) {
                memcpy(d, &rgba, 3);
            }
        }
        return true;
    }

    LumMode lumMode;
    bool isNormalized;
    bool dstIsSRGB;
    SkRasterPipeline::StockStage store;
    GrSwizzle storeSwizzle = get_dst_swizzle_and_store(dstInfo.colorType(), &store, &lumMode,
                                                       &isNormalized, &dstIsSRGB);
    char block[64];
    SkArenaAlloc alloc(block, sizeof(block), 1024);
    SkRasterPipeline_<256> pipeline;
    pipeline.append_constant_color(&alloc, color);
    switch (lumMode) {
        case LumMode::kNone:
            break;
        case LumMode::kToRGB:
            pipeline.append(SkRasterPipeline::StockStage::bt709_luminance_or_luma_to_rgb);
            break;
        case LumMode::kToAlpha:
            pipeline.append(SkRasterPipeline::StockStage::bt709_luminance_or_luma_to_alpha);
            // If we ever need to store srgb-encoded gray (e.g. GL_SLUMINANCE8) then we should use
            // ToRGB and then a swizzle stage rather than ToAlpha. The subsequent transfer function
            // stage ignores the alpha channel (where we just stashed the gray).
            SkASSERT(!dstIsSRGB);
            break;
    }
    if (dstIsSRGB) {
        pipeline.append_transfer_function(*skcms_sRGB_Inverse_TransferFunction());
    }
    storeSwizzle.apply(&pipeline);
    SkRasterPipeline_MemoryCtx dstCtx{dst, SkToInt(dstRB/dstInfo.bpp())};
    pipeline.append(store, &dstCtx);
    pipeline.run(0, 0, dstInfo.width(), dstInfo.height());

    return true;
}

GrColorType SkColorTypeAndFormatToGrColorType(const GrCaps* caps,
                                              SkColorType skCT,
                                              const GrBackendFormat& format) {
    GrColorType grCT = SkColorTypeToGrColorType(skCT);
    // Until we support SRGB in the SkColorType we have to do this manual check here to make sure
    // we use the correct GrColorType.
    if (caps->isFormatSRGB(format)) {
        if (grCT != GrColorType::kRGBA_8888) {
            return GrColorType::kUnknown;
        }
        grCT = GrColorType::kRGBA_8888_SRGB;
    }
    return grCT;
}
