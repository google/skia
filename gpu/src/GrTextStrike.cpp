/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#include "GrAtlas.h"
#include "GrGpu.h"
#include "GrMemory.h"
#include "GrRectanizer.h"
#include "GrTextStrike.h"
#include "GrTextStrike_impl.h"
#include "GrRect.h"

GrFontCache::GrFontCache(GrGpu* gpu) : fGpu(gpu) {
    gpu->ref();
    fAtlasMgr = NULL;

    fHead = fTail = NULL;
}

GrFontCache::~GrFontCache() {
    fCache.deleteAll();
    delete fAtlasMgr;
    fGpu->unref();
}

GrTextStrike* GrFontCache::generateStrike(GrFontScaler* scaler,
                                          const Key& key) {
    if (NULL == fAtlasMgr) {
        fAtlasMgr = new GrAtlasMgr(fGpu);
    }
    GrTextStrike* strike = new GrTextStrike(this, scaler->getKey(),
                                            scaler->getMaskFormat(), fAtlasMgr);
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
    while (strike) {
        if (strike == preserveStrike) {
            strike = strike->fPrev;
            continue;
        }
        GrTextStrike* strikeToPurge = strike;
        // keep going if we won't free up any atlases with this strike.
        strike = (NULL == strikeToPurge->fAtlas) ? strikeToPurge->fPrev : NULL;
        int index = fCache.slowFindIndex(strikeToPurge);
        GrAssert(index >= 0);
        fCache.removeAt(index, strikeToPurge->fFontScalerKey->getHash());
        this->detachStrikeFromList(strikeToPurge);
        delete strikeToPurge;
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
    GrPrintf(" GrTextStrike %p %d\n", this, gCounter);
    gCounter += 1;
#endif
}

static void FreeGlyph(GrGlyph*& glyph) { glyph->free(); }

GrTextStrike::~GrTextStrike() {
    GrAtlas::FreeLList(fAtlas);
    fFontScalerKey->unref();
    fCache.getArray().visit(FreeGlyph);

#if GR_DEBUG
    gCounter -= 1;
    GrPrintf("~GrTextStrike %p %d\n", this, gCounter);
#endif
}

GrGlyph* GrTextStrike::generateGlyph(GrGlyph::PackedID packed,
                                     GrFontScaler* scaler) {
    GrIRect bounds;
    if (!scaler->getPackedGlyphBounds(packed, &bounds)) {
        return NULL;
    }

    GrGlyph* glyph = fPool.alloc();
    glyph->init(packed, bounds);
    fCache.insert(packed, glyph);
    return glyph;
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
        return true;
    }

    GrAutoRef ar(scaler);

    int bytesPerPixel = GrMaskFormatBytesPerPixel(fMaskFormat);
    size_t size = glyph->fBounds.area() * bytesPerPixel;
    GrAutoSMalloc<1024> storage(size);
    if (!scaler->getPackedGlyphImage(glyph->fPackedID, glyph->width(),
                                     glyph->height(),
                                     glyph->width() * bytesPerPixel,
                                     storage.get())) {
        return false;
    }

    GrAtlas* atlas = fAtlasMgr->addToAtlas(fAtlas, glyph->width(),
                                           glyph->height(), storage.get(),
                                           fMaskFormat,
                                           &glyph->fAtlasLocation);
    if (NULL == atlas) {
        return false;
    }

    // update fAtlas as well, since they may be chained in a linklist
    glyph->fAtlas = fAtlas = atlas;
    return true;
}


