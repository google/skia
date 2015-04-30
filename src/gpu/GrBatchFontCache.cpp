/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchFontCache.h"
#include "GrFontAtlasSizes.h"
#include "GrGpu.h"
#include "GrRectanizer.h"
#include "GrSurfacePriv.h"
#include "SkString.h"

#include "SkDistanceFieldGen.h"

///////////////////////////////////////////////////////////////////////////////

static GrBatchAtlas* make_atlas(GrContext* context, GrPixelConfig config,
                                int textureWidth, int textureHeight,
                                int numPlotsX, int numPlotsY) {
    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = textureWidth;
    desc.fHeight = textureHeight;
    desc.fConfig = config;

    // We don't want to flush the context so we claim we're in the middle of flushing so as to
    // guarantee we do not recieve a texture with pending IO
    GrTexture* texture = context->textureProvider()->refScratchTexture(
        desc, GrTextureProvider::kApprox_ScratchTexMatch, true);
    if (!texture) {
        return NULL;
    }
    return SkNEW_ARGS(GrBatchAtlas, (texture, numPlotsX, numPlotsY));
}

bool GrBatchFontCache::initAtlas(GrMaskFormat format) {
    int index = MaskFormatToAtlasIndex(format);
    if (!fAtlases[index]) {
        GrPixelConfig config = this->getPixelConfig(format);
        if (kA8_GrMaskFormat == format) {
            fAtlases[index] = make_atlas(fContext, config,
                                         GR_FONT_ATLAS_A8_TEXTURE_WIDTH,
                                         GR_FONT_ATLAS_TEXTURE_HEIGHT,
                                         GR_FONT_ATLAS_A8_NUM_PLOTS_X,
                                         GR_FONT_ATLAS_NUM_PLOTS_Y);
        } else {
            fAtlases[index] = make_atlas(fContext, config,
                                         GR_FONT_ATLAS_TEXTURE_WIDTH,
                                         GR_FONT_ATLAS_TEXTURE_HEIGHT,
                                         GR_FONT_ATLAS_NUM_PLOTS_X,
                                         GR_FONT_ATLAS_NUM_PLOTS_Y);
        }

        // Atlas creation can fail
        if (fAtlases[index]) {
            fAtlases[index]->registerEvictionCallback(&GrBatchFontCache::HandleEviction,
                                                      (void*)this);
        } else {
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
        (*iter).unref();
        ++iter;
    }
    fCache.rewind();
    for (int i = 0; i < kMaskFormatCount; ++i) {
        SkDELETE(fAtlases[i]);
        fAtlases[i] = NULL;
    }
}

GrPixelConfig GrBatchFontCache::getPixelConfig(GrMaskFormat format) const {
    static const GrPixelConfig kPixelConfigs[] = {
        kAlpha_8_GrPixelConfig,
        kRGB_565_GrPixelConfig,
        kSkia8888_GrPixelConfig
    };
    SK_COMPILE_ASSERT(SK_ARRAY_COUNT(kPixelConfigs) == kMaskFormatCount, array_size_mismatch);

    return kPixelConfigs[format];
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

GrGlyph* GrBatchTextStrike::generateGlyph(GrGlyph::PackedID packed,
                                          GrFontScaler* scaler) {
    SkIRect bounds;
    if (GrGlyph::kDistance_MaskStyle == GrGlyph::UnpackMaskStyle(packed)) {
        if (!scaler->getPackedGlyphDFBounds(packed, &bounds)) {
            return NULL;
        }
    } else {
        if (!scaler->getPackedGlyphBounds(packed, &bounds)) {
            return NULL;
        }
    }
    GrMaskFormat format = scaler->getPackedGlyphMaskFormat(packed);
    
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
                                        GrFontScaler* scaler) {
    SkASSERT(glyph);
    SkASSERT(scaler);
    SkASSERT(fCache.find(glyph->fPackedID));
    SkASSERT(NULL == glyph->fPlot);

    SkAutoUnref ar(SkSafeRef(scaler));

    int bytesPerPixel = GrMaskFormatBytesPerPixel(glyph->fMaskFormat);

    size_t size = glyph->fBounds.area() * bytesPerPixel;
    GrAutoMalloc<1024> storage(size);

    if (GrGlyph::kDistance_MaskStyle == GrGlyph::UnpackMaskStyle(glyph->fPackedID)) {
        if (!scaler->getPackedGlyphDFImage(glyph->fPackedID, glyph->width(),
                                           glyph->height(),
                                           storage.get())) {
            return false;
        }
    } else {
        if (!scaler->getPackedGlyphImage(glyph->fPackedID, glyph->width(),
                                         glyph->height(),
                                         glyph->width() * bytesPerPixel,
                                         storage.get())) {
            return false;
        }
    }

    bool success = fBatchFontCache->addToAtlas(this, &glyph->fID, batchTarget, glyph->fMaskFormat,
                                               glyph->width(), glyph->height(),
                                               storage.get(), &glyph->fAtlasLocation);
    if (success) {
        fAtlasedGlyphs++;
    }
    return success;
}
