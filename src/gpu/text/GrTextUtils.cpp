/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextUtils.h"
#include "GrContext.h"
#include "SkGlyphCache.h"
#include "SkGr.h"
#include "SkPaint.h"

void GrTextUtils::Paint::initFilteredColor() {
    GrColor4f filteredColor = SkColorToUnpremulGrColor4f(fPaint->getColor(), *fDstColorSpaceInfo);
    if (fPaint->getColorFilter()) {
        filteredColor = GrColor4f::FromSkColor4f(
            fPaint->getColorFilter()->filterColor4f(filteredColor.toSkColor4f(),
                                                    fDstColorSpaceInfo->colorSpace()));
    }
    fFilteredPremulColor = filteredColor.premul().toGrColor();
}


bool GrTextUtils::RunPaint::modifyForRun(std::function<void(SkPaint*)> paintModFunc) {
    if (!fModifiedPaint.isValid()) {
        fModifiedPaint.init(fOriginalPaint->skPaint());
        fPaint = fModifiedPaint.get();
    }
    paintModFunc(fModifiedPaint.get());
    return true;
}

