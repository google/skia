/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCharToGlyphCache_DEFINED
#define SkCharToGlyphCache_DEFINED

#include "SkTypes.h"
#include "../private/SkTDArray.h"

class SkCharToGlyphCache {
public:
    SkCharToGlyphCache();
    ~SkCharToGlyphCache();

    /**
     *  Given a unichar, return its glyphID (if the return value is positive), else return
     *  ~index of where to insert the computed glyphID.
     *
     *  int result = cache.charToGlyph(unichar);
     *  if (result >= 0) {
     *      glyphID = result;
     *  } else {
     *      glyphID = compute_glyph_using_typeface(unichar);
     *      cache.insertCharAndGlyph(~result, unichar, glyphID);
     *  }
     */
    int findGlyphIndex(SkUnichar c) const;

    /**
     *  Insert a new char/glyph pair into the cache at the specified index.
     *  See charToGlyph() for how to compute the bit-not of the index.
     */
    void insertCharAndGlyph(int index, SkUnichar, SkGlyphID);

    // helper to pre-seed an entry in the cache
    void addCharAndGlyph(SkUnichar unichar, SkGlyphID glyph) {
        int index = this->findGlyphIndex(unichar);
        if (index >= 0) {
            SkASSERT(SkToU16(index) == glyph);
        } else {
            this->insertCharAndGlyph(~index, unichar, glyph);
        }
    }

private:
    SkTDArray<int32_t>   fK32;
    SkTDArray<uint16_t>  fV16;
    double               fDenom;
};

#endif
