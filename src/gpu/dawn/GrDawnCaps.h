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
#include "GrDawnUtil.h"
#include "GrBackendSurface.h"

class GrDawnCaps : public GrCaps {
public:
    GrDawnCaps(const GrContextOptions& contextOptions);

    bool isConfigTexturable(GrPixelConfig config) const override;

    bool isConfigCopyable(GrPixelConfig config) const override {
        return true;
    }

    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override {
        return true;
    }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            GrSurfaceOrigin* origi, bool* rectsMustMatch,
                            bool* disallowSubrect) const override {
        return false;
    }

    GrPixelConfig validateBackendRenderTarget(const GrBackendRenderTarget&, SkColorType) const override {
        return GrPixelConfig::kUnknown_GrPixelConfig;
    }

    GrPixelConfig getConfigFromBackendFormat(const GrBackendFormat&, SkColorType) const override;

    GrPixelConfig getYUVAConfigFromBackendFormat(const GrBackendFormat&) const override;

    GrBackendFormat getBackendFormatFromGrColorType(GrColorType ct, GrSRGBEncoded srgbEncoded) const override {
        GrPixelConfig config = GrColorTypeToPixelConfig(ct, srgbEncoded);
        dawn::TextureFormat format;
        if (config == kUnknown_GrPixelConfig || !GrPixelConfigToDawnFormat(config, &format)) {
            return GrBackendFormat();
        }
        return GrBackendFormat::MakeDawn(format);
    }

    bool surfaceSupportsReadPixels(const GrSurface*) const override {
        return true;
    }

    bool onSurfaceSupportsWritePixels(const GrSurface* surface) const override {
        return true;
    }

    int getRenderTargetSampleCount(int requestedCount, GrPixelConfig config) const override {
        return this->isConfigTexturable(config) ? 1 : 0;
    }

    int maxRenderTargetSampleCount(GrPixelConfig config) const override {
        return this->isConfigTexturable(config) ? 1 : 0;
    }

    GrSurfaceOrigin renderTargetOrigin() const override {
        return kTopLeft_GrSurfaceOrigin;
    }

    typedef GrCaps INHERITED;
};

#endif
