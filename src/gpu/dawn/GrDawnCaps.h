/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnCaps_DEFINED
#define GrDawnCaps_DEFINED

#include "GrCaps.h"
#include "GrContextOptions.h"
#include "GrBackendSurface.h"

class GrDawnCaps : public GrCaps {
public:
    GrDawnCaps(const GrContextOptions& contextOptions);

    bool isConfigTexturable(GrPixelConfig config) const override;

    bool isConfigCopyable(GrPixelConfig config) const override {
        return true;
    }

    bool canCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                        const SkIRect& srcRect, const SkIPoint& dstPoint) const override {
        return true;
    }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            GrSurfaceOrigin* origi, bool* rectsMustMatch,
                            bool* disallowSubrect) const override {
        return false;
    }

    bool validateBackendTexture(const GrBackendTexture&, SkColorType,
                                GrPixelConfig*) const override {
        return true;
    }
    bool validateBackendRenderTarget(const GrBackendRenderTarget&, SkColorType,
                                     GrPixelConfig*) const override {
        return true;
    }

    bool getConfigFromBackendFormat(const GrBackendFormat&, SkColorType,
                                    GrPixelConfig*) const override {
        return false;
    }

    bool getYUVAConfigFromBackendFormat(const GrBackendFormat&,
                                        GrPixelConfig*) const override {
        return false;
    }
    bool getYUVAConfigFromBackendTexture(const GrBackendTexture&,
                                         GrPixelConfig*) const override {
        return false;
    }

#ifdef GR_TEST_UTILS
    GrBackendFormat onCreateFormatFromBackendTexture(
        const GrBackendTexture& backendTex) const override {
        return GrBackendFormat(); // Dawn BackendFormat not yet implemented.
    }
#endif

    bool surfaceSupportsReadPixels(const GrSurface*) const override {
        return true;
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

    bool performPartialClearsAsDraws() const override {
        return true;
    }

    GrSurfaceOrigin renderTargetOrigin() const override {
        return kTopLeft_GrSurfaceOrigin;
    }

    typedef GrCaps INHERITED;
};

#endif
