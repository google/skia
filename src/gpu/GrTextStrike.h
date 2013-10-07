
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrTextStrike_DEFINED
#define GrTextStrike_DEFINED

#include "GrAllocPool.h"
#include "GrFontScaler.h"
#include "GrTHashCache.h"
#include "GrPoint.h"
#include "GrGlyph.h"
#include "GrDrawTarget.h"
#include "GrAtlas.h"

class GrFontCache;
class GrGpu;
class GrFontPurgeListener;

/**
 *  The textcache maps a hostfontscaler instance to a dictionary of
 *  glyphid->strike
 */
class GrTextStrike {
public:
    GrTextStrike(GrFontCache*, const GrKey* fontScalerKey, GrMaskFormat,
                 GrAtlasMgr*);
    ~GrTextStrike();

    const GrKey* getFontScalerKey() const { return fFontScalerKey; }
    GrFontCache* getFontCache() const { return fFontCache; }
    GrMaskFormat getMaskFormat() const { return fMaskFormat; }

    inline GrGlyph* getGlyph(GrGlyph::PackedID, GrFontScaler*);
    bool getGlyphAtlas(GrGlyph*, GrFontScaler*);

    // testing
    int countGlyphs() const { return fCache.getArray().count(); }
    const GrGlyph* glyphAt(int index) const {
        return fCache.getArray()[index];
    }

    // returns true if a plot was removed
    bool removeUnusedPlots();

public:
    // for LRU
    GrTextStrike*   fPrev;
    GrTextStrike*   fNext;

private:
    class Key;
    GrTHashTable<GrGlyph, Key, 7> fCache;
    const GrKey* fFontScalerKey;
    GrTAllocPool<GrGlyph> fPool;

    GrFontCache*    fFontCache;
    GrAtlasMgr*     fAtlasMgr;
    GrAtlas         fAtlas;

    GrMaskFormat    fMaskFormat;

    GrGlyph* generateGlyph(GrGlyph::PackedID packed, GrFontScaler* scaler);

    friend class GrFontCache;
};

class GrFontCache {
public:
    GrFontCache(GrGpu*);
    ~GrFontCache();

    inline GrTextStrike* getStrike(GrFontScaler*);

    void freeAll();

    void purgeExceptFor(GrTextStrike*);

    // remove an unused plot and its strike (if necessary)
    void freePlotExceptFor(GrTextStrike*);

    // testing
    int countStrikes() const { return fCache.getArray().count(); }
    const GrTextStrike* strikeAt(int index) const {
        return fCache.getArray()[index];
    }
    GrTextStrike* getHeadStrike() const { return fHead; }

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

#ifdef SK_DEVELOPER
    void dump() const;
#endif

private:
    friend class GrFontPurgeListener;

    class Key;
    GrTHashTable<GrTextStrike, Key, 8> fCache;
    // for LRU
    GrTextStrike* fHead;
    GrTextStrike* fTail;

    GrGpu*      fGpu;
    GrAtlasMgr* fAtlasMgr[kMaskFormatCount];

    GrTextStrike* generateStrike(GrFontScaler*, const Key&);
    inline void detachStrikeFromList(GrTextStrike*);
};

#endif
