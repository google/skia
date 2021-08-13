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

    bool isFormatRenderable(const GrBackendFormat& format,
                            int sampleCount = 1) const override;
    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override;


    bool isFormatCopyable(const GrBackendFormat& format) const override { return true; }

    bool isFormatTexturable(const GrBackendFormat& format, GrTextureType) const override;

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override {
        return {surfaceColorType, GrColorTypeBytesPerPixel(surfaceColorType)};
    }

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override;

    int getRenderTargetSampleCount(int requestedCount,
                                   const GrBackendFormat&) const override;

    int maxRenderTargetSampleCount(const GrBackendFormat& format) const override;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    GrSwizzle getWriteSwizzle(const GrBackendFormat&, GrColorType) const override;

    uint64_t computeFormatKey(const GrBackendFormat&) const override;

    GrProgramDesc makeDesc(GrRenderTarget*,
                           const GrProgramInfo&,
                           ProgramDescOverrideFlags) const override;

#if GR_TEST_UTILS
    std::vector<TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif

private:
    bool onSurfaceSupportsWritePixels(const GrSurface* surface) const override;
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
        const SkIRect& srcRect, const SkIPoint& dstPoint) const override {
        return true;
    }
    GrBackendFormat onGetDefaultBackendFormat(GrColorType) const override;

    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    SupportedRead onSupportedReadPixelsColorType(GrColorType srcColorType,
                                                 const GrBackendFormat& backendFormat,
                                                 GrColorType dstColorType) const override {
        return { srcColorType, GrColorTypeBytesPerPixel(srcColorType) };
    }

    GrSwizzle onGetReadSwizzle(const GrBackendFormat&, GrColorType) const override;

    using INHERITED = GrCaps;
};

#endif
