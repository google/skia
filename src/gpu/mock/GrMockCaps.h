/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockCaps_DEFINED
#define GrMockCaps_DEFINED

#include "GrCaps.h"
#include "mock/GrMockTypes.h"

class GrMockCaps : public GrCaps {
public:
    GrMockCaps(const GrContextOptions& contextOptions, const GrMockOptions& options)
            : INHERITED(contextOptions), fOptions(options) {
        fInstanceAttribSupport = options.fInstanceAttribSupport;
        fMapBufferFlags = options.fMapBufferFlags;
        fBufferMapThreshold = SK_MaxS32; // Overridable in GrContextOptions.
        fMaxTextureSize = options.fMaxTextureSize;
        fMaxRenderTargetSize = SkTMin(options.fMaxRenderTargetSize, fMaxTextureSize);
        fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;
        fMaxVertexAttributes = options.fMaxVertexAttributes;

        fShaderCaps.reset(new GrShaderCaps(contextOptions));
        fShaderCaps->fGeometryShaderSupport = options.fGeometryShaderSupport;
        fShaderCaps->fTexelBufferSupport = options.fTexelBufferSupport;
        fShaderCaps->fIntegerSupport = options.fIntegerSupport;
        fShaderCaps->fFlatInterpolationSupport = options.fFlatInterpolationSupport;
        fShaderCaps->fMaxVertexSamplers = options.fMaxVertexSamplers;
        fShaderCaps->fMaxFragmentSamplers = options.fMaxFragmentSamplers;
        fShaderCaps->fShaderDerivativeSupport = options.fShaderDerivativeSupport;

        this->applyOptionsOverrides(contextOptions);
    }
    bool isConfigTexturable(GrPixelConfig config) const override {
        return fOptions.fConfigOptions[config].fTexturable;
    }

    bool isConfigCopyable(GrPixelConfig config) const override {
        return false;
    }

    int getRenderTargetSampleCount(int requestCount, GrPixelConfig config) const override {
        requestCount = SkTMax(requestCount, 1);
        switch (fOptions.fConfigOptions[config].fRenderability) {
            case GrMockOptions::ConfigOptions::Renderability::kNo:
                return 0;
            case GrMockOptions::ConfigOptions::Renderability::kNonMSAA:
                return requestCount > 1 ? 0 : 1;
            case GrMockOptions::ConfigOptions::Renderability::kMSAA:
                return requestCount > kMaxSampleCnt ? 0 : GrNextPow2(requestCount);
        }
        return 0;
    }

    int maxRenderTargetSampleCount(GrPixelConfig config) const override {
        switch (fOptions.fConfigOptions[config].fRenderability) {
            case GrMockOptions::ConfigOptions::Renderability::kNo:
                return 0;
            case GrMockOptions::ConfigOptions::Renderability::kNonMSAA:
                return 1;
            case GrMockOptions::ConfigOptions::Renderability::kMSAA:
                return kMaxSampleCnt;
        }
        return 0;
    }

    bool surfaceSupportsWritePixels(const GrSurface* surface) const override { return true; }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

    bool validateBackendTexture(const GrBackendTexture& tex, SkColorType,
                                GrPixelConfig* config) const override {
        const GrMockTextureInfo* texInfo = tex.getMockTextureInfo();
        if (!texInfo) {
            return false;
        }

        *config = texInfo->fConfig;
        return true;
    }

    bool validateBackendRenderTarget(const GrBackendRenderTarget& rt, SkColorType,
                                     GrPixelConfig*) const override {
        return false;
    }

    bool getConfigFromBackendFormat(const GrBackendFormat& format, SkColorType ct,
                                    GrPixelConfig* config) const override {
        const GrPixelConfig* mockFormat = format.getMockFormat();
        if (!mockFormat) {
            return false;
        }
        *config = *mockFormat;
        return true;
    }

private:
    static const int kMaxSampleCnt = 16;

    GrMockOptions fOptions;
    typedef GrCaps INHERITED;
};

#endif
