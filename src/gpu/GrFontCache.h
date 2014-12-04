
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrTextStrike_DEFINED
#define GrTextStrike_DEFINED

#include "GrAtlas.h"
#include "GrDrawTarget.h"
#include "GrFontScaler.h"
#include "GrGlyph.h"
#include "SkTDynamicHash.h"
#include "SkVarAlloc.h"

class GrFontCache;
class GrGpu;
class GrFontPurgeListener;

/**
 *  The textstrike maps a hostfontscaler instance to a dictionary of
 *  glyphid->strike
 */
class GrTextStrike {
public:
    GrTextStrike(GrFontCache*, const GrFontDescKey* fontScalerKey);
    ~GrTextStrike();

    const GrFontDescKey* getFontScalerKey() const { return fFontScalerKey; }
    GrFontCache* getFontCache() const { return fFontCache; }

    inline GrGlyph* getGlyph(GrGlyph::PackedID packed, GrFontScaler* scaler) {
        GrGlyph* glyph = fCache.find(packed);
        if (NULL == glyph) {
            glyph = this->generateGlyph(packed, scaler);
        }
        return glyph;
    }

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
    SkVarAlloc fPool;

    GrFontCache*    fFontCache;
    bool            fUseDistanceField;

    GrAtlas::ClientPlotUsage fPlotUsage;

    GrGlyph* generateGlyph(GrGlyph::PackedID packed, GrFontScaler* scaler);

    friend class GrFontCache;
};

class GrFontCache {
public:
    GrFontCache(GrGpu*);
    ~GrFontCache();

    inline GrTextStrike* getStrike(GrFontScaler* scaler, bool useDistanceField) {
        this->validate();
        
        GrTextStrike* strike = fCache.find(*(scaler->getKey()));
        if (NULL == strike) {
            strike = this->generateStrike(scaler);
        } else if (strike->fPrev) {
            // Need to put the strike at the head of its dllist, since that is how
            // we age the strikes for purging (we purge from the back of the list)
            this->detachStrikeFromList(strike);
            // attach at the head
            fHead->fPrev = strike;
            strike->fNext = fHead;
            strike->fPrev = NULL;
            fHead = strike;
        }
        strike->fUseDistanceField = useDistanceField;
        this->validate();
        return strike;
    }

    // add to texture atlas that matches this format
    GrPlot* addToAtlas(GrMaskFormat format, GrAtlas::ClientPlotUsage* usage,
                       int width, int height, const void* image,
                       SkIPoint16* loc);

    void freeAll();

    // make an unused plot available for this glyph
    bool freeUnusedPlot(GrTextStrike* preserveStrike, const GrGlyph* glyph);

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
    
    inline void detachStrikeFromList(GrTextStrike* strike)  {
        if (strike->fPrev) {
            SkASSERT(fHead != strike);
            strike->fPrev->fNext = strike->fNext;
        } else {
            SkASSERT(fHead == strike);
            fHead = strike->fNext;
        }
        
        if (strike->fNext) {
            SkASSERT(fTail != strike);
            strike->fNext->fPrev = strike->fPrev;
        } else {
            SkASSERT(fTail == strike);
            fTail = strike->fPrev;
        }
    }
    
    void purgeStrike(GrTextStrike* strike);
};

#endif
