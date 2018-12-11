/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextToPathIter_DEFINED
#define SkTextToPathIter_DEFINED

#include "SkFontPriv.h"
#include "SkPaint.h"
#include "SkStrikeCache.h"

class SkTextBaseIter {
public:
    const SkFont&   getFont() const { return fFont; }
    const SkPaint&  getPaint() const { return fPaint; }
    SkScalar        getPathScale() const { return fScale; }

protected:
    SkTextBaseIter(const char text[], size_t length, SkTextEncoding,
                   const SkFont&, const SkPaint& paint, bool applyStrokeAndPathEffects);

    SkExclusiveStrikePtr fCache;
    SkFont               fFont;
    SkPaint              fPaint;
    SkScalar             fScale;
    SkScalar             fPrevAdvance;
    const char*          fText;
    const char*          fStop;
    SkFontPriv::GlyphCacheProc fGlyphCacheProc;

    SkScalar        fXPos;      // accumulated xpos, returned in next
};

class SkTextToPathIter : SkTextBaseIter {
public:
    SkTextToPathIter(const char text[], size_t length, SkTextEncoding encoding,
                     const SkFont& font, const SkPaint& paint, bool applyStrokeAndPathEffects)
         : SkTextBaseIter(text, length, encoding, font, paint, applyStrokeAndPathEffects)
    {}

    /**
     *  Returns false when all of the text has been consumed
     */
    bool next(const SkPath** path, SkScalar* xpos);
};

class SkTextInterceptsIter : SkTextBaseIter {
public:
    enum class TextType {
        kText,
        kPosText
    };

    SkTextInterceptsIter(const char text[], size_t length, SkTextEncoding encoding, const SkFont& font,
                         const SkPaint& paint, const SkScalar bounds[2], SkScalar x, SkScalar y,
                         TextType textType)
         : SkTextBaseIter(text, length, encoding, font, paint, false)
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
