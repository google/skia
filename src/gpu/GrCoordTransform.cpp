/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCoordTransform.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrGpu.h"

void GrCoordTransform::reset(GrCoordSet sourceCoords, const SkMatrix& m, const GrTexture* texture,
                             GrTextureParams::FilterMode filter) {
    SkASSERT(texture);
    SkASSERT(!fInProcessor);

    fSourceCoords = sourceCoords;
    fMatrix = m;
    fReverseY = kBottomLeft_GrSurfaceOrigin == texture->origin();

    // Always start at kDefault. Then if precisions differ we see if the precision needs to be
    // increased. Our rule is that we want at least 4 subpixel values in the representation for
    // coords between 0 to 1 when bi- or tri-lerping and 1 value when nearest filtering. Note that
    // this still might not be enough when drawing with repeat or mirror-repeat modes but that case
    // can be arbitrarily bad.
    int subPixelThresh = filter > GrTextureParams::kNone_FilterMode ? 4 : 1;
    fPrecision = kDefault_GrSLPrecision;
    if (texture->getContext()) {
        const GrShaderCaps* caps = texture->getContext()->caps()->shaderCaps();
        if (caps->floatPrecisionVaries()) {
            int maxD = SkTMax(texture->width(), texture->height());
            const GrShaderCaps::PrecisionInfo* info;
            info = &caps->getFloatShaderPrecisionInfo(kFragment_GrShaderType, fPrecision);
            do {
                SkASSERT(info->supported());
                // Make sure there is at least 2 bits of subpixel precision in the range of
                // texture coords from 0.5 to 1.0.
                if ((2 << info->fBits) / maxD > subPixelThresh) {
                    break;
                }
                if (kHigh_GrSLPrecision == fPrecision) {
                    break;
                }
                GrSLPrecision nextP = static_cast<GrSLPrecision>(fPrecision + 1);
                info = &caps->getFloatShaderPrecisionInfo(kFragment_GrShaderType, nextP);
                if (!info->supported()) {
                    break;
                }
                fPrecision = nextP;
            } while (true);
        }
    }
}

void GrCoordTransform::reset(GrCoordSet sourceCoords,
                             const SkMatrix& m,
                             GrSLPrecision precision) {
    SkASSERT(!fInProcessor);
    fSourceCoords = sourceCoords;
    fMatrix = m;
    fReverseY = false;
    fPrecision = precision;
}
