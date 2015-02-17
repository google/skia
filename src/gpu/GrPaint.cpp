
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
    , fColor(GrColor_WHITE) {
}

void GrPaint::setCoverageSetOpXPFactory(SkRegion::Op regionOp, bool invertCoverage) {
    fXPFactory.reset(GrCoverageSetOpXPFactory::Create(regionOp, invertCoverage));
}

void GrPaint::addColorTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
    this->addColorProcessor(GrSimpleTextureEffect::Create(texture, matrix))->unref();
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture, const SkMatrix& matrix) {
    this->addCoverageProcessor(GrSimpleTextureEffect::Create(texture, matrix))->unref();
}

void GrPaint::addColorTextureProcessor(GrTexture* texture,
                                       const SkMatrix& matrix,
                                       const GrTextureParams& params) {
    this->addColorProcessor(GrSimpleTextureEffect::Create(texture, matrix, params))->unref();
}

void GrPaint::addCoverageTextureProcessor(GrTexture* texture,
                                          const SkMatrix& matrix,
                                          const GrTextureParams& params) {
    this->addCoverageProcessor(GrSimpleTextureEffect::Create(texture, matrix, params))->unref();
}

bool GrPaint::isOpaqueAndConstantColor(GrColor* color) const {
    GrProcOptInfo coverageProcInfo;
    coverageProcInfo.calcWithInitialValues(fCoverageStages.begin(), this->numCoverageStages(),
                                           0xFFFFFFFF, kRGBA_GrColorComponentFlags, true);
    GrProcOptInfo colorProcInfo;
    colorProcInfo.calcWithInitialValues(fColorStages.begin(), this->numColorStages(), fColor,
                                        kRGBA_GrColorComponentFlags, false);

    GrXPFactory::InvariantOutput output;
    fXPFactory->getInvariantOutput(colorProcInfo, coverageProcInfo, &output);

    if (kRGBA_GrColorComponentFlags == output.fBlendedColorFlags &&
        0xFF == GrColorUnpackA(output.fBlendedColor)) {
        *color = output.fBlendedColor;
        return true;
    }
    return false;
}
