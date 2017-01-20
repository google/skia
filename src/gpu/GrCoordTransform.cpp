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
#include "GrSurfaceProxy.h"

static GrSLPrecision compute_precision(const GrShaderCaps* caps, int width, int height,
                                       GrSamplerParams::FilterMode filter) {
    // Always start at kDefault. Then if precisions differ we see if the precision needs to be
    // increased. Our rule is that we want at least 4 subpixel values in the representation for
    // coords between 0 to 1 when bi- or tri-lerping and 1 value when nearest filtering. Note that
    // this still might not be enough when drawing with repeat or mirror-repeat modes but that case
    // can be arbitrarily bad.
    int subPixelThresh = filter > GrSamplerParams::kNone_FilterMode ? 4 : 1;
    GrSLPrecision precision = kDefault_GrSLPrecision;
    if (caps) {
        if (caps->floatPrecisionVaries()) {
            int maxD = SkTMax(width, height);
            const GrShaderCaps::PrecisionInfo* info;
            info = &caps->getFloatShaderPrecisionInfo(kFragment_GrShaderType, precision);
            do {
                SkASSERT(info->supported());
                // Make sure there is at least 2 bits of subpixel precision in the range of
                // texture coords from 0.5 to 1.0.
                if ((2 << info->fBits) / maxD > subPixelThresh) {
                    break;
                }
                if (kHigh_GrSLPrecision == precision) {
                    break;
                }
                GrSLPrecision nextP = static_cast<GrSLPrecision>(precision + 1);
                info = &caps->getFloatShaderPrecisionInfo(kFragment_GrShaderType, nextP);
                if (!info->supported()) {
                    break;
                }
                precision = nextP;
            } while (true);
        }
    }

    return precision;
}

void GrCoordTransform::reset(const SkMatrix& m, const GrTexture* texture,
                             GrSamplerParams::FilterMode filter) {
    SkASSERT(texture);
    SkASSERT(!fInProcessor);

    fMatrix = m;
    fReverseY = kBottomLeft_GrSurfaceOrigin == texture->origin();

    if (texture->getContext()) {
        fPrecision = compute_precision(texture->getContext()->caps()->shaderCaps(),
                                       texture->width(), texture->height(), filter);
    } else {
        fPrecision = kDefault_GrSLPrecision;
    }
}

void GrCoordTransform::reset(const SkMatrix& m, const GrSurfaceProxy* proxy,
                             GrSamplerParams::FilterMode filter) {
    SkASSERT(proxy);
    SkASSERT(!fInProcessor);

    fMatrix = m;
    fReverseY = kBottomLeft_GrSurfaceOrigin == proxy->origin();

    GrCaps* caps = nullptr;
    fPrecision = compute_precision(caps->shaderCaps(),
                                   proxy->worstCaseWidth(*caps),
                                   proxy->worstCaseHeight(*caps), filter);
}

void GrCoordTransform::reset(const SkMatrix& m, GrSLPrecision precision) {
    SkASSERT(!fInProcessor);
    fMatrix = m;
    fReverseY = false;
    fPrecision = precision;
}
