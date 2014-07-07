
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
    this->addColorEffect(GrSimpleTextureEffect::Create(texture, matrix))->unref();
}

void GrPaint::addCoverageTextureEffect(GrTexture* texture, const SkMatrix& matrix) {
    this->addCoverageEffect(GrSimpleTextureEffect::Create(texture, matrix))->unref();
}

void GrPaint::addColorTextureEffect(GrTexture* texture,
                                    const SkMatrix& matrix,
                                    const GrTextureParams& params) {
    this->addColorEffect(GrSimpleTextureEffect::Create(texture, matrix, params))->unref();
}

void GrPaint::addCoverageTextureEffect(GrTexture* texture,
                                       const SkMatrix& matrix,
                                       const GrTextureParams& params) {
    this->addCoverageEffect(GrSimpleTextureEffect::Create(texture, matrix, params))->unref();
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

    GrColor coverage = GrColorPackRGBA(fCoverage, fCoverage, fCoverage, fCoverage);
    uint32_t coverageComps = kRGBA_GrColorComponentFlags;
    int count = fCoverageStages.count();
    for (int i = 0; i < count; ++i) {
        fCoverageStages[i].getEffect()->getConstantColorComponents(&coverage, &coverageComps);
    }
    if (kRGBA_GrColorComponentFlags != coverageComps || 0xffffffff != coverage) {
        return false;
    }

    GrColor color = fColor;
    uint32_t colorComps = kRGBA_GrColorComponentFlags;
    count = fColorStages.count();
    for (int i = 0; i < count; ++i) {
        fColorStages[i].getEffect()->getConstantColorComponents(&color, &colorComps);
    }

    SkASSERT((NULL == solidColor) == (NULL == solidColorKnownComponents));

    GrBlendCoeff srcCoeff = fSrcBlendCoeff;
    GrBlendCoeff dstCoeff = fDstBlendCoeff;
    GrSimplifyBlend(&srcCoeff, &dstCoeff, color, colorComps, 0, 0, 0);

    bool opaque = kZero_GrBlendCoeff == dstCoeff && !GrBlendCoeffRefsDst(srcCoeff);
    if (NULL != solidColor) {
        if (opaque) {
            switch (srcCoeff) {
                case kZero_GrBlendCoeff:
                    *solidColor = 0;
                    *solidColorKnownComponents = kRGBA_GrColorComponentFlags;
                    break;

                case kOne_GrBlendCoeff:
                    *solidColor = color;
                    *solidColorKnownComponents = colorComps;
                    break;

                // The src coeff should never refer to the src and if it refers to dst then opaque
                // should have been false.
                case kSC_GrBlendCoeff:
                case kISC_GrBlendCoeff:
                case kDC_GrBlendCoeff:
                case kIDC_GrBlendCoeff:
                case kSA_GrBlendCoeff:
                case kISA_GrBlendCoeff:
                case kDA_GrBlendCoeff:
                case kIDA_GrBlendCoeff:
                default:
                    SkFAIL("srcCoeff should not refer to src or dst.");
                    break;

                // TODO: update this once GrPaint actually has a const color.
                case kConstC_GrBlendCoeff:
                case kIConstC_GrBlendCoeff:
                case kConstA_GrBlendCoeff:
                case kIConstA_GrBlendCoeff:
                    *solidColorKnownComponents = 0;
                    break;
            }
        } else {
            solidColorKnownComponents = 0;
        }
    }
    return opaque;
}
