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
    int getSampleCount(int /*requestCount*/, GrPixelConfig /*config*/) const override {
        return 0;
    }
    bool isConfigTexturable(GrPixelConfig config) const override {
        return fOptions.fConfigOptions[config].fTexturable;
    }
    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override {
        return fOptions.fConfigOptions[config].fRenderable[withMSAA];
    }
    bool isConfigCopyable(GrPixelConfig config) const override {
        return false;
    }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

    bool validateBackendTexture(const GrBackendTexture& tex, SkColorType,
                                GrPixelConfig*) const override {
        return SkToBool(tex.getMockTextureInfo());
    }

    bool validateBackendRenderTarget(const GrBackendRenderTarget& rt, SkColorType,
                                     GrPixelConfig*) const override {
        return false;
    }

private:
    GrMockOptions fOptions;
    typedef GrCaps INHERITED;
};

#endif
