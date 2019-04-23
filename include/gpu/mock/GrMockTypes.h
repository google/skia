/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockOptions_DEFINED
#define GrMockOptions_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"

struct GrMockTextureInfo {
    GrPixelConfig fConfig;
    int fID;

    bool operator==(const GrMockTextureInfo& that) const {
        return fConfig == that.fConfig && fID == that.fID;
    }
};

struct GrMockRenderTargetInfo {
    GrPixelConfig fConfig;
    int fID;

    bool operator==(const GrMockRenderTargetInfo& that) const {
        return fConfig == that.fConfig && fID == that.fID;
    }
};

/**
 * A pointer to this type is used as the GrBackendContext when creating a Mock GrContext. It can be
 * used to specify capability options for the mock context. If nullptr is used a default constructed
 * GrMockOptions is used.
 */
struct GrMockOptions {
    GrMockOptions() {
        using Renderability = ConfigOptions::Renderability;
        // By default RGBA_8888 is textureable and renderable and A8 and RGB565 are texturable.
        fConfigOptions[kRGBA_8888_GrPixelConfig].fRenderability = Renderability::kNonMSAA;
        fConfigOptions[kRGBA_8888_GrPixelConfig].fTexturable = true;
        fConfigOptions[kAlpha_8_GrPixelConfig].fTexturable = true;
        fConfigOptions[kAlpha_8_as_Alpha_GrPixelConfig].fTexturable = true;
        fConfigOptions[kAlpha_8_as_Red_GrPixelConfig].fTexturable = true;
        fConfigOptions[kRGB_565_GrPixelConfig].fTexturable = true;
    }

    struct ConfigOptions {
        enum Renderability { kNo, kNonMSAA, kMSAA };
        Renderability fRenderability = kNo;
        bool fTexturable = false;
    };

    // GrCaps options.
    bool fInstanceAttribSupport = false;
    bool fHalfFloatVertexAttributeSupport = false;
    uint32_t fMapBufferFlags = 0;
    int fMaxTextureSize = 2048;
    int fMaxRenderTargetSize = 2048;
    int fMaxVertexAttributes = 16;
    ConfigOptions fConfigOptions[kGrPixelConfigCnt];

    // GrShaderCaps options.
    bool fGeometryShaderSupport = false;
    bool fIntegerSupport = false;
    bool fFlatInterpolationSupport = false;
    int fMaxVertexSamplers = 0;
    int fMaxFragmentSamplers = 8;
    bool fShaderDerivativeSupport = true;
    bool fDualSourceBlendingSupport = false;

    // GrMockGpu options.
    bool fFailTextureAllocations = false;
};

#endif
