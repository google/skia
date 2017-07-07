/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockOptions_DEFINED
#define GrMockOptions_DEFINED

#include "GrTypes.h"

struct GrMockTextureInfo {
    int fID;
};

/**
 * A pointer to this type is used as the GrBackendContext when creating a Mock GrContext. It can be
 * used to specificy capability options for the mock context. If nullptr is used a default
 * constructed GrMockOptions is used.
 */
struct GrMockOptions {
    GrMockOptions() {
        // By default RGBA_8888 is textureable and renderable and A8 and RGB565 are texturable.
        fConfigOptions[kRGBA_8888_GrPixelConfig].fRenderable[0] = true;
        fConfigOptions[kRGBA_8888_GrPixelConfig].fTexturable = true;
        fConfigOptions[kAlpha_8_GrPixelConfig].fTexturable = true;
        fConfigOptions[kRGB_565_GrPixelConfig].fTexturable = true;
    }

    struct ConfigOptions {
        /** The first value is for non-MSAA rendering, the second for MSAA. */
        bool fRenderable[2] = {false, false};
        bool fTexturable = false;
    };

    int fMaxTextureSize = 2048;
    int fMaxRenderTargetSize = 2048;
    int fMaxVertexAttributes = 16;
    ConfigOptions fConfigOptions[kGrPixelConfigCnt];
};

#endif
