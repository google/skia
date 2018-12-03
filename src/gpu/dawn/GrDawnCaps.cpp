/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnCaps.h"

GrDawnCaps::GrDawnCaps(const GrContextOptions& contextOptions) : INHERITED(contextOptions) {
    fBufferMapThreshold = SK_MaxS32;  // FIXME: get this from Dawn?
    fShaderCaps.reset(new GrShaderCaps(contextOptions));
    fMaxTextureSize = fMaxRenderTargetSize = 4096; // FIXME
    fMaxVertexAttributes = 16; // FIXME

    fShaderCaps->fFlatInterpolationSupport = true;
    fShaderCaps->fIntegerSupport = true;
    // FIXME: each fragment sampler takes two binding slots in Dawn (sampler + texture). Limit to
    // 6 * 2 = 12, since kMaxBindingsPerGroup is 16 in Dawn, and we need to keep a few for
    // non-texture bindings. Eventually, we may be able to increase kMaxBindingsPerGroup in Dawn.
    fShaderCaps->fMaxFragmentSamplers = 6;
    fShaderCaps->fConfigTextureSwizzle[kAlpha_8_GrPixelConfig] = GrSwizzle::RRRR();
    fShaderCaps->fShaderDerivativeSupport = true;

    this->applyOptionsOverrides(contextOptions);
    fShaderCaps->applyOptionsOverrides(contextOptions);
}

bool GrDawnCaps::isConfigTexturable(GrPixelConfig config) const {
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
        case kBGRA_8888_GrPixelConfig:
        case kRGBA_4444_GrPixelConfig:
        case kRGB_565_GrPixelConfig:
        case kGray_8_GrPixelConfig:
        case kAlpha_8_GrPixelConfig:
            return true;
        default:
            return false;
    }
}
