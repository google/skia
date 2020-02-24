/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DCaps_DEFINED
#define GrD3DCaps_DEFINED

#include "src/gpu/GrCaps.h"

class GrShaderCaps;

/**
 * Stores some capabilities of a D3D backend.
 */
class GrD3DCaps : public GrCaps {
public:
    /**
     * Creates a GrD3DCaps that is set such that nothing is supported. The init function should
     * be called to fill out the caps.
     */
    GrD3DCaps(const GrContextOptions& contextOptions, GrProtected isProtected = GrProtected::kNo);

    bool isFormatSRGB(const GrBackendFormat&) const override;
    SkImage::CompressionType compressionType(const GrBackendFormat&) const override;

    bool isFormatTexturableAndUploadable(GrColorType, const GrBackendFormat&) const override;
    bool isFormatTexturable(const GrBackendFormat&) const override;

    bool isFormatCopyable(const GrBackendFormat&) const override { return true; }

    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override;
    bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const override;

    int getRenderTargetSampleCount(int requestedCount, const GrBackendFormat&) const override;

    int maxRenderTargetSampleCount(const GrBackendFormat&) const override;

    size_t bytesPerPixel(const GrBackendFormat&) const override;

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override;

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override;

    GrColorType getYUVAColorTypeFromBackendFormat(const GrBackendFormat&,
                                                  bool isAlphaChannel) const override;

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    GrSwizzle getReadSwizzle(const GrBackendFormat&, GrColorType) const override;
    GrSwizzle getOutputSwizzle(const GrBackendFormat&, GrColorType) const override;

    uint64_t computeFormatKey(const GrBackendFormat&) const override;

    void addExtraSamplerKey(GrProcessorKeyBuilder*,
                            GrSamplerState,
                            const GrBackendFormat&) const override;

    GrProgramDesc makeDesc(const GrRenderTarget*, const GrProgramInfo&) const override;

#if GR_TEST_UTILS
    std::vector<TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif

private:
    void init(const GrContextOptions& contextOptions);

    bool onSurfaceSupportsWritePixels(const GrSurface*) const override;
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override;
    GrBackendFormat onGetDefaultBackendFormat(GrColorType, GrRenderable) const override;

    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    SupportedRead onSupportedReadPixelsColorType(GrColorType, const GrBackendFormat&,
                                                 GrColorType) const override;

    typedef GrCaps INHERITED;
};

#endif
