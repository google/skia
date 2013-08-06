
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPaint.h"

#include "GrBlend.h"
#include "effects/GrSimpleTextureEffect.h"

void GrPaint::addColorTextureEffect(GrTexture* texture, const SkMatrix& matrix) {
    GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix);
    this->addColorEffect(effect)->unref();
}

void GrPaint::addCoverageTextureEffect(GrTexture* texture, const SkMatrix& matrix) {
    GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix);
    this->addCoverageEffect(effect)->unref();
}

void GrPaint::addColorTextureEffect(GrTexture* texture,
                                    const SkMatrix& matrix,
                                    const GrTextureParams& params) {
    GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix, params);
    this->addColorEffect(effect)->unref();
}

void GrPaint::addCoverageTextureEffect(GrTexture* texture,
                                       const SkMatrix& matrix,
                                       const GrTextureParams& params) {
    GrEffectRef* effect = GrSimpleTextureEffect::Create(texture, matrix, params);
    this->addCoverageEffect(effect)->unref();
}

bool GrPaint::isOpaque() const {
    return this->getOpaqueAndKnownColor(NULL, NULL);
}

bool GrPaint::isOpaqueAndConstantColor(GrColor* color) const {
    GrColor tempColor;
    uint32_t colorComps;
    if (this->getOpaqueAndKnownColor(&tempColor, &colorComps)) {
        if (kRGBA_GrColorComponentFlags == colorComps) {
            *color = tempColor;
            return true;
        }
    }
    return false;
}

bool GrPaint::getOpaqueAndKnownColor(GrColor* solidColor,
                                     uint32_t* solidColorKnownComponents) const {

    // TODO: Share this implementation with GrDrawState

    // Since fColorFilterXfermode is going away soon, we aren't attempting to handle anything but
    // the default setting.
    if (SkXfermode::kDst_Mode != fColorFilterXfermode) {
        return false;
    }

    GrColor coverage = GrColorPackRGBA(fCoverage, fCoverage, fCoverage, fCoverage);
    uint32_t coverageComps = kRGBA_GrColorComponentFlags;
    int count = fCoverageStages.count();
    for (int i = 0; i < count; ++i) {
        (*fCoverageStages[i].getEffect())->getConstantColorComponents(&coverage, &coverageComps);
    }
    if (kRGBA_GrColorComponentFlags != coverageComps || 0xffffffff != coverage) {
        return false;
    }

    GrColor color = fColor;
    uint32_t colorComps = kRGBA_GrColorComponentFlags;
    count = fColorStages.count();
    for (int i = 0; i < count; ++i) {
        (*fColorStages[i].getEffect())->getConstantColorComponents(&color, &colorComps);
    }

    GrAssert((NULL == solidColor) == (NULL == solidColorKnownComponents));
    if (NULL != solidColor) {
        *solidColor = color;
        *solidColorKnownComponents = colorComps;
    }

    GrBlendCoeff srcCoeff = fSrcBlendCoeff;
    GrBlendCoeff dstCoeff = fDstBlendCoeff;
    GrSimplifyBlend(&srcCoeff, &dstCoeff, color, colorComps, 0, 0, 0);
    return kZero_GrBlendCoeff == dstCoeff && !GrBlendCoeffRefsDst(srcCoeff);
}
