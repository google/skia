
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
    , fDither(false)
    , fColor(GrColor_WHITE)
    , fProcDataManager(SkNEW(GrProcessorDataManager)) {
}

void GrPaint::setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage) {
    fXPFactory.reset(GrCoverageSetOpXPFactory::Create(regionOp, invertCoverage));
}

void GrPaint::addColorTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Create(fProcDataManager, texture,
                                                          matrix))->unref();
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Create(fProcDataManager, texture,
                                                             matrix))->unref();
}

void GrPaint::addColorTextureProcessor(GrTexture* texture,
                                       const SkMatrix& matrix,
                                       const GrTextureParams& params) {
    this->addColorFragmentProcessor(GrSimpleTextureEffect::Create(fProcDataManager, texture, matrix,
                                                          params))->unref();
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture,
                                          const SkMatrix& matrix,
                                          const GrTextureParams& params) {
    this->addCoverageFragmentProcessor(GrSimpleTextureEffect::Create(fProcDataManager, texture, matrix,
                                                             params))->unref();
}

bool GrPaint::isConstantBlendedColor(GrColor* color) const {
    GrProcOptInfo colorProcInfo;
    colorProcInfo.calcWithInitialValues(fColorFragmentProcessors.begin(),
                                        this->numColorFragmentProcessors(), fColor,
                                        kRGBA_GrColorComponentFlags, false);

    GrXPFactory::InvariantBlendedColor blendedColor;
    fXPFactory->getInvariantBlendedColor(colorProcInfo, &blendedColor);

    if (kRGBA_GrColorComponentFlags == blendedColor.fKnownColorFlags) {
        *color = blendedColor.fKnownColor;
        return true;
    }
    return false;
}
