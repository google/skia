/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCaps_DEFINED
#define GrMtlCaps_DEFINED

#include "GrCaps.h"

#include "SkTDArray.h"

#import <Metal/Metal.h>

class GrShaderCaps;

/**
 * Stores some capabilities of a Mtl backend.
 */
class GrMtlCaps : public GrCaps {
public:
    GrMtlCaps(const GrContextOptions& contextOptions, id<MTLDevice> device,
              MTLFeatureSet featureSet);

    int getSampleCount(int requestedCount, GrPixelConfig config) const override;

    bool isConfigTexturable(GrPixelConfig config) const override {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kTextureable_Flag);
    }

    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override {
        if (withMSAA) {
            return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kRenderable_Flag) &&
                   SkToBool(fConfigTable[config].fFlags & ConfigInfo::kMSAA_Flag);
        } else {
            return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kRenderable_Flag);
        }
    }

    bool isConfigCopyable(GrPixelConfig config) const override {
        return true;
    }

#if 0
    /**
     * Returns both a supported and most prefered stencil format to use in draws.
     */
    const StencilFormat& preferedStencilFormat() const {
        return fPreferedStencilFormat;
    }
#endif
    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

    bool validateBackendTexture(const GrBackendTexture&, SkColorType,
                                GrPixelConfig*) const override {
        return false;
    }
    bool validateBackendRenderTarget(const GrBackendRenderTarget&, SkColorType,
                                     GrPixelConfig*) const override {
        return false;
    }

private:
    void initFeatureSet(MTLFeatureSet featureSet);

    void initGrCaps(const id<MTLDevice> device);
    void initShaderCaps();
    void initSampleCount();
    void initConfigTable();

    struct ConfigInfo {
        ConfigInfo() : fFlags(0) {}

        enum {
            kTextureable_Flag = 0x1,
            kRenderable_Flag  = 0x2, // Color attachment and blendable
            kMSAA_Flag        = 0x4,
            kResolve_Flag     = 0x8,
        };
        static const uint16_t kAllFlags = kTextureable_Flag | kRenderable_Flag |
                                          kMSAA_Flag | kResolve_Flag;

        uint16_t fFlags;
    };
    ConfigInfo fConfigTable[kGrPixelConfigCnt];

    enum class Platform {
        kMac,
        kIOS
    };
    bool isMac() { return Platform::kMac == fPlatform; }
    bool isIOS() { return Platform::kIOS == fPlatform; }

    Platform fPlatform;
    int fFamilyGroup;
    int fVersion;

    SkTDArray<int> fSampleCounts;

    typedef GrCaps INHERITED;
};

#endif
