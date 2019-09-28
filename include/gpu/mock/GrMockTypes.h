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

class GrBackendFormat;

struct GrMockTextureInfo {
    GrMockTextureInfo()
        : fColorType(GrColorType::kUnknown)
        , fID(0) {}

    GrMockTextureInfo(GrColorType colorType, int id)
            : fColorType(colorType)
            , fID(id) {
        SkASSERT(fID);
    }

    bool operator==(const GrMockTextureInfo& that) const {
        return fColorType == that.fColorType &&
               fID == that.fID;
    }

    GrPixelConfig pixelConfig() const {
        return GrColorTypeToPixelConfig(fColorType);
    }

    GrBackendFormat getBackendFormat() const;

    GrColorType   fColorType;
    int           fID;
};

struct GrMockRenderTargetInfo {
    GrMockRenderTargetInfo()
            : fColorType(GrColorType::kUnknown)
            , fID(0) {}

    GrMockRenderTargetInfo(GrColorType colorType, int id)
            : fColorType(colorType)
            , fID(id) {
        SkASSERT(fID);
    }

    bool operator==(const GrMockRenderTargetInfo& that) const {
        return fColorType == that.fColorType &&
               fID == that.fID;
    }

    GrPixelConfig pixelConfig() const {
        return GrColorTypeToPixelConfig(fColorType);
    }

    GrBackendFormat getBackendFormat() const;

    GrColorType colorType() const { return fColorType; }

private:
    GrColorType   fColorType;
    int           fID;
};

/**
 * A pointer to this type is used as the GrBackendContext when creating a Mock GrContext. It can be
 * used to specify capability options for the mock context. If nullptr is used a default constructed
 * GrMockOptions is used.
 */
struct GrMockOptions {
    GrMockOptions() {
        using Renderability = ConfigOptions::Renderability;
        // By default RGBA_8888 and BGRA_8888 are textureable and renderable and
        // A8 and RGB565 are texturable.
        fConfigOptions[(int)GrColorType::kRGBA_8888].fRenderability = Renderability::kNonMSAA;
        fConfigOptions[(int)GrColorType::kRGBA_8888].fTexturable = true;
        fConfigOptions[(int)GrColorType::kAlpha_8].fTexturable = true;
        fConfigOptions[(int)GrColorType::kBGR_565].fTexturable = true;

        fConfigOptions[(int)GrColorType::kBGRA_8888] = fConfigOptions[(int)GrColorType::kRGBA_8888];
    }

    struct ConfigOptions {
        enum Renderability { kNo, kNonMSAA, kMSAA };
        Renderability fRenderability = kNo;
        bool fTexturable = false;
    };

    // GrCaps options.
    bool fMipMapSupport = false;
    bool fInstanceAttribSupport = false;
    bool fHalfFloatVertexAttributeSupport = false;
    uint32_t fMapBufferFlags = 0;
    int fMaxTextureSize = 2048;
    int fMaxRenderTargetSize = 2048;
    int fMaxVertexAttributes = 16;
    ConfigOptions fConfigOptions[kGrColorTypeCnt];

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
