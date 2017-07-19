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
        fBufferMapThreshold = SK_MaxS32;
        fMaxTextureSize = options.fMaxTextureSize;
        fMaxRenderTargetSize = SkTMin(options.fMaxRenderTargetSize, fMaxTextureSize);
        fMaxVertexAttributes = options.fMaxVertexAttributes;
        fShaderCaps.reset(new GrShaderCaps(contextOptions));
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
    bool canConfigBeImageStorage(GrPixelConfig) const override { return false; }
    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

private:
    GrMockOptions fOptions;
    typedef GrCaps INHERITED;
};

#endif
