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
    fAtlasMgr = NULL;

    fHead = fTail = NULL;
}

GrFontCache::~GrFontCache() {
    fCache.deleteAll();
    delete fAtlasMgr;
    fGpu->unref();
#if FONT_CACHE_STATS
      GrPrintf("Num purges: %d\n", g_PurgeCount);
#endif
}

GrTextStrike* GrFontCache::generateStrike(GrFontScaler* scaler,
                                          const Key& key) {
    if (NULL == fAtlasMgr) {
        fAtlasMgr = SkNEW_ARGS(GrAtlasMgr, (fGpu));
    }
    GrTextStrike* strike = SkNEW_ARGS(GrTextStrike,
                                      (this, scaler->getKey(),
                                       scaler->getMaskFormat(), fAtlasMgr));
    fCache.insert(key, strike);

    if (fHead) {
        fHead->fPrev = strike;
    } else {
        GrAssert(NULL == fTail);
        fTail = strike;
    }
    strike->fPrev = NULL;
    strike->fNext = fHead;
    fHead = strike;

    return strike;
}

void GrFontCache::freeAll() {
    fCache.deleteAll();
    delete fAtlasMgr;
    fAtlasMgr = NULL;
    fHead = NULL;
    fTail = NULL;
}

void GrFontCache::purgeExceptFor(GrTextStrike* preserveStrike) {
    GrTextStrike* strike = fTail;
    bool purge = true;
    while (strike) {
        if (strike == preserveStrike) {
            strike = strike->fPrev;
            continue;
        }
        GrTextStrike* strikeToPurge = strike;
        strike = strikeToPurge->fPrev;
        if (purge) {
            // keep purging if we won't free up any atlases with this strike.
            purge = (NULL == strikeToPurge->fAtlas);
            int index = fCache.slowFindIndex(strikeToPurge);
            GrAssert(index >= 0);
            fCache.removeAt(index, strikeToPurge->fFontScalerKey->getHash());
            this->detachStrikeFromList(strikeToPurge);
            delete strikeToPurge;
        } else {
            // for the remaining strikes, we just mark them unused
            GrAtlas::MarkAllUnused(strikeToPurge->fAtlas);
        }
    }
#if FONT_CACHE_STATS
    ++g_PurgeCount;
#endif
}

void GrFontCache::freeAtlasExceptFor(GrTextStrike* preserveStrike) {
    GrTextStrike* strike = fTail;
    while (strike) {
        if (strike == preserveStrike) {
            strike = strike->fPrev;
            continue;
        }
        GrTextStrike* strikeToPurge = strike;
        strike = strikeToPurge->fPrev;
        if (strikeToPurge->removeUnusedAtlases()) {
            if (NULL == strikeToPurge->fAtlas) {
                int index = fCache.slowFindIndex(strikeToPurge);
                GrAssert(index >= 0);
                fCache.removeAt(index, strikeToPurge->fFontScalerKey->getHash());
                this->detachStrikeFromList(strikeToPurge);
                delete strikeToPurge;
            }
            break;
        }
    }
}

#if GR_DEBUG
void GrFontCache::validate() const {
    int count = fCache.count();
    if (0 == count) {
        GrAssert(!fHead);
        GrAssert(!fTail);
    } else if (1 == count) {
        GrAssert(fHead == fTail);
    } else {
        GrAssert(fHead != fTail);
    }

    int count2 = 0;
    const GrTextStrike* strike = fHead;
    while (strike) {
        count2 += 1;
        strike = strike->fNext;
    }
    GrAssert(count == count2);

    count2 = 0;
    strike = fTail;
    while (strike) {
        count2 += 1;
        strike = strike->fPrev;
    }
    GrAssert(count == count2);
}
#endif

///////////////////////////////////////////////////////////////////////////////

#if GR_DEBUG
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

#if GR_DEBUG
//    GrPrintf(" GrTextStrike %p %d\n", this, gCounter);
    gCounter += 1;
#endif
}

// these signatures are needed because they're used with
// SkTDArray::visitAll() (see destructor & removeUnusedAtlases())
static void free_glyph(GrGlyph*& glyph) { glyph->free(); }

static void invalidate_glyph(GrGlyph*& glyph) {
    if (glyph->fAtlas && !glyph->fAtlas->used()) {
        glyph->fAtlas = NULL;
    }
}

GrTextStrike::~GrTextStrike() {
    GrAtlas::FreeLList(fAtlas);
    fFontScalerKey->unref();
    fCache.getArray().visitAll(free_glyph);

#if GR_DEBUG
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

    return false;
}

bool GrTextStrike::getGlyphAtlas(GrGlyph* glyph, GrFontScaler* scaler) {
#if 0   // testing hack to force us to flush our cache often
    static int gCounter;
    if ((++gCounter % 10) == 0) return false;
#endif

    GrAssert(glyph);
    GrAssert(scaler);
    GrAssert(fCache.contains(glyph));
    if (glyph->fAtlas) {
        glyph->fAtlas->setUsed(true);
        return true;
    }

    GrAutoRef ar(scaler);

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
                                           fMaskFormat,
                                           &glyph->fAtlasLocation);
    if (NULL == atlas) {
        return false;
    }

    glyph->fAtlas = atlas;
    atlas->setUsed(true);
    return true;
}
