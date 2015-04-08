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
    GrTexture* texture = context->refScratchTexture(desc, GrContext::kApprox_ScratchTexMatch, true);
    if (!texture) {
        return NULL;
    }
    return SkNEW_ARGS(GrBatchAtlas, (texture, numPlotsX, numPlotsY));
}

int GrBatchFontCache::MaskFormatToAtlasIndex(GrMaskFormat format) {
    static const int sAtlasIndices[] = {
        kA8_GrMaskFormat,
        kA565_GrMaskFormat,
        kARGB_GrMaskFormat,
    };
    SK_COMPILE_ASSERT(SK_ARRAY_COUNT(sAtlasIndices) == kMaskFormatCount, array_size_mismatch);

    SkASSERT(sAtlasIndices[format] < kMaskFormatCount);
    return sAtlasIndices[format];
}

GrMaskFormat GrBatchFontCache::AtlasIndexToMaskFormat(int atlasIndex) {
    static GrMaskFormat sMaskFormats[] = {
        kA8_GrMaskFormat,
        kA565_GrMaskFormat,
        kARGB_GrMaskFormat,
    };
    SK_COMPILE_ASSERT(SK_ARRAY_COUNT(sMaskFormats) == kMaskFormatCount, array_size_mismatch);

    SkASSERT(sMaskFormats[atlasIndex] < kMaskFormatCount);
    return sMaskFormats[atlasIndex];
}

GrBatchFontCache::GrBatchFontCache()
    : fPreserveStrike(NULL) {
}

void GrBatchFontCache::init(GrContext* context) {
    for (int i = 0; i < kMaskFormatCount; i++) {
        GrMaskFormat format = AtlasIndexToMaskFormat(i);
        GrPixelConfig config = this->getPixelConfig(format);

        if (kA8_GrMaskFormat == format) {
            fAtlases[i] = make_atlas(context, config,
                                     GR_FONT_ATLAS_A8_TEXTURE_WIDTH,
                                     GR_FONT_ATLAS_TEXTURE_HEIGHT,
                                     GR_FONT_ATLAS_A8_NUM_PLOTS_X,
                                     GR_FONT_ATLAS_NUM_PLOTS_Y);
        } else {
            fAtlases[i] = make_atlas(context, config,
                                     GR_FONT_ATLAS_TEXTURE_WIDTH,
                                     GR_FONT_ATLAS_TEXTURE_HEIGHT,
                                     GR_FONT_ATLAS_NUM_PLOTS_X,
                                     GR_FONT_ATLAS_NUM_PLOTS_Y);
        }

        if (fAtlases[i]) {
            fAtlases[i]->registerEvictionCallback(&GrBatchFontCache::HandleEviction, (void*)this);
        }
    }
}

GrBatchFontCache::~GrBatchFontCache() {
    SkTDynamicHash<GrBatchTextStrike, GrFontDescKey>::Iter iter(&fCache);
    while (!iter.done()) {
        SkDELETE(&(*iter));
        ++iter;
    }
    for (int i = 0; i < kMaskFormatCount; ++i) {
        SkDELETE(fAtlases[i]);
    }
}

GrBatchTextStrike* GrBatchFontCache::generateStrike(GrFontScaler* scaler) {
    GrBatchTextStrike* strike = SkNEW_ARGS(GrBatchTextStrike, (this, scaler->getKey()));
    fCache.add(strike);
    return strike;
}

void GrBatchFontCache::freeAll() {
    SkTDynamicHash<GrBatchTextStrike, GrFontDescKey>::Iter iter(&fCache);
    while (!iter.done()) {
        SkDELETE(&(*iter));
        ++iter;
    }
    fCache.rewind();
    for (int i = 0; i < kMaskFormatCount; ++i) {
        SkDELETE(fAtlases[i]);
        fAtlases[i] = NULL;
    }
}

inline GrBatchAtlas* GrBatchFontCache::getAtlas(GrMaskFormat format) const {
    int atlasIndex = MaskFormatToAtlasIndex(format);
    SkASSERT(fAtlases[atlasIndex]);
    return fAtlases[atlasIndex];
}

bool GrBatchFontCache::hasGlyph(GrGlyph* glyph) {
    SkASSERT(glyph);
    return this->getAtlas(glyph->fMaskFormat)->hasID(glyph->fID);
}

void GrBatchFontCache::addGlyphToBulkAndSetUseToken(GrBatchAtlas::BulkUseTokenUpdater* updater,
                                                    GrGlyph* glyph,
                                                    GrBatchAtlas::BatchToken token) {
    SkASSERT(glyph);
    updater->add(glyph->fID);
    this->getAtlas(glyph->fMaskFormat)->setLastUseToken(glyph->fID, token);
}

void GrBatchFontCache::setUseTokenBulk(const GrBatchAtlas::BulkUseTokenUpdater& updater,
                                       GrBatchAtlas::BatchToken token,
                                       GrMaskFormat format) {
    this->getAtlas(format)->setLastUseTokenBulk(updater, token);
}

bool GrBatchFontCache::addToAtlas(GrBatchTextStrike* strike, GrBatchAtlas::AtlasID* id,
                                  GrBatchTarget* batchTarget,
                                  GrMaskFormat format, int width, int height, const void* image,
                                  SkIPoint16* loc) {
    fPreserveStrike = strike;
    return this->getAtlas(format)->addToAtlas(id, batchTarget, width, height, image, loc);
}

uint64_t GrBatchFontCache::atlasGeneration(GrMaskFormat format) const {
    return this->getAtlas(format)->atlasGeneration();
}

GrTexture* GrBatchFontCache::getTexture(GrMaskFormat format) {
    int atlasIndex = MaskFormatToAtlasIndex(format);
    SkASSERT(fAtlases[atlasIndex]);
    return fAtlases[atlasIndex]->getTexture();
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
            SkDELETE(strike);
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
    , fAtlasedGlyphs(0) {

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

bool GrBatchTextStrike::glyphTooLargeForAtlas(GrGlyph* glyph) {
    int width = glyph->fBounds.width();
    int height = glyph->fBounds.height();
    bool useDistanceField =
            (GrGlyph::kDistance_MaskStyle == GrGlyph::UnpackMaskStyle(glyph->fPackedID));
    int pad = useDistanceField ? 2 * SK_DistanceFieldPad : 0;
    int plotWidth = (kA8_GrMaskFormat == glyph->fMaskFormat) ? GR_FONT_ATLAS_A8_PLOT_WIDTH
                                                             : GR_FONT_ATLAS_PLOT_WIDTH;
    if (width + pad > plotWidth) {
        return true;
    }
    if (height + pad > GR_FONT_ATLAS_PLOT_HEIGHT) {
        return true;
    }

    return false;
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
