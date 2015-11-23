/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchFontCache.h"
#include "GrContext.h"
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
        int width = fAtlasConfigs[index].fWidth;
        int height = fAtlasConfigs[index].fHeight;
        int numPlotsX = fAtlasConfigs[index].numPlotsX();
        int numPlotsY = fAtlasConfigs[index].numPlotsY();

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
    , fPreserveStrike(nullptr) {
    for (int i = 0; i < kMaskFormatCount; ++i) {
        fAtlases[i] = nullptr;
    }

    // setup default atlas configs
    fAtlasConfigs[kA8_GrMaskFormat].fWidth = 2048;
    fAtlasConfigs[kA8_GrMaskFormat].fHeight = 2048;
    fAtlasConfigs[kA8_GrMaskFormat].fPlotWidth = 512;
    fAtlasConfigs[kA8_GrMaskFormat].fPlotHeight = 256;

    fAtlasConfigs[kA565_GrMaskFormat].fWidth = 1024;
    fAtlasConfigs[kA565_GrMaskFormat].fHeight = 2048;
    fAtlasConfigs[kA565_GrMaskFormat].fPlotWidth = 256;
    fAtlasConfigs[kA565_GrMaskFormat].fPlotHeight = 256;

    fAtlasConfigs[kARGB_GrMaskFormat].fWidth = 1024;
    fAtlasConfigs[kARGB_GrMaskFormat].fHeight = 2048;
    fAtlasConfigs[kARGB_GrMaskFormat].fPlotWidth = 256;
    fAtlasConfigs[kARGB_GrMaskFormat].fPlotHeight = 256;
}

GrBatchFontCache::~GrBatchFontCache() {
    SkTDynamicHash<GrBatchTextStrike, GrFontDescKey>::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).fIsAbandoned = true;
        (*iter).unref();
        ++iter;
    }
    for (int i = 0; i < kMaskFormatCount; ++i) {
        delete fAtlases[i];
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
        delete fAtlases[i];
        fAtlases[i] = nullptr;
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

void GrBatchFontCache::setAtlasSizes_ForTesting(const GrBatchAtlasConfig configs[3]) {
    // delete any old atlases, this should be safe to do as long as we are not in the middle of a
    // flush
    for (int i = 0; i < kMaskFormatCount; i++) {
        if (fAtlases[i]) {
            delete fAtlases[i];
            fAtlases[i] = nullptr;
        }
    }
    memcpy(fAtlasConfigs, configs, sizeof(fAtlasConfigs));
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
            return nullptr;
        }
    } else {
        if (!scaler->getPackedGlyphBounds(skGlyph, &bounds)) {
            return nullptr;
        }
    }
    GrMaskFormat format = scaler->getPackedGlyphMaskFormat(skGlyph);

    GrGlyph* glyph = (GrGlyph*)fPool.alloc(sizeof(GrGlyph));
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

bool GrBatchTextStrike::addGlyphToAtlas(GrDrawBatch::Target* target,
                                        GrGlyph* glyph,
                                        GrFontScaler* scaler,
                                        GrMaskFormat expectedMaskFormat) {
    SkASSERT(glyph);
    SkASSERT(scaler);
    SkASSERT(fCache.find(glyph->fPackedID));

    SkAutoUnref ar(SkSafeRef(scaler));

    int bytesPerPixel = GrMaskFormatBytesPerPixel(expectedMaskFormat);

    size_t size = glyph->fBounds.area() * bytesPerPixel;
    SkAutoSMalloc<1024> storage(size);

    const SkGlyph& skGlyph = scaler->grToSkGlyph(glyph->fPackedID);
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

    bool success = fBatchFontCache->addToAtlas(this, &glyph->fID, target, expectedMaskFormat,
                                               glyph->width(), glyph->height(),
                                               storage.get(), &glyph->fAtlasLocation);
    if (success) {
        SkASSERT(GrBatchAtlas::kInvalidAtlasID != glyph->fID);
        fAtlasedGlyphs++;
    }
    return success;
}
