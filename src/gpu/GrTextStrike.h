
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
#include "SkTDynamicHash.h"
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
    GrTextStrike(GrFontCache*, const GrFontDescKey* fontScalerKey, GrMaskFormat, GrAtlas*);
    ~GrTextStrike();

    const GrFontDescKey* getFontScalerKey() const { return fFontScalerKey; }
    GrFontCache* getFontCache() const { return fFontCache; }
    GrMaskFormat getMaskFormat() const { return fMaskFormat; }
    GrTexture*   getTexture() const { return fAtlas->getTexture(); }

    inline GrGlyph* getGlyph(GrGlyph::PackedID, GrFontScaler*);
    // returns true if glyph (or glyph+padding for distance field)
    // is too large to ever fit in texture atlas subregions (GrPlots)
    bool glyphTooLargeForAtlas(GrGlyph*);
    // returns true if glyph successfully added to texture atlas, false otherwise
    bool addGlyphToAtlas(GrGlyph*, GrFontScaler*);

    // testing
    int countGlyphs() const { return fCache.count(); }

    // remove any references to this plot
    void removePlot(const GrPlot* plot);

    static const GrFontDescKey& GetKey(const GrTextStrike& ts) {
        return *(ts.fFontScalerKey);
    }
    static uint32_t Hash(const GrFontDescKey& key) {
        return key.getHash();
    }

public:
    // for easy removal from list
    GrTextStrike*   fPrev;
    GrTextStrike*   fNext;

private:
    SkTDynamicHash<GrGlyph, GrGlyph::PackedID> fCache;
    const GrFontDescKey* fFontScalerKey;
    GrTAllocPool<GrGlyph> fPool;

    GrFontCache*    fFontCache;
    GrAtlas*        fAtlas;
    GrMaskFormat    fMaskFormat;
    bool            fUseDistanceField;

    GrAtlas::ClientPlotUsage fPlotUsage;

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
    int countStrikes() const { return fCache.count(); }
    GrTextStrike* getHeadStrike() const { return fHead; }

    void updateTextures() {
        for (int i = 0; i < kAtlasCount; ++i) {
            if (fAtlases[i]) {
                fAtlases[i]->uploadPlotsToTexture();
            }
        }
    }

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

    void dump() const;

    enum AtlasType {
        kA8_AtlasType,   //!< 1-byte per pixel
        k565_AtlasType,  //!< 2-bytes per pixel
        k8888_AtlasType, //!< 4-bytes per pixel

        kLast_AtlasType = k8888_AtlasType
    };
    static const int kAtlasCount = kLast_AtlasType + 1;

private:
    friend class GrFontPurgeListener;

    SkTDynamicHash<GrTextStrike, GrFontDescKey> fCache;
    // for LRU
    GrTextStrike* fHead;
    GrTextStrike* fTail;

    GrGpu*      fGpu;
    GrAtlas*    fAtlases[kAtlasCount];

    GrTextStrike* generateStrike(GrFontScaler*);
    inline void detachStrikeFromList(GrTextStrike*);
    void purgeStrike(GrTextStrike* strike);
};

#endif
