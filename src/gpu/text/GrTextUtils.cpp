/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextUtils.h"
#include "GrContext.h"
#include "SkDrawFilter.h"
#include "SkGlyphCache.h"
#include "SkGr.h"
#include "SkPaint.h"
#include "SkTextBlobRunIterator.h"

void GrTextUtils::Paint::initFilteredColor() {
    // This mirrors the logic in skpaint_to_grpaint_impl for handling paint colors
    if (fDstColorSpaceInfo->colorSpace()) {
        GrColor4f filteredColor =
                SkColorToUnpremulGrColor4f(fPaint->getColor(), *fDstColorSpaceInfo);
        if (fPaint->getColorFilter()) {
            filteredColor = GrColor4f::FromSkColor4f(
                fPaint->getColorFilter()->filterColor4f(filteredColor.toSkColor4f()));
        }
        fFilteredPremulColor = filteredColor.premul().toGrColor();
    } else {
        SkColor filteredSkColor = fPaint->getColor();
        if (fPaint->getColorFilter()) {
            filteredSkColor = fPaint->getColorFilter()->filterColor(filteredSkColor);
        }
        fFilteredPremulColor = SkColorToPremulGrColor(filteredSkColor);
    }
}


bool GrTextUtils::RunPaint::modifyForRun(std::function<void(SkPaint*)> paintModFunc) {
    if (!fModifiedPaint.isValid()) {
        fModifiedPaint.init(fOriginalPaint->skPaint());
        fPaint = fModifiedPaint.get();
    } else if (fFilter) {
        // We have to reset before applying the run because the filter could have arbitrary
        // changed the paint.
        *fModifiedPaint.get() = fOriginalPaint->skPaint();
    }
    paintModFunc(fModifiedPaint.get());

    if (fFilter) {
        if (!fFilter->filter(fModifiedPaint.get(), SkDrawFilter::kText_Type)) {
            // A false return from filter() means we should abort the current draw.
            return false;
        }
        // The draw filter could have changed either the paint color or color filter.
        this->initFilteredColor();
    }
    return true;
}

bool GrTextUtils::PathTextIter::next(const SkGlyph** skGlyph, const SkPath** path, SkScalar* xpos) {
    SkASSERT(skGlyph);
    SkASSERT(path);
    SkASSERT(xpos);
    if (fText < fStop) {
        const SkGlyph& glyph = fGlyphCacheProc(fCache.get(), &fText);

        fXPos += fPrevAdvance * fScale;
        SkASSERT(0 == fXYIndex || 1 == fXYIndex);
        fPrevAdvance = SkFloatToScalar((&glyph.fAdvanceX)[fXYIndex]);

        if (glyph.fWidth) {
            if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
                *skGlyph = &glyph;
                *path = nullptr;
            } else {
                *skGlyph = nullptr;
                *path = fCache->findPath(glyph);
            }
        } else {
            *skGlyph = nullptr;
            *path = nullptr;
        }
        *xpos = fXPos;
        return true;
    }
    return false;
}
