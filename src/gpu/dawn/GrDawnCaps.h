/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnCaps_DEFINED
#define GrDawnCaps_DEFINED

#include "src/gpu/GrCaps.h"
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/dawn/GrDawnUtil.h"
#include "include/gpu/GrBackendSurface.h"

class GrDawnCaps : public GrCaps {
public:
    GrDawnCaps(const GrContextOptions& contextOptions);

    bool isFormatSRGB(const GrBackendFormat& format) const override;
    bool isFormatTexturable(GrColorType, const GrBackendFormat& format) const override;
    bool isFormatCopyable(GrColorType, const GrBackendFormat& format) const override;

    bool isConfigTexturable(GrPixelConfig config) const override;

    bool isConfigCopyable(GrPixelConfig config) const override {
        return true;
    }

    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override {
        return true;
    }

    GrPixelConfig onGetConfigFromBackendFormat(const GrBackendFormat&, GrColorType) const override;

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override {
        return SurfaceReadPixelsSupport::kSupported;
    }

    bool onSurfaceSupportsWritePixels(const GrSurface* surface) const override {
        return true;
    }

    int getRenderTargetSampleCount(int requestedCount, GrColorType,
                                   const GrBackendFormat&) const override;

    int getRenderTargetSampleCount(int requestedCount, GrPixelConfig config) const override {
        return this->isConfigTexturable(config) ? 1 : 0;
    }

    int maxRenderTargetSampleCount(GrColorType ct,
                                   const GrBackendFormat& format) const override {
        return this->maxRenderTargetSampleCount(this->getConfigFromBackendFormat(format, ct));
    }

    int maxRenderTargetSampleCount(GrPixelConfig config) const override {
        return this->isConfigTexturable(config) ? 1 : 0;
    }

    GrBackendFormat getBackendFormatFromColorType(GrColorType ct) const override;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    bool canClearTextureOnCreation() const override;

    GrSwizzle getTextureSwizzle(const GrBackendFormat&, GrColorType) const override;

    GrSwizzle getOutputSwizzle(const GrBackendFormat&, GrColorType) const override;

    size_t onTransferFromOffsetAlignment(GrColorType bufferColorType) const override;

    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    typedef GrCaps INHERITED;
};

#endif
