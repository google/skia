/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpu.h"
#include "GrRectanizer.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"
#include "SkString.h"

#include "SkDistanceFieldGen.h"

///////////////////////////////////////////////////////////////////////////////

#define GR_ATLAS_TEXTURE_WIDTH 1024
#define GR_ATLAS_TEXTURE_HEIGHT 2048

#define GR_PLOT_WIDTH  256
#define GR_PLOT_HEIGHT 256

#define GR_NUM_PLOTS_X   (GR_ATLAS_TEXTURE_WIDTH / GR_PLOT_WIDTH)
#define GR_NUM_PLOTS_Y   (GR_ATLAS_TEXTURE_HEIGHT / GR_PLOT_HEIGHT)

#define FONT_CACHE_STATS 0
#if FONT_CACHE_STATS
static int g_PurgeCount = 0;
#endif

GrFontCache::GrFontCache(GrGpu* gpu) : fGpu(gpu) {
    gpu->ref();
    for (int i = 0; i < kAtlasCount; ++i) {
        fAtlases[i] = NULL;
    }

    fHead = fTail = NULL;
}

GrFontCache::~GrFontCache() {
    SkTDynamicHash<GrTextStrike, GrFontDescKey>::Iter iter(&fCache);
    while (!iter.done()) {
        SkDELETE(&(*iter));
        ++iter;
    }
    for (int i = 0; i < kAtlasCount; ++i) {
        delete fAtlases[i];
    }
    fGpu->unref();
#if FONT_CACHE_STATS
      GrPrintf("Num purges: %d\n", g_PurgeCount);
#endif
}

static GrPixelConfig mask_format_to_pixel_config(GrMaskFormat format) {
    static const GrPixelConfig sPixelConfigs[] = {
        kAlpha_8_GrPixelConfig,
        kRGB_565_GrPixelConfig,
        kSkia8888_GrPixelConfig,
        kSkia8888_GrPixelConfig
    };
    SK_COMPILE_ASSERT(SK_ARRAY_COUNT(sPixelConfigs) == kMaskFormatCount, array_size_mismatch);

    return sPixelConfigs[format];
}

static int mask_format_to_atlas_index(GrMaskFormat format) {
    static const int sAtlasIndices[] = {
        GrFontCache::kA8_AtlasType,
        GrFontCache::k565_AtlasType,
        GrFontCache::k8888_AtlasType,
        GrFontCache::k8888_AtlasType
    };
    SK_COMPILE_ASSERT(SK_ARRAY_COUNT(sAtlasIndices) == kMaskFormatCount, array_size_mismatch);

    SkASSERT(sAtlasIndices[format] < GrFontCache::kAtlasCount);
    return sAtlasIndices[format];
}

GrTextStrike* GrFontCache::generateStrike(GrFontScaler* scaler) {
    GrMaskFormat format = scaler->getMaskFormat();
    GrPixelConfig config = mask_format_to_pixel_config(format);
    int atlasIndex = mask_format_to_atlas_index(format);
    if (NULL == fAtlases[atlasIndex]) {
        SkISize textureSize = SkISize::Make(GR_ATLAS_TEXTURE_WIDTH,
                                            GR_ATLAS_TEXTURE_HEIGHT);
        fAtlases[atlasIndex] = SkNEW_ARGS(GrAtlas, (fGpu, config, kNone_GrTextureFlags,
                                                    textureSize,
                                                    GR_NUM_PLOTS_X,
                                                    GR_NUM_PLOTS_Y,
                                                    true));
    }
    GrTextStrike* strike = SkNEW_ARGS(GrTextStrike,
                                      (this, scaler->getKey(), format, fAtlases[atlasIndex]));
    fCache.add(strike);

    if (fHead) {
        fHead->fPrev = strike;
    } else {
        SkASSERT(NULL == fTail);
        fTail = strike;
    }
    strike->fPrev = NULL;
    strike->fNext = fHead;
    fHead = strike;

    return strike;
}

void GrFontCache::freeAll() {
    SkTDynamicHash<GrTextStrike, GrFontDescKey>::Iter iter(&fCache);
    while (!iter.done()) {
        SkDELETE(&(*iter));
        ++iter;
    }
    fCache.rewind();
    for (int i = 0; i < kAtlasCount; ++i) {
        delete fAtlases[i];
        fAtlases[i] = NULL;
    }
    fHead = NULL;
    fTail = NULL;
}

void GrFontCache::purgeStrike(GrTextStrike* strike) {
    fCache.remove(*(strike->fFontScalerKey));
    this->detachStrikeFromList(strike);
    delete strike;
}

bool GrFontCache::freeUnusedPlot(GrTextStrike* preserveStrike) {
    SkASSERT(NULL != preserveStrike);

    GrAtlas* atlas = preserveStrike->fAtlas;
    GrPlot* plot = atlas->getUnusedPlot();
    if (NULL == plot) {
        return false;
    }
    plot->resetRects();

    GrTextStrike* strike = fHead;
    GrMaskFormat maskFormat = preserveStrike->fMaskFormat;
    while (strike) {
        if (maskFormat != strike->fMaskFormat) {
            strike = strike->fNext;
            continue;
        }

        GrTextStrike* strikeToPurge = strike;
        strike = strikeToPurge->fNext;
        strikeToPurge->removePlot(plot);

        // clear out any empty strikes (except this one)
        if (strikeToPurge != preserveStrike && strikeToPurge->fPlotUsage.isEmpty()) {
            this->purgeStrike(strikeToPurge);
        }
    }

#if FONT_CACHE_STATS
    ++g_PurgeCount;
#endif

    return true;
}

#ifdef SK_DEBUG
void GrFontCache::validate() const {
    int count = fCache.count();
    if (0 == count) {
        SkASSERT(!fHead);
        SkASSERT(!fTail);
    } else if (1 == count) {
        SkASSERT(fHead == fTail);
    } else {
        SkASSERT(fHead != fTail);
    }

    int count2 = 0;
    const GrTextStrike* strike = fHead;
    while (strike) {
        count2 += 1;
        strike = strike->fNext;
    }
    SkASSERT(count == count2);

    count2 = 0;
    strike = fTail;
    while (strike) {
        count2 += 1;
        strike = strike->fPrev;
    }
    SkASSERT(count == count2);
}
#endif

void GrFontCache::dump() const {
    static int gDumpCount = 0;
    for (int i = 0; i < kAtlasCount; ++i) {
        if (NULL != fAtlases[i]) {
            GrTexture* texture = fAtlases[i]->getTexture();
            if (NULL != texture) {
                SkString filename;
#ifdef SK_BUILD_FOR_ANDROID
                filename.printf("/sdcard/fontcache_%d%d.png", gDumpCount, i);
#else
                filename.printf("fontcache_%d%d.png", gDumpCount, i);
#endif
                texture->savePixels(filename.c_str());
            }
        }
    }
    ++gDumpCount;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG
    static int gCounter;
#endif

/*
    The text strike is specific to a given font/style/matrix setup, which is
    represented by the GrHostFontScaler object we are given in getGlyph().

    We map a 32bit glyphID to a GrGlyph record, which in turn points to a
    atlas and a position within that texture.
 */

GrTextStrike::GrTextStrike(GrFontCache* cache, const GrFontDescKey* key,
                           GrMaskFormat format,
                           GrAtlas* atlas) : fPool(64) {
    fFontScalerKey = key;
    fFontScalerKey->ref();

    fFontCache = cache;     // no need to ref, it won't go away before we do
    fAtlas = atlas;         // no need to ref, it won't go away before we do

    fMaskFormat = format;

#ifdef SK_DEBUG
//    GrPrintf(" GrTextStrike %p %d\n", this, gCounter);
    gCounter += 1;
#endif
}

GrTextStrike::~GrTextStrike() {
    fFontScalerKey->unref();
    SkTDynamicHash<GrGlyph, GrGlyph::PackedID>::Iter iter(&fCache);
    while (!iter.done()) {
        (*iter).free();
        ++iter;
    }

#ifdef SK_DEBUG
    gCounter -= 1;
//    GrPrintf("~GrTextStrike %p %d\n", this, gCounter);
#endif
}

GrGlyph* GrTextStrike::generateGlyph(GrGlyph::PackedID packed,
                                     GrFontScaler* scaler) {
    SkIRect bounds;
    if (fUseDistanceField) {
        if (!scaler->getPackedGlyphDFBounds(packed, &bounds)) {
            return NULL;
        }
    } else {
        if (!scaler->getPackedGlyphBounds(packed, &bounds)) {
            return NULL;
        }
    }

    GrGlyph* glyph = fPool.alloc();
    glyph->init(packed, bounds);
    fCache.add(glyph);
    return glyph;
}

void GrTextStrike::removePlot(const GrPlot* plot) {
    SkTDynamicHash<GrGlyph, GrGlyph::PackedID>::Iter iter(&fCache);
    while (!iter.done()) {
        if (plot == (*iter).fPlot) {
            (*iter).fPlot = NULL;
        }
        ++iter;
    }

    GrAtlas::RemovePlot(&fPlotUsage, plot);
}


bool GrTextStrike::addGlyphToAtlas(GrGlyph* glyph, GrFontScaler* scaler) {
#if 0   // testing hack to force us to flush our cache often
    static int gCounter;
    if ((++gCounter % 10) == 0) return false;
#endif

    SkASSERT(glyph);
    SkASSERT(scaler);
    SkASSERT(fCache.find(glyph->fPackedID));
    SkASSERT(NULL == glyph->fPlot);

    SkAutoUnref ar(SkSafeRef(scaler));

    int bytesPerPixel = GrMaskFormatBytesPerPixel(fMaskFormat);

    size_t size = glyph->fBounds.area() * bytesPerPixel;
    SkAutoSMalloc<1024> storage(size);
    if (fUseDistanceField) {
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

    GrPlot* plot  = fAtlas->addToAtlas(&fPlotUsage, glyph->width(),
                                       glyph->height(), storage.get(),
                                       &glyph->fAtlasLocation);

    if (NULL == plot) {
        return false;
    }

    glyph->fPlot = plot;
    return true;
}
