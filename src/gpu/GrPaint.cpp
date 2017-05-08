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

void GrPaint::setPorterDuffXPFactory(SkBlendMode mode) {
    fXPFactory = GrPorterDuffXPFactory::Get(mode);
}

void GrPaint::setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage) {
    fXPFactory = GrCoverageSetOpXPFactory::Get(regionOp, invertCoverage);
}

void GrPaint::addColorTextureProcessor(GrResourceProvider* resourceProvider,
                                       sk_sp<GrTextureProxy> proxy,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(resourceProvider, std::move(proxy),
                                                                std::move(colorSpaceXform),
                                                                matrix));
}

void GrPaint::addColorTextureProcessor(GrResourceProvider* resourceProvider,
                                       sk_sp<GrTextureProxy> proxy,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix,
                                       const GrSamplerParams& params) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(resourceProvider,
                                                                std::move(proxy),
                                                                std::move(colorSpaceXform),
                                                                matrix, params));
}

void GrPaint::addCoverageTextureProcessor(GrResourceProvider* resourceProvider,
                                          sk_sp<GrTextureProxy> proxy,
                                          const SkMatrix& matrix) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(resourceProvider,
                                                                   std::move(proxy),
                                                                   nullptr, matrix));
}

void GrPaint::addCoverageTextureProcessor(GrResourceProvider* resourceProvider,
                                          sk_sp<GrTextureProxy> proxy,
                                          const SkMatrix& matrix,
                                          const GrSamplerParams& params) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(resourceProvider,
                                                                   std::move(proxy),
                                                                   nullptr, matrix, params));
}

bool GrPaint::isConstantBlendedColor(GrColor* constantColor) const {
    // This used to do a more sophisticated analysis but now it just explicitly looks for common
    // cases.
    static const GrXPFactory* kSrc = GrPorterDuffXPFactory::Get(SkBlendMode::kSrc);
    static const GrXPFactory* kClear = GrPorterDuffXPFactory::Get(SkBlendMode::kClear);
    if (kClear == fXPFactory) {
        *constantColor = GrColor_TRANSPARENT_BLACK;
        return true;
    }
    if (this->numColorFragmentProcessors()) {
        return false;
    }
    if (kSrc == fXPFactory || (!fXPFactory && fColor.isOpaque())) {
        *constantColor = fColor.toGrColor();
        return true;
    }
    return false;
}
