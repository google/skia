
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
#include "GrTHashTable.h"
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
    GrTextStrike(GrFontCache*, const GrKey* fontScalerKey, GrMaskFormat, GrAtlasMgr*);
    ~GrTextStrike();

    const GrKey* getFontScalerKey() const { return fFontScalerKey; }
    GrFontCache* getFontCache() const { return fFontCache; }
    GrMaskFormat getMaskFormat() const { return fMaskFormat; }

    inline GrGlyph* getGlyph(GrGlyph::PackedID, GrFontScaler*);
    bool addGlyphToAtlas(GrGlyph*, GrFontScaler*);

    SkISize getAtlasSize() const { return fAtlas.getSize(); }

    // testing
    int countGlyphs() const { return fCache.getArray().count(); }
    const GrGlyph* glyphAt(int index) const {
        return fCache.getArray()[index];
    }

    // remove any references to this plot
    void removePlot(const GrPlot* plot);

public:
    // for easy removal from list
    GrTextStrike*   fPrev;
    GrTextStrike*   fNext;

private:
    class Key;
    GrTHashTable<GrGlyph, Key, 7> fCache;
    const GrKey* fFontScalerKey;
    GrTAllocPool<GrGlyph> fPool;

    GrFontCache*    fFontCache;
    GrAtlasMgr*     fAtlasMgr;
    GrMaskFormat    fMaskFormat;
    bool            fUseDistanceField;

    GrAtlas         fAtlas;

    GrGlyph* generateGlyph(GrGlyph::PackedID packed, GrFontScaler* scaler);

    friend class GrFontCache;
};

class GrFontCache {
public:
    GrFontCache(GrGpu*);
    ~GrFontCache();

    inline GrTextStrike* getStrike(GrFontScaler*, bool useDistanceField);

    void freeAll();

    // make an unused plot available
    bool freeUnusedPlot(GrTextStrike* preserveStrike);

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

    enum AtlasType {
        kA8_AtlasType,   //!< 1-byte per pixel
        k565_AtlasType,  //!< 2-bytes per pixel
        k8888_AtlasType, //!< 4-bytes per pixel

        kLast_AtlasType = k8888_AtlasType
    };
    static const int kAtlasCount = kLast_AtlasType + 1;

private:
    friend class GrFontPurgeListener;

    class Key;
    GrTHashTable<GrTextStrike, Key, 8> fCache;
    // for LRU
    GrTextStrike* fHead;
    GrTextStrike* fTail;

    GrGpu*      fGpu;
    GrAtlasMgr* fAtlasMgr[kAtlasCount];

    GrTextStrike* generateStrike(GrFontScaler*, const Key&);
    inline void detachStrikeFromList(GrTextStrike*);
    void purgeStrike(GrTextStrike* strike);
};

#endif
