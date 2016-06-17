/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTextToPathIter_DEFINED
#define SkTextToPathIter_DEFINED

#include "SkAutoKern.h"
#include "SkPaint.h"

class SkGlyphCache;

class SkTextBaseIter {
protected:
    SkTextBaseIter(const char text[], size_t length, const SkPaint& paint,
                   bool applyStrokeAndPathEffects);
    ~SkTextBaseIter();

    SkGlyphCache*   fCache;
    SkPaint         fPaint;
    SkScalar        fScale;
    SkScalar        fPrevAdvance;
    const char*     fText;
    const char*     fStop;
    SkPaint::GlyphCacheProc fGlyphCacheProc;

    SkScalar        fXPos;      // accumulated xpos, returned in next
    SkAutoKern      fAutoKern;
    int             fXYIndex;   // cache for horizontal -vs- vertical text
};

class SkTextToPathIter : SkTextBaseIter {
public:
    SkTextToPathIter(const char text[], size_t length, const SkPaint& paint,
                     bool applyStrokeAndPathEffects)
                     : SkTextBaseIter(text, length, paint, applyStrokeAndPathEffects) {
    }

    const SkPaint&  getPaint() const { return fPaint; }
    SkScalar        getPathScale() const { return fScale; }

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

    SkTextInterceptsIter(const char text[], size_t length, const SkPaint& paint,
                         const SkScalar bounds[2], SkScalar x, SkScalar y, TextType textType)
                         : SkTextBaseIter(text, length, paint, false)
                         , fTextType(textType) {
        fBoundsBase[0] = bounds[0];
        fBoundsBase[1] = bounds[1];
        this->setPosition(x, y);
    }

    /**
     *  Returns false when all of the text has been consumed
     */
    bool next(SkScalar* array, int* count);

    void setPosition(SkScalar x, SkScalar y) {
        SkScalar xOffset = TextType::kText == fTextType && fXYIndex ? fXPos : 0;
        if (TextType::kPosText == fTextType
                && fPaint.getTextAlign() != SkPaint::kLeft_Align) { // need to measure first
            const char* text = fText;
            const SkGlyph& glyph = fGlyphCacheProc(fCache, &text);
            SkScalar width = SkScalarMul(SkFloatToScalar((&glyph.fAdvanceX)[0]), fScale);
            if (fPaint.getTextAlign() == SkPaint::kCenter_Align) {
                width = SkScalarHalf(width);
            }
            xOffset = width;
        }

        for (int i = 0; i < (int) SK_ARRAY_COUNT(fBounds); ++i) {
            SkScalar bound = fBoundsBase[i] - (fXYIndex ? x : y);
            if (fXYIndex) {
                bound += xOffset;
            }
            fBounds[i] = bound / fScale;
        }

        fXPos = xOffset + (fXYIndex ? y : x);
        fPrevAdvance = 0;
    }

private:
    SkScalar fBounds[2];
    SkScalar fBoundsBase[2];
    TextType fTextType;
};

#endif
