/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextToPathIter_DEFINED
#define SkTextToPathIter_DEFINED

#include "include/core/SkPaint.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkStrikeCache.h"

class SkTextBaseIter {
public:
    const SkFont&   getFont() const { return fFont; }
    const SkPaint&  getPaint() const { return fPaint; }
    SkScalar        getPathScale() const { return fScale; }

protected:
    SkTextBaseIter(const SkGlyphID glyphs[], int count, const SkFont&, const SkPaint*);

    SkExclusiveStrikePtr fCache;
    SkFont               fFont;
    SkPaint              fPaint;
    SkScalar             fScale;
    SkScalar             fPrevAdvance;
    const SkGlyphID*     fGlyphs;
    const SkGlyphID*     fStop;

    SkScalar        fXPos;      // accumulated xpos, returned in next
};

class SkTextInterceptsIter : SkTextBaseIter {
public:
    enum class TextType {
        kText,
        kPosText
    };

    SkTextInterceptsIter(const SkGlyphID glyphs[], int count, const SkFont& font,
                         const SkPaint* paint, const SkScalar bounds[2], SkScalar x, SkScalar y,
                         TextType textType)
         : SkTextBaseIter(glyphs, count, font, paint)
    {
        fBoundsBase[0] = bounds[0];
        fBoundsBase[1] = bounds[1];
        this->setPosition(x, y);
    }

    /**
     *  Returns false when all of the text has been consumed
     */
    bool next(SkScalar* array, int* count);

    void setPosition(SkScalar x, SkScalar y) {
        SkScalar xOffset = 0;
        for (int i = 0; i < (int) SK_ARRAY_COUNT(fBounds); ++i) {
            SkScalar bound = fBoundsBase[i] - y;
            fBounds[i] = bound / fScale;
        }

        fXPos = xOffset + x;
        fPrevAdvance = 0;
    }

private:
    SkScalar fBounds[2];
    SkScalar fBoundsBase[2];
};

#endif
