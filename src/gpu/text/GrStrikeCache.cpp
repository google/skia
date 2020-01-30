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

GrStrikeCache::GrStrikeCache(const GrCaps* caps, size_t maxTextureBytes)
        : fPreserveStrike(nullptr)
        , f565Masks(SkMasks::CreateMasks({0xF800, 0x07E0, 0x001F, 0},
                    GrMaskFormatBytesPerPixel(kA565_GrMaskFormat))) { }

GrStrikeCache::~GrStrikeCache() {
    this->freeAll();
}

void GrStrikeCache::freeAll() {
    fCache.reset();
}

void GrStrikeCache::evict(GrDrawOpAtlas::PlotLocator) { }

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

static void get_packed_glyph_image(const SkGlyph* glyph, int width,
                                   int height, int dstRB, GrMaskFormat expectedMaskFormat,
                                   void* dst, const SkMasks& masks) {
    SkASSERT(glyph->width() == width);
    SkASSERT(glyph->height() == height);

    const void* src = glyph->image();
    SkASSERT(src != nullptr);

    if (kA565_GrMaskFormat == GrGlyph::FormatFromSkGlyph(glyph->maskFormat()) &&
        kARGB_GrMaskFormat == expectedMaskFormat) {
        // Convert if the glyph uses a 565 mask format since it is using LCD text rendering but the
        // expected format is 8888 (will happen on macOS with Metal since that combination does not
        // support 565).
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
    } else if (GrGlyph::FormatFromSkGlyph(glyph->maskFormat()) != expectedMaskFormat) {
        // crbug:510931
        // Retrieving the image from the cache can actually change the mask format.  This case is
        // very uncommon so for now we just draw a clear box for these glyphs.
        const int bpp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
        for (int y = 0; y < height; y++) {
            sk_bzero(dst, width * bpp);
            dst = (char*)dst + dstRB;
        }
    } else {
        int srcRB = glyph->rowBytes();
        // The windows font host sometimes has BW glyphs in a non-BW strike. So it is important here
        // to check the glyph's format, not the strike's format, and to be able to convert to any
        // of the GrMaskFormats.
        if (glyph->maskFormat() == SkMask::kBW_Format) {
            // expand bits to our mask type
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
        } else if (srcRB == dstRB) {
            memcpy(dst, src, dstRB * height);
        } else {
            const int bbp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
            for (int y = 0; y < height; y++) {
                memcpy(dst, src, width * bbp);
                src = (const char*) src + srcRB;
                dst = (char*) dst + dstRB;
            }
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

void GrTextStrike::removeID(GrDrawOpAtlas::PlotLocator plotLocator) {
    fCache.foreach([this, plotLocator](GrGlyph** glyph){
        if ((*glyph)->fPlotLocator == plotLocator) {
            (*glyph)->fPlotLocator = GrDrawOpAtlas::kInvalidPlotLocator;
            fAtlasedGlyphs--;
            SkASSERT(fAtlasedGlyphs >= 0);
        }
    });
}

GrDrawOpAtlas::ErrorCode GrTextStrike::addGlyphToAtlas(
                                   GrResourceProvider* resourceProvider,
                                   GrDeferredUploadTarget* target,
                                   GrStrikeCache* glyphCache,
                                   GrAtlasManager* fullAtlasManager,
                                   GrGlyph* glyph,
                                   SkBulkGlyphMetricsAndImages* metricsAndImages,
                                   GrMaskFormat expectedMaskFormat,
                                   bool isScaledGlyph) {
    SkASSERT(glyph);
    SkASSERT(metricsAndImages);
    SkASSERT(fCache.findOrNull(glyph->fPackedID));

    expectedMaskFormat = fullAtlasManager->resolveMaskFormat(expectedMaskFormat);
    int bytesPerPixel = GrMaskFormatBytesPerPixel(expectedMaskFormat);

    bool isSDFGlyph =  glyph->maskStyle() == GrGlyph::kDistance_MaskStyle;
    // Add 1 pixel padding around glyph if needed.
    bool addPad = isScaledGlyph && !isSDFGlyph;
    const int width = addPad ? glyph->width() + 2 : glyph->width();
    const int height = addPad ? glyph->height() + 2 : glyph->height();
    int rowBytes = width * bytesPerPixel;
    size_t size = height * rowBytes;

    const SkGlyph* skGlyph = metricsAndImages->glyph(glyph->fPackedID);
    if (skGlyph->image() == nullptr) { return GrDrawOpAtlas::ErrorCode::kError; }

    // Temporary storage for normalizing glyph image.
    SkAutoSMalloc<1024> storage(size);
    void* dataPtr = storage.get();
    if (addPad) {
        sk_bzero(dataPtr, size);
        // Advance in one row and one column.
        dataPtr = (char*)(dataPtr) + rowBytes + bytesPerPixel;
    }

    get_packed_glyph_image(skGlyph, glyph->width(), glyph->height(),
            rowBytes, expectedMaskFormat, dataPtr, glyphCache->getMasks());

    GrDrawOpAtlas::ErrorCode result = fullAtlasManager->addToAtlas(
            resourceProvider, glyphCache, this,
            &glyph->fPlotLocator, target, expectedMaskFormat,
            width, height,
            storage.get(), &glyph->fAtlasLocation);
    if (GrDrawOpAtlas::ErrorCode::kSucceeded == result) {
        if (addPad) {
            glyph->fAtlasLocation.fX += 1;
            glyph->fAtlasLocation.fY += 1;
        }
        SkASSERT(GrDrawOpAtlas::kInvalidPlotLocator != glyph->fPlotLocator);
        fAtlasedGlyphs++;
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

GrGlyph*
GrTextStrike::getGlyph(SkPackedGlyphID packed, SkBulkGlyphMetricsAndImages* metricsAndImages) {
    GrGlyph* grGlyph = fCache.findOrNull(packed);
    if (grGlyph == nullptr) {
        // We could return this to the caller, but in practice it adds code complexity for
        // potentially little benefit(ie, if the glyph is not in our font cache, then its not
        // in the atlas and we're going to be doing a texture upload anyways).
        grGlyph = fAlloc.make<GrGlyph>(*metricsAndImages->glyph(packed));
        fCache.set(grGlyph);
    }
    return grGlyph;
}
