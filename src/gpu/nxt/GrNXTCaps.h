/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTCaps_DEFINED
#define GrNXTCaps_DEFINED

#include "GrCaps.h"
#include "GrContextOptions.h"

class GrNXTCaps : public GrCaps {
public:
    GrNXTCaps(const GrContextOptions& contextOptions);

    bool isConfigTexturable(GrPixelConfig config) const override;

    bool isConfigCopyable(GrPixelConfig config) const override {
        return true;
    }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc, GrSurfaceOrigin*,
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

    bool getConfigFromBackendFormat(const GrBackendFormat&, SkColorType,
                                    GrPixelConfig*) const override {
        return false;
    }

    bool surfaceSupportsWritePixels(const GrSurface* surface) const override {
        return true;
    }

    int getRenderTargetSampleCount(int requestedCount, GrPixelConfig) const override {
        return 1;
    }

    int maxRenderTargetSampleCount(GrPixelConfig) const override {
        return 1;
    }

    typedef GrCaps INHERITED;
};

#endif
