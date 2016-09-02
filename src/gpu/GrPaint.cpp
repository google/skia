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

GrPaint::GrPaint()
    : fAntiAlias(false)
    , fDisableOutputConversionToSRGB(false)
    , fAllowSRGBInputs(false)
    , fUsesDistanceVectorField(false)
    , fColor(GrColor4f::FromGrColor(GrColor_WHITE)) {}

void GrPaint::setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage) {
    fXPFactory = GrCoverageSetOpXPFactory::Make(regionOp, invertCoverage);
}

void GrPaint::addColorTextureProcessor(GrTexture* texture,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(texture,
                                                                std::move(colorSpaceXform),
                                                                matrix));
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(texture, nullptr, matrix));
}

void GrPaint::addColorTextureProcessor(GrTexture* texture,
                                       sk_sp<GrColorSpaceXform> colorSpaceXform,
                                       const SkMatrix& matrix,
                                       const GrTextureParams& params) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Make(texture,
                                                                std::move(colorSpaceXform),
                                                                matrix, params));
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture,
                                          const SkMatrix& matrix,
                                          const GrTextureParams& params) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(texture, nullptr, matrix,
                                                                   params));
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
