
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPaint.h"

#include "GrProcOptInfo.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrSimpleTextureEffect.h"

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

bool GrPaint::isOpaque() const {
    return this->getOpaqueAndKnownColor(NULL, NULL);
}

bool GrPaint::isOpaqueAndConstantColor(GrColor* color) const {
    GrColor tempColor = 0;
    uint32_t colorComps = 0;
    if (this->getOpaqueAndKnownColor(&tempColor, &colorComps)) {
        if (kRGBA_GrColorComponentFlags == colorComps) {
            *color = tempColor;
            return true;
        }
    }
    return false;
}

void GrPaint::resetStages() {
    fColorStages.reset();
    fCoverageStages.reset();
    fXPFactory.reset(GrPorterDuffXPFactory::Create(SkXfermode::kSrc_Mode));
}

bool GrPaint::getOpaqueAndKnownColor(GrColor* solidColor,
                                     uint32_t* solidColorKnownComponents) const {

    GrProcOptInfo coverageProcInfo;
    coverageProcInfo.calcWithInitialValues(fCoverageStages.begin(), this->numCoverageStages(),
                                           0xFFFFFFFF, kRGBA_GrColorComponentFlags, true);
    GrProcOptInfo colorProcInfo;
    colorProcInfo.calcWithInitialValues(fColorStages.begin(), this->numColorStages(), fColor,
                                        kRGBA_GrColorComponentFlags, false);

    return fXPFactory->getOpaqueAndKnownColor(colorProcInfo, coverageProcInfo, solidColor,
                                              solidColorKnownComponents);
}

