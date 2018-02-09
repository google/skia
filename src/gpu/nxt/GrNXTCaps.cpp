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
    fMaxTextureSize = 2048; // FIXME
    fMaxVertexAttributes = 8; // FIXME

    this->applyOptionsOverrides(contextOptions);
    fShaderCaps->applyOptionsOverrides(contextOptions);
}

bool GrNXTCaps::isConfigRenderable(GrPixelConfig config, bool withMSAA) const {
    // FIXME: get this from NXT?
    switch (config) {
        case kRGBA_8888_GrPixelConfig:
            return withMSAA == false;
            break;
        default:
            return false;
    }
}
