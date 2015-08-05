/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchFontCache.h"
#include "GrContext.h"
#include "GrFontAtlasSizes.h"
#include "GrGpu.h"
#include "GrRectanizer.h"
#include "GrResourceProvider.h"
#include "GrSurfacePriv.h"
#include "SkString.h"

#include "SkDistanceFieldGen.h"

///////////////////////////////////////////////////////////////////////////////

bool GrBatchFontCache::initAtlas(GrMaskFormat format) {
    int index = MaskFormatToAtlasIndex(format);
    if (!fAtlases[index]) {
        GrPixelConfig config = MaskFormatToPixelConfig(format);
        int width = GR_FONT_ATLAS_TEXTURE_WIDTH;
        int height = GR_FONT_ATLAS_TEXTURE_HEIGHT;
        int numPlotsX = GR_FONT_ATLAS_NUM_PLOTS_X;
        int numPlotsY = GR_FONT_ATLAS_NUM_PLOTS_Y;

        if (kA8_GrMaskFormat == format) {
            width = GR_FONT_ATLAS_A8_TEXTURE_WIDTH;
            numPlotsX = GR_FONT_ATLAS_A8_NUM_PLOTS_X;
        }
        fAtlases[index] =
                fContext->resourceProvider()->createAtlas(config, width, height,
                                                          numPlotsX, numPlotsY,
                                                          &GrBatchFontCache::HandleEviction,
                                                          (void*)this);
        if (!fAtlases[index]) {
            return false;
        }
    }
    return true;
}

GrBatchFontCache::GrBatchFontCache(GrContext* context)
    : fContext(context)
    , fPreserveStrike(NULL) {
    for (int i = 0; i < kMaskFormatCount; ++i) {
        fAtlases[i] = NULL;
    }
}

GrBatchFontCache::~GrBatchFontCache() {
    SkTDynamicHash<GrBatchTextStrike, GrFontDescKey>::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).fIsAbandoned = true;
        (*iter).unref();
        ++iter;
    }
    for (int i = 0; i < kMaskFormatCount; ++i) {
        SkDELETE(fAtlases[i]);
    }
}

void GrBatchFontCache::freeAll() {
    SkTDynamicHash<GrBatchTextStrike, GrFontDescKey>::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).fIsAbandoned = true;
        (*iter).unref();
        ++iter;
    }
    fCache.rewind();
    for (int i = 0; i < kMaskFormatCount; ++i) {
        SkDELETE(fAtlases[i]);
        fAtlases[i] = NULL;
    }
}

void GrBatchFontCache::HandleEviction(GrBatchAtlas::AtlasID id, void* ptr) {
    GrBatchFontCache* fontCache = reinterpret_cast<GrBatchFontCache*>(ptr);

    SkTDynamicHash<GrBatchTextStrike, GrFontDescKey>::Iter iter(&fontCache->fCache);
    for (; !iter.done(); ++iter) {
        GrBatchTextStrike* strike = &*iter;
        strike->removeID(id);

        // clear out any empty strikes.  We will preserve the strike whose call to addToAtlas
        // triggered the eviction
        if (strike != fontCache->fPreserveStrike && 0 == strike->fAtlasedGlyphs) {
            fontCache->fCache.remove(*(strike->fFontScalerKey));
            strike->fIsAbandoned = true;
            strike->unref();
        }
    }
}

void GrBatchFontCache::dump() const {
    static int gDumpCount = 0;
    for (int i = 0; i < kMaskFormatCount; ++i) {
        if (fAtlases[i]) {
            GrTexture* texture = fAtlases[i]->getTexture();
            if (texture) {
                SkString filename;
#ifdef SK_BUILD_FOR_ANDROID
                filename.printf("/sdcard/fontcache_%d%d.png", gDumpCount, i);
#else
                filename.printf("fontcache_%d%d.png", gDumpCount, i);
#endif
                texture->surfacePriv().savePixels(filename.c_str());
            }
        }
    }
    ++gDumpCount;
}

///////////////////////////////////////////////////////////////////////////////

/*
    The text strike is specific to a given font/style/matrix setup, which is
    represented by the GrHostFontScaler object we are given in getGlyph().

    We map a 32bit glyphID to a GrGlyph record, which in turn points to a
    atlas and a position within that texture.
 */

GrBatchTextStrike::GrBatchTextStrike(GrBatchFontCache* cache, const GrFontDescKey* key)
    : fFontScalerKey(SkRef(key))
    , fPool(9/*start allocations at 512 bytes*/)
    , fAtlasedGlyphs(0)
    , fIsAbandoned(false) {

    fBatchFontCache = cache;     // no need to ref, it won't go away before we do
}

GrBatchTextStrike::~GrBatchTextStrike() {
    SkTDynamicHash<GrGlyph, GrGlyph::PackedID>::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).free();
        ++iter;
    }
}

GrGlyph* GrBatchTextStrike::generateGlyph(const SkGlyph& skGlyph, GrGlyph::PackedID packed,
                                          GrFontScaler* scaler) {
    SkIRect bounds;
    if (GrGlyph::kDistance_MaskStyle == GrGlyph::UnpackMaskStyle(packed)) {
        if (!scaler->getPackedGlyphDFBounds(skGlyph, &bounds)) {
            return NULL;
        }
    } else {
        if (!scaler->getPackedGlyphBounds(skGlyph, &bounds)) {
            return NULL;
        }
    }
    GrMaskFormat format = scaler->getPackedGlyphMaskFormat(skGlyph);

    GrGlyph* glyph = (GrGlyph*)fPool.alloc(sizeof(GrGlyph), SK_MALLOC_THROW);
    glyph->init(packed, bounds, format);
    fCache.add(glyph);
    return glyph;
}

void GrBatchTextStrike::removeID(GrBatchAtlas::AtlasID id) {
    SkTDynamicHash<GrGlyph, GrGlyph::PackedID>::Iter iter(&fCache);
    while (!iter.done()) {
        if (id == (*iter).fID) {
            (*iter).fID = GrBatchAtlas::kInvalidAtlasID;
            fAtlasedGlyphs--;
            SkASSERT(fAtlasedGlyphs >= 0);
        }
        ++iter;
    }
}

bool GrBatchTextStrike::addGlyphToAtlas(GrBatchTarget* batchTarget, GrGlyph* glyph,
                                        GrFontScaler* scaler, const SkGlyph& skGlyph,
                                        GrMaskFormat expectedMaskFormat) {
    SkASSERT(glyph);
    SkASSERT(scaler);
    SkASSERT(fCache.find(glyph->fPackedID));

    SkAutoUnref ar(SkSafeRef(scaler));

    int bytesPerPixel = GrMaskFormatBytesPerPixel(expectedMaskFormat);

    size_t size = glyph->fBounds.area() * bytesPerPixel;
    SkAutoSMalloc<1024> storage(size);

    if (GrGlyph::kDistance_MaskStyle == GrGlyph::UnpackMaskStyle(glyph->fPackedID)) {
        if (!scaler->getPackedGlyphDFImage(skGlyph, glyph->width(), glyph->height(),
                                           storage.get())) {
            return false;
        }
    } else {
        if (!scaler->getPackedGlyphImage(skGlyph, glyph->width(), glyph->height(),
                                         glyph->width() * bytesPerPixel, expectedMaskFormat,
                                         storage.get())) {
            return false;
        }
    }

    bool success = fBatchFontCache->addToAtlas(this, &glyph->fID, batchTarget, expectedMaskFormat,
                                               glyph->width(), glyph->height(),
                                               storage.get(), &glyph->fAtlasLocation);
    if (success) {
        SkASSERT(GrBatchAtlas::kInvalidAtlasID != glyph->fID);
        fAtlasedGlyphs++;
    }
    return success;
}
