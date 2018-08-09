/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTCaps.h"

GrNXTCaps::GrNXTCaps(const GrContextOptions& contextOptions) : INHERITED(contextOptions) {
    fBufferMapThreshold = SK_MaxS32;  // FIXME: get this from NXT?
    fShaderCaps.reset(new GrShaderCaps(contextOptions));
    fMaxTextureSize = fMaxRenderTargetSize = 4096; // FIXME
    fMaxVertexAttributes = 8; // FIXME
    fUseDrawForClear = true;

    fShaderCaps->fFlatInterpolationSupport = true;
    fShaderCaps->fIntegerSupport = true;
    fShaderCaps->fMaxFragmentSamplers = 16; // FIXME
    fShaderCaps->fMaxCombinedSamplers = 16; // FIXME
    fShaderCaps->fConfigTextureSwizzle[kAlpha_8_GrPixelConfig] = GrSwizzle::RRRR();
    fShaderCaps->fShaderDerivativeSupport = true;

    this->applyOptionsOverrides(contextOptions);
    fShaderCaps->applyOptionsOverrides(contextOptions);
}

bool GrNXTCaps::isConfigTexturable(GrPixelConfig config) const {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
            return true;
        default:
            return false;
    }
}
