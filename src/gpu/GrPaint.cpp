/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPaint.h"
#include "GrXferProcessor.h"
#include "effects/GrCoverageSetOpXP.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

GrPaint::GrPaint(const GrPaint& that)
        : fXPFactory(that.fXPFactory)
        , fColorFragmentProcessors(that.fColorFragmentProcessors.count())
        , fCoverageFragmentProcessors(that.fCoverageFragmentProcessors.count())
        , fTrivial(that.fTrivial)
        , fColor(that.fColor) {
    for (int i = 0; i < that.fColorFragmentProcessors.count(); ++i) {
        fColorFragmentProcessors.push_back(that.fColorFragmentProcessors[i]->clone());
        SkASSERT(fColorFragmentProcessors[i]);
    }
    for (int i = 0; i < that.fCoverageFragmentProcessors.count(); ++i) {
        fCoverageFragmentProcessors.push_back(that.fCoverageFragmentProcessors[i]->clone());
        SkASSERT(fCoverageFragmentProcessors[i]);
    }
}

void GrPaint::setPorterDuffXPFactory(SkBlendMode mode) {
    this->setXPFactory(GrPorterDuffXPFactory::Get(mode));
}

void GrPaint::setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage) {
    this->setXPFactory(GrCoverageSetOpXPFactory::Get(regionOp, invertCoverage));
}

void GrPaint::addColorTextureProcessor(sk_sp<GrTextureProxy> proxy, const SkMatrix& matrix) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(std::move(proxy), matrix));
}

void GrPaint::addColorTextureProcessor(sk_sp<GrTextureProxy> proxy, const SkMatrix& matrix,
                                       const GrSamplerState& samplerState) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(std::move(proxy), matrix,
                                                                samplerState));
}

void GrPaint::addCoverageTextureProcessor(sk_sp<GrTextureProxy> proxy,
                                          const SkMatrix& matrix) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(std::move(proxy), matrix));
}

void GrPaint::addCoverageTextureProcessor(sk_sp<GrTextureProxy> proxy,
                                          const SkMatrix& matrix,
                                          const GrSamplerState& params) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(std::move(proxy), matrix,
                                                                   params));
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
    if (this->numColorFragmentProcessors()) {
        return false;
    }
    if (kSrc == fXPFactory || (!fXPFactory && fColor.isOpaque())) {
        *constantColor = fColor;
        return true;
    }
    return false;
}
