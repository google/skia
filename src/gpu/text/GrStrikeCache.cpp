/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDistanceFieldGenFromVector.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/text/GrStrikeCache.h"

GrStrikeCache::~GrStrikeCache() {
    this->freeAll();
}

void GrStrikeCache::freeAll() {
    fCache.reset();
}

// expands each bit in a bitmask to 0 or ~0 of type INT_TYPE. Used to expand a BW glyph mask to
// A8, RGB565, or RGBA8888.
template <typename INT_TYPE>
static void expand_bits(INT_TYPE* dst,
                        const uint8_t* src,
                        int width,
                        int height,
                        int dstRowBytes,
                        int srcRowBytes) {
    for (int i = 0; i < height; ++i) {
        int rowWritesLeft = width;
        const uint8_t* s = src;
        INT_TYPE* d = dst;
        while (rowWritesLeft > 0) {
            unsigned mask = *s++;
            for (int i = 7; i >= 0 && rowWritesLeft; --i, --rowWritesLeft) {
                *d++ = (mask & (1 << i)) ? (INT_TYPE)(~0UL) : 0;
            }
        }
        dst = reinterpret_cast<INT_TYPE*>(reinterpret_cast<intptr_t>(dst) + dstRowBytes);
        src += srcRowBytes;
    }
}

static void get_packed_glyph_image(
        const SkGlyph& glyph, int dstRB, GrMaskFormat expectedMaskFormat, void* dst) {
    const int width = glyph.width();
    const int height = glyph.height();
    const void* src = glyph.image();
    SkASSERT(src != nullptr);

    GrMaskFormat grMaskFormat = GrGlyph::FormatFromSkGlyph(glyph.maskFormat());
    if (grMaskFormat == expectedMaskFormat) {
        int srcRB = glyph.rowBytes();
        // Notice this comparison is with the glyphs raw mask format, and not its GrMaskFormat.
        if (glyph.maskFormat() != SkMask::kBW_Format) {
            if (srcRB != dstRB) {
                const int bbp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
                for (int y = 0; y < height; y++) {
                    memcpy(dst, src, width * bbp);
                    src = (const char*) src + srcRB;
                    dst = (char*) dst + dstRB;
                }
            } else {
                memcpy(dst, src, dstRB * height);
            }
        } else {
            // Handle 8-bit format by expanding the mask to the expected format.
            const uint8_t* bits = reinterpret_cast<const uint8_t*>(src);
            switch (expectedMaskFormat) {
                case kA8_GrMaskFormat: {
                    uint8_t* bytes = reinterpret_cast<uint8_t*>(dst);
                    expand_bits(bytes, bits, width, height, dstRB, srcRB);
                    break;
                }
                case kA565_GrMaskFormat: {
                    uint16_t* rgb565 = reinterpret_cast<uint16_t*>(dst);
                    expand_bits(rgb565, bits, width, height, dstRB, srcRB);
                    break;
                }
                default:
                    SK_ABORT("Invalid GrMaskFormat");
            }
        }
    } else if (grMaskFormat == kA565_GrMaskFormat && expectedMaskFormat == kARGB_GrMaskFormat) {
        // Convert if the glyph uses a 565 mask format since it is using LCD text rendering
        // but the expected format is 8888 (will happen on macOS with Metal since that
        // combination does not support 565).
        static constexpr SkMasks masks{
                {0b1111'1000'0000'0000, 11, 5},  // Red
                {0b0000'0111'1110'0000,  5, 6},  // Green
                {0b0000'0000'0001'1111,  0, 5},  // Blue
                {0, 0, 0}                        // Alpha
        };
        const int a565Bpp = GrMaskFormatBytesPerPixel(kA565_GrMaskFormat);
        const int argbBpp = GrMaskFormatBytesPerPixel(kARGB_GrMaskFormat);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                uint16_t color565 = 0;
                memcpy(&color565, src, a565Bpp);
                uint32_t colorRGBA = GrColorPackRGBA(masks.getRed(color565),
                                                     masks.getGreen(color565),
                                                     masks.getBlue(color565),
                                                     0xFF);
                memcpy(dst, &colorRGBA, argbBpp);
                src = (char*)src + a565Bpp;
                dst = (char*)dst + argbBpp;
            }
        }
    } else {
        // crbug:510931
        // Retrieving the image from the cache can actually change the mask format. This case is
        // very uncommon so for now we just draw a clear box for these glyphs.
        const int bpp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
        for (int y = 0; y < height; y++) {
            sk_bzero(dst, width * bpp);
            dst = (char*)dst + dstRB;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

/*
    The text strike is specific to a given font/style/matrix setup, which is
    represented by the GrHostFontScaler object we are given in getGlyph().

    We map a 32bit glyphID to a GrGlyph record, which in turn points to a
    atlas and a position within that texture.
 */

GrTextStrike::GrTextStrike(const SkDescriptor& key)
    : fFontScalerKey(key) {}

GrDrawOpAtlas::ErrorCode GrTextStrike::addGlyphToAtlas(const SkGlyph& skGlyph,
                                                       GrMaskFormat expectedMaskFormat,
                                                       bool isScaledGlyph,
                                                       GrResourceProvider* resourceProvider,
                                                       GrDeferredUploadTarget* target,
                                                       GrAtlasManager* fullAtlasManager,
                                                       GrGlyph* grGlyph) {
    SkASSERT(grGlyph != nullptr);
    SkASSERT(fCache.findOrNull(grGlyph->fPackedID));
    SkASSERT(grGlyph->width() == skGlyph.width());
    SkASSERT(grGlyph->height() == skGlyph.height());
    SkASSERT(skGlyph.image() != nullptr);

    expectedMaskFormat = fullAtlasManager->resolveMaskFormat(expectedMaskFormat);
    int bytesPerPixel = GrMaskFormatBytesPerPixel(expectedMaskFormat);

    bool isSDFGlyph = skGlyph.maskFormat() == SkMask::kSDF_Format;
    // Add 1 pixel padding around grGlyph if needed.
    bool addPad = isScaledGlyph && !isSDFGlyph;
    const int width = addPad ? skGlyph.width() + 2 : skGlyph.width();
    const int height = addPad ? skGlyph.height() + 2 : skGlyph.height();
    int rowBytes = width * bytesPerPixel;
    size_t size = height * rowBytes;

    // Temporary storage for normalizing grGlyph image.
    SkAutoSMalloc<1024> storage(size);
    void* dataPtr = storage.get();
    if (addPad) {
        sk_bzero(dataPtr, size);
        // Advance in one row and one column.
        dataPtr = (char*)(dataPtr) + rowBytes + bytesPerPixel;
    }

    get_packed_glyph_image(skGlyph, rowBytes, expectedMaskFormat, dataPtr);

    GrDrawOpAtlas::ErrorCode result = fullAtlasManager->addToAtlas(
            resourceProvider, &grGlyph->fPlotLocator, target, expectedMaskFormat,
            width, height,
            storage.get(), &grGlyph->fAtlasLocation);
    if (GrDrawOpAtlas::ErrorCode::kSucceeded == result) {
        if (addPad) {
            grGlyph->fAtlasLocation.fX += 1;
            grGlyph->fAtlasLocation.fY += 1;
        }
        SkASSERT(grGlyph->fPlotLocator != GrDrawOpAtlas::kInvalidPlotLocator);
    }
    return result;
}

GrGlyph* GrTextStrike::getGlyph(const SkGlyph& skGlyph) {
    GrGlyph* grGlyph = fCache.findOrNull(skGlyph.getPackedID());
    if (grGlyph == nullptr) {
        grGlyph = fAlloc.make<GrGlyph>(skGlyph);
        fCache.set(grGlyph);
    }
    return grGlyph;
}

