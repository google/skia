/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnCaps_DEFINED
#define GrDawnCaps_DEFINED

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/dawn/GrDawnUtil.h"

class GrDawnCaps : public GrCaps {
public:
    GrDawnCaps(const GrContextOptions& contextOptions);

    bool isFormatSRGB(const GrBackendFormat&) const override;
    bool isFormatCompressed(const GrBackendFormat&) const override;

    bool isFormatTexturable(GrColorType, const GrBackendFormat& format) const override;
    bool isFormatRenderable(GrColorType, const GrBackendFormat& format,
                            int sampleCount = 1) const override;
    bool isFormatCopyable(const GrBackendFormat& format) const override { return true; }

    bool isConfigTexturable(GrPixelConfig config) const override;

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override {
        return {surfaceColorType, GrColorTypeBytesPerPixel(surfaceColorType)};
    }

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override {
        return SurfaceReadPixelsSupport::kSupported;
    }

    int getRenderTargetSampleCount(int requestedCount, GrColorType,
                                   const GrBackendFormat&) const override;
    int getRenderTargetSampleCount(int requestedCount, GrPixelConfig) const override;

    int maxRenderTargetSampleCount(const GrBackendFormat& format) const override;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    bool canClearTextureOnCreation() const override;

    GrSwizzle getTextureSwizzle(const GrBackendFormat&, GrColorType) const override;

    GrSwizzle getOutputSwizzle(const GrBackendFormat&, GrColorType) const override;

    GrColorType getYUVAColorTypeFromBackendFormat(const GrBackendFormat&) const override;

#if GR_TEST_UTILS
    std::vector<TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif

private:
    bool onSurfaceSupportsWritePixels(const GrSurface* surface) const override {
        return true;
    }
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
        const SkIRect& srcRect, const SkIPoint& dstPoint) const override {
        return true;
    }
    GrBackendFormat onGetDefaultBackendFormat(GrColorType, GrRenderable) const override;

    GrPixelConfig onGetConfigFromBackendFormat(const GrBackendFormat&, GrColorType) const override;

    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    SupportedRead onSupportedReadPixelsColorType(GrColorType srcColorType,
                                                 const GrBackendFormat& backendFormat,
                                                 GrColorType dstColorType) const override {
        return { GrColorType::kUnknown, 0 };
    }

    typedef GrCaps INHERITED;
};

#endif
