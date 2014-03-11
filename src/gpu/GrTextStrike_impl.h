
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrTextStrike_impl_DEFINED
#define GrTextStrike_impl_DEFINED

class GrFontCache::Key {
public:
    explicit Key(const GrKey* fontScalarKey) {
        fFontScalerKey = fontScalarKey;
    }

    intptr_t getHash() const { return fFontScalerKey->getHash(); }

    static bool LessThan(const GrTextStrike& strike, const Key& key) {
        return *strike.getFontScalerKey() < *key.fFontScalerKey;
    }
    static bool Equals(const GrTextStrike& strike, const Key& key) {
        return *strike.getFontScalerKey() == *key.fFontScalerKey;
    }

private:
    const GrKey* fFontScalerKey;
};

void GrFontCache::detachStrikeFromList(GrTextStrike* strike) {
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

GrTextStrike* GrFontCache::getStrike(GrFontScaler* scaler, bool useDistanceField) {
    this->validate();

    const Key key(scaler->getKey());
    GrTextStrike* strike = fCache.find(key);
    if (NULL == strike) {
        strike = this->generateStrike(scaler, key);
    } else if (strike->fPrev) {
        // Need to put the strike at the head of its dllist, since that is how
        // we age the strikes for purging (we purge from the back of the list
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

///////////////////////////////////////////////////////////////////////////////

/**
 *  This Key just wraps a glyphID, and matches the protocol need for
 *  GrTHashTable
 */
class GrTextStrike::Key {
public:
    Key(GrGlyph::PackedID id) : fPackedID(id) {}

    uint32_t getHash() const { return fPackedID; }

    static bool LessThan(const GrGlyph& glyph, const Key& key) {
        return glyph.fPackedID < key.fPackedID;
    }
    static bool Equals(const GrGlyph& glyph, const Key& key) {
        return glyph.fPackedID == key.fPackedID;
    }

private:
    GrGlyph::PackedID fPackedID;
};

GrGlyph* GrTextStrike::getGlyph(GrGlyph::PackedID packed,
                                GrFontScaler* scaler) {
    GrGlyph* glyph = fCache.find(packed);
    if (NULL == glyph) {
        glyph = this->generateGlyph(packed, scaler);
    }
    return glyph;
}

#endif
