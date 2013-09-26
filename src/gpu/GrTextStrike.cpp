/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlas.h"
#include "GrGpu.h"
#include "GrRectanizer.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"

SK_DEFINE_INST_COUNT(GrFontScaler)
SK_DEFINE_INST_COUNT(GrKey)

///////////////////////////////////////////////////////////////////////////////

#define FONT_CACHE_STATS 0
#if FONT_CACHE_STATS
static int g_PurgeCount = 0;
#endif

GrFontCache::GrFontCache(GrGpu* gpu) : fGpu(gpu) {
    gpu->ref();
    for (int i = 0; i < kMaskFormatCount; ++i) {
        fAtlasMgr[i] = NULL;
    }

    fHead = fTail = NULL;
}

GrFontCache::~GrFontCache() {
    fCache.deleteAll();
    for (int i = 0; i < kMaskFormatCount; ++i) {
        delete fAtlasMgr[i];
    }
    fGpu->unref();
#if FONT_CACHE_STATS
      GrPrintf("Num purges: %d\n", g_PurgeCount);
#endif
}

GrTextStrike* GrFontCache::generateStrike(GrFontScaler* scaler,
                                          const Key& key) {
    GrMaskFormat format = scaler->getMaskFormat();
    if (NULL == fAtlasMgr[format]) {
        fAtlasMgr[format] = SkNEW_ARGS(GrAtlasMgr, (fGpu, format));
    }
    GrTextStrike* strike = SkNEW_ARGS(GrTextStrike,
                                      (this, scaler->getKey(),
                                       scaler->getMaskFormat(), fAtlasMgr[format]));
    fCache.insert(key, strike);

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
    fCache.deleteAll();
    for (int i = 0; i < kMaskFormatCount; ++i) {
        delete fAtlasMgr[i];
        fAtlasMgr[i] = NULL;
    }
    fHead = NULL;
    fTail = NULL;
}

void GrFontCache::purgeExceptFor(GrTextStrike* preserveStrike) {
    SkASSERT(NULL != preserveStrike);
    GrTextStrike* strike = fTail;
    bool purge = true;
    GrMaskFormat maskFormat = preserveStrike->fMaskFormat;
    while (strike) {
        if (strike == preserveStrike || maskFormat != strike->fMaskFormat) {
            strike = strike->fPrev;
            continue;
        }
        GrTextStrike* strikeToPurge = strike;
        strike = strikeToPurge->fPrev;
        if (purge) {
            // keep purging if we won't free up any atlases with this strike.
            purge = (NULL == strikeToPurge->fAtlas);
            int index = fCache.slowFindIndex(strikeToPurge);
            SkASSERT(index >= 0);
            fCache.removeAt(index, strikeToPurge->fFontScalerKey->getHash());
            this->detachStrikeFromList(strikeToPurge);
            delete strikeToPurge;
        }
    }
#if FONT_CACHE_STATS
    ++g_PurgeCount;
#endif
}

void GrFontCache::freeAtlasExceptFor(GrTextStrike* preserveStrike) {
    SkASSERT(NULL != preserveStrike);
    GrTextStrike* strike = fTail;
    GrMaskFormat maskFormat = preserveStrike->fMaskFormat;
    while (strike) {
        if (strike == preserveStrike || maskFormat != strike->fMaskFormat) {
            strike = strike->fPrev;
            continue;
        }
        GrTextStrike* strikeToPurge = strike;
        strike = strikeToPurge->fPrev;
        if (strikeToPurge->removeUnusedAtlases()) {
            if (NULL == strikeToPurge->fAtlas) {
                int index = fCache.slowFindIndex(strikeToPurge);
                SkASSERT(index >= 0);
                fCache.removeAt(index, strikeToPurge->fFontScalerKey->getHash());
                this->detachStrikeFromList(strikeToPurge);
                delete strikeToPurge;
            }
            break;
        }
    }
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

GrTextStrike::GrTextStrike(GrFontCache* cache, const GrKey* key,
                           GrMaskFormat format,
                           GrAtlasMgr* atlasMgr) : fPool(64) {
    fFontScalerKey = key;
    fFontScalerKey->ref();

    fFontCache = cache;     // no need to ref, it won't go away before we do
    fAtlasMgr = atlasMgr;   // no need to ref, it won't go away before we do
    fAtlas = NULL;

    fMaskFormat = format;

#ifdef SK_DEBUG
//    GrPrintf(" GrTextStrike %p %d\n", this, gCounter);
    gCounter += 1;
#endif
}

// these signatures are needed because they're used with
// SkTDArray::visitAll() (see destructor & removeUnusedAtlases())
static void free_glyph(GrGlyph*& glyph) { glyph->free(); }

static void invalidate_glyph(GrGlyph*& glyph) {
    if (glyph->fAtlas && glyph->fAtlas->drawToken().isIssued()) {
        glyph->fAtlas = NULL;
    }
}

GrTextStrike::~GrTextStrike() {
    GrAtlas::FreeLList(fAtlas);
    fFontScalerKey->unref();
    fCache.getArray().visitAll(free_glyph);

#ifdef SK_DEBUG
    gCounter -= 1;
//    GrPrintf("~GrTextStrike %p %d\n", this, gCounter);
#endif
}

GrGlyph* GrTextStrike::generateGlyph(GrGlyph::PackedID packed,
                                     GrFontScaler* scaler) {
    SkIRect bounds;
    if (!scaler->getPackedGlyphBounds(packed, &bounds)) {
        return NULL;
    }

    GrGlyph* glyph = fPool.alloc();
    glyph->init(packed, bounds);
    fCache.insert(packed, glyph);
    return glyph;
}

bool GrTextStrike::removeUnusedAtlases() {
    fCache.getArray().visitAll(invalidate_glyph);
    return GrAtlas::RemoveUnusedAtlases(fAtlasMgr, &fAtlas);
}

bool GrTextStrike::getGlyphAtlas(GrGlyph* glyph, GrFontScaler* scaler,
                                 GrDrawTarget::DrawToken currentDrawToken) {
#if 0   // testing hack to force us to flush our cache often
    static int gCounter;
    if ((++gCounter % 10) == 0) return false;
#endif

    SkASSERT(glyph);
    SkASSERT(scaler);
    SkASSERT(fCache.contains(glyph));
    if (glyph->fAtlas) {
        glyph->fAtlas->setDrawToken(currentDrawToken);
        return true;
    }

    SkAutoRef ar(scaler);

    int bytesPerPixel = GrMaskFormatBytesPerPixel(fMaskFormat);
    size_t size = glyph->fBounds.area() * bytesPerPixel;
    SkAutoSMalloc<1024> storage(size);
    if (!scaler->getPackedGlyphImage(glyph->fPackedID, glyph->width(),
                                     glyph->height(),
                                     glyph->width() * bytesPerPixel,
                                     storage.get())) {
        return false;
    }

    GrAtlas* atlas = fAtlasMgr->addToAtlas(&fAtlas, glyph->width(),
                                           glyph->height(), storage.get(),
                                           &glyph->fAtlasLocation);
    if (NULL == atlas) {
        return false;
    }

    glyph->fAtlas = atlas;
    atlas->setDrawToken(currentDrawToken);
    return true;
}
