/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrPaint.h"

#include "include/core/SkBlendMode.h"
#include "src/gpu/ganesh/effects/GrCoverageSetOpXP.h"
#include "src/gpu/ganesh/effects/GrPorterDuffXferProcessor.h"

GrPaint::GrPaint(const GrPaint& that)
        : fXPFactory(that.fXPFactory)
        , fTrivial(that.fTrivial)
        , fColor(that.fColor) {
    if (that.fColorFragmentProcessor) {
        fColorFragmentProcessor = that.fColorFragmentProcessor->clone();
        SkASSERT(fColorFragmentProcessor);
    }
    if (that.fCoverageFragmentProcessor) {
        fCoverageFragmentProcessor = that.fCoverageFragmentProcessor->clone();
        SkASSERT(fCoverageFragmentProcessor);
    }
}

void GrPaint::setPorterDuffXPFactory(SkBlendMode mode) {
    this->setXPFactory(GrPorterDuffXPFactory::Get(mode));
}

void GrPaint::setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage) {
    this->setXPFactory(GrCoverageSetOpXPFactory::Get(regionOp, invertCoverage));
}

bool GrPaint::isConstantBlendedColor(SkPMColor4f* constantColor) const {
    // This used to do a more sophisticated analysis but now it just explicitly looks for common
    // cases.
    static const GrXPFactory* kSrc = GrPorterDuffXPFactory::Get(SkBlendMode::kSrc);
    static const GrXPFactory* kClear = GrPorterDuffXPFactory::Get(SkBlendMode::kClear);
    if (kClear == fXPFactory) {
        *constantColor = SK_PMColor4fTRANSPARENT;
        return true;
    }
    if (this->hasColorFragmentProcessor()) {
        return false;
    }
    if (kSrc == fXPFactory || (!fXPFactory && fColor.isOpaque())) {
        *constantColor = fColor;
        return true;
    }
    return false;
}
