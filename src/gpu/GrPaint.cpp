/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPaint.h"

#include "GrProcOptInfo.h"
#include "effects/GrCoverageSetOpXP.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

void GrPaint::setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage) {
    fXPFactory = GrCoverageSetOpXPFactory::Get(regionOp, invertCoverage);
}

void GrPaint::addColorTextureProcessor(GrTexture* texture,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix, bool bFoo) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(texture,
                                                                std::move(colorSpaceXform),
                                                                matrix, bFoo)); //$$
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture, const SkMatrix& matrix, bool bFoo) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(texture, nullptr, matrix, bFoo)); //$$
}

void GrPaint::addColorTextureProcessor(GrTexture* texture,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix, bool bFoo,
                                       const GrSamplerParams& params) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(texture,
                                                                std::move(colorSpaceXform),
                                                                matrix, bFoo, params)); //$$
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture,
                                          const SkMatrix& matrix, bool bFoo,
                                          const GrSamplerParams& params) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(texture, nullptr, matrix, bFoo,
                                                                   params)); //$$
}

bool GrPaint::internalIsConstantBlendedColor(GrColor paintColor, GrColor* color) const {
    GrProcOptInfo colorProcInfo;
    colorProcInfo.calcWithInitialValues(
        sk_sp_address_as_pointer_address(fColorFragmentProcessors.begin()),
        this->numColorFragmentProcessors(), paintColor, kRGBA_GrColorComponentFlags, false);

    GrXPFactory::InvariantBlendedColor blendedColor;
    if (fXPFactory) {
        fXPFactory->getInvariantBlendedColor(colorProcInfo, &blendedColor);
    } else {
        GrPorterDuffXPFactory::SrcOverInvariantBlendedColor(colorProcInfo.color(),
                                                            colorProcInfo.validFlags(),
                                                            colorProcInfo.isOpaque(),
                                                            &blendedColor);
    }

    if (kRGBA_GrColorComponentFlags == blendedColor.fKnownColorFlags) {
        *color = blendedColor.fKnownColor;
        return true;
    }
    return false;
}
