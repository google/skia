/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrColor.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDistanceFieldGenFromVector.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"

#include "src/core/SkAutoMalloc.h"
#include "src/core/SkDistanceFieldGen.h"

GrStrikeCache::GrStrikeCache(const GrCaps* caps, size_t maxTextureBytes)
        : fPreserveStrike(nullptr)
        , f565Masks(SkMasks::CreateMasks({0xF800, 0x07E0, 0x001F, 0},
                    GrMaskFormatBytesPerPixel(kA565_GrMaskFormat))) { }

GrStrikeCache::~GrStrikeCache() {
    StrikeHash::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).fIsAbandoned = true;
        (*iter).unref();
        ++iter;
    }
}

void GrStrikeCache::freeAll() {
    StrikeHash::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).fIsAbandoned = true;
        (*iter).unref();
        ++iter;
    }
    fCache.rewind();
}

void GrStrikeCache::HandleEviction(GrDrawOpAtlas::AtlasID id, void* ptr) {
    GrStrikeCache* grStrikeCache = reinterpret_cast<GrStrikeCache*>(ptr);

    StrikeHash::Iter iter(&grStrikeCache->fCache);
    for (; !iter.done(); ++iter) {
        GrTextStrike* strike = &*iter;
        strike->removeID(id);

        // clear out any empty strikes.  We will preserve the strike whose call to addToAtlas
        // triggered the eviction
        if (strike != grStrikeCache->fPreserveStrike && 0 == strike->fAtlasedGlyphs) {
            grStrikeCache->fCache.remove(GrTextStrike::GetKey(*strike));
            strike->fIsAbandoned = true;
            strike->unref();
        }
    }
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

static bool get_packed_glyph_image(SkStrike* cache, const SkGlyph& glyph, int width,
                                   int height, int dstRB, GrMaskFormat expectedMaskFormat,
                                   void* dst, const SkMasks& masks) {
    SkASSERT(glyph.fWidth == width);
    SkASSERT(glyph.fHeight == height);
    const void* src = cache->findImage(glyph);
    if (nullptr == src) {
        return false;
    }

    // Convert if the glyph uses a 565 mask format since it is using LCD text rendering but the
    // expected format is 8888 (will happen on macOS with Metal since that combination does not
    // support 565).
    if (kA565_GrMaskFormat == GrGlyph::FormatFromSkGlyph(glyph) &&
        kARGB_GrMaskFormat == expectedMaskFormat) {
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
        return true;
    }

    // crbug:510931
    // Retrieving the image from the cache can actually change the mask format.  This case is very
    // uncommon so for now we just draw a clear box for these glyphs.
    if (GrGlyph::FormatFromSkGlyph(glyph) != expectedMaskFormat) {
        const int bpp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
        for (int y = 0; y < height; y++) {
            sk_bzero(dst, width * bpp);
            dst = (char*)dst + dstRB;
        }
        return true;
    }

    int srcRB = glyph.rowBytes();
    // The windows font host sometimes has BW glyphs in a non-BW strike. So it is important here to
    // check the glyph's format, not the strike's format, and to be able to convert to any of the
    // GrMaskFormats.
    if (glyph.maskFormat() == SkMask::kBW_Format) {
        // expand bits to our mask type
        const uint8_t* bits = reinterpret_cast<const uint8_t*>(src);
        switch (expectedMaskFormat) {
            case kA8_GrMaskFormat:{
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
            src = (const char*)src + srcRB;
            dst = (char*)dst + dstRB;
        }
    }
    return true;
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

void GrTextStrike::removeID(GrDrawOpAtlas::AtlasID id) {
    SkTDynamicHash<GrGlyph, SkPackedGlyphID>::Iter iter(&fCache);
    while (!iter.done()) {
        if (id == (*iter).fID) {
            (*iter).fID = GrDrawOpAtlas::kInvalidAtlasID;
            fAtlasedGlyphs--;
            SkASSERT(fAtlasedGlyphs >= 0);
        }
        ++iter;
    }
}

GrDrawOpAtlas::ErrorCode GrTextStrike::addGlyphToAtlas(
                                   GrResourceProvider* resourceProvider,
                                   GrDeferredUploadTarget* target,
                                   GrStrikeCache* glyphCache,
                                   GrAtlasManager* fullAtlasManager,
                                   GrGlyph* glyph,
                                   SkStrike* skStrikeCache,
                                   GrMaskFormat expectedMaskFormat,
                                   bool isScaledGlyph) {
    SkASSERT(glyph);
    SkASSERT(skStrikeCache);
    SkASSERT(fCache.find(glyph->fPackedID));

    expectedMaskFormat = fullAtlasManager->resolveMaskFormat(expectedMaskFormat);
    int bytesPerPixel = GrMaskFormatBytesPerPixel(expectedMaskFormat);
    int width = glyph->width();
    int height = glyph->height();
    int rowBytes = width * bytesPerPixel;

    size_t size = glyph->fBounds.area() * bytesPerPixel;
    bool isSDFGlyph = GrGlyph::kDistance_MaskStyle == glyph->maskStyle();
    bool addPad = isScaledGlyph && !isSDFGlyph;
    if (addPad) {
        width += 2;
        rowBytes += 2*bytesPerPixel;
        size += 2 * rowBytes;
        height += 2;
        size += 2 * (height + 2) * bytesPerPixel;
    }
    SkAutoSMalloc<1024> storage(size);

    const SkGlyph& skGlyph = skStrikeCache->getGlyphIDMetrics(glyph->fPackedID);
    void* dataPtr = storage.get();
    if (addPad) {
        sk_bzero(dataPtr, size);
        dataPtr = (char*)(dataPtr) + rowBytes + bytesPerPixel;
    }
    if (!get_packed_glyph_image(skStrikeCache, skGlyph, glyph->width(), glyph->height(),
                                rowBytes, expectedMaskFormat,
                                dataPtr, glyphCache->getMasks())) {
        return GrDrawOpAtlas::ErrorCode::kError;
    }

    GrDrawOpAtlas::ErrorCode result = fullAtlasManager->addToAtlas(
                                                resourceProvider, glyphCache, this,
                                                &glyph->fID, target, expectedMaskFormat,
                                                width, height,
                                                storage.get(), &glyph->fAtlasLocation);
    if (GrDrawOpAtlas::ErrorCode::kSucceeded == result) {
        if (addPad) {
            glyph->fAtlasLocation.fX += 1;
            glyph->fAtlasLocation.fY += 1;
        }
        SkASSERT(GrDrawOpAtlas::kInvalidAtlasID != glyph->fID);
        fAtlasedGlyphs++;
    }
    return result;
}
