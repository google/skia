/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMockCaps_DEFINED
#define GrMockCaps_DEFINED

#include "include/gpu/mock/GrMockTypes.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/SkGr.h"

class GrMockCaps : public GrCaps {
public:
    GrMockCaps(const GrContextOptions& contextOptions, const GrMockOptions& options)
            : INHERITED(contextOptions), fOptions(options) {
        fInstanceAttribSupport = options.fInstanceAttribSupport;
        fHalfFloatVertexAttributeSupport = options.fHalfFloatVertexAttributeSupport;
        fMapBufferFlags = options.fMapBufferFlags;
        fBufferMapThreshold = SK_MaxS32; // Overridable in GrContextOptions.
        fMaxTextureSize = options.fMaxTextureSize;
        fMaxRenderTargetSize = SkTMin(options.fMaxRenderTargetSize, fMaxTextureSize);
        fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;
        fMaxVertexAttributes = options.fMaxVertexAttributes;

        fShaderCaps.reset(new GrShaderCaps(contextOptions));
        fShaderCaps->fGeometryShaderSupport = options.fGeometryShaderSupport;
        fShaderCaps->fIntegerSupport = options.fIntegerSupport;
        fShaderCaps->fFlatInterpolationSupport = options.fFlatInterpolationSupport;
        fShaderCaps->fMaxFragmentSamplers = options.fMaxFragmentSamplers;
        fShaderCaps->fShaderDerivativeSupport = options.fShaderDerivativeSupport;
        fShaderCaps->fDualSourceBlendingSupport = options.fDualSourceBlendingSupport;

        this->applyOptionsOverrides(contextOptions);
    }

    bool isFormatSRGB(const GrBackendFormat& format) const override {
        if (!format.getMockSRGBEncoded()) {
            return false;
        }

        return GrSRGBEncoded::kYes == *format.getMockSRGBEncoded();
    }

    bool isFormatTexturable(SkColorType, const GrBackendFormat& format) const override {
        if (!format.getMockColorType()) {
            return false;
        }

        // We are ignoring the srgb encoding here
        return fOptions.fConfigOptions[(int)*format.getMockColorType()].fTexturable;
    }

    bool isConfigTexturable(GrPixelConfig config) const override {
        GrColorType ct = GrPixelConfigToColorType(config);

        return fOptions.fConfigOptions[(int)ct].fTexturable;
    }

    bool isFormatCopyable(SkColorType, const GrBackendFormat& format) const override {
        if (!format.getMockColorType() || !format.getMockSRGBEncoded()) {
            return false;
        }

        GrPixelConfig config = GrColorTypeToPixelConfig(*format.getMockColorType(),
                                                        *format.getMockSRGBEncoded());

        return this->isConfigCopyable(config);
    }

    bool isConfigCopyable(GrPixelConfig config) const override {
        return false;
    }

    int getRenderTargetSampleCount(int requestCount, GrColorType ct) const {
        requestCount = SkTMax(requestCount, 1);

        switch (fOptions.fConfigOptions[(int)ct].fRenderability) {
            case GrMockOptions::ConfigOptions::Renderability::kNo:
                return 0;
            case GrMockOptions::ConfigOptions::Renderability::kNonMSAA:
                return requestCount > 1 ? 0 : 1;
            case GrMockOptions::ConfigOptions::Renderability::kMSAA:
                return requestCount > kMaxSampleCnt ? 0 : GrNextPow2(requestCount);
        }
        return 0;
    }

    int getRenderTargetSampleCount(int requestCount,
                                   SkColorType, const GrBackendFormat& format) const override {
        if (!format.getMockColorType()) {
            return 0;
        }

        return this->getRenderTargetSampleCount(requestCount, *format.getMockColorType());
    }

    int getRenderTargetSampleCount(int requestCount, GrPixelConfig config) const override {
        GrColorType ct = GrPixelConfigToColorType(config);

        return this->getRenderTargetSampleCount(requestCount, ct);
    }

    int maxRenderTargetSampleCount(GrColorType ct) const {
        switch (fOptions.fConfigOptions[(int)ct].fRenderability) {
            case GrMockOptions::ConfigOptions::Renderability::kNo:
                return 0;
            case GrMockOptions::ConfigOptions::Renderability::kNonMSAA:
                return 1;
            case GrMockOptions::ConfigOptions::Renderability::kMSAA:
                return kMaxSampleCnt;
        }
        return 0;
    }

    int maxRenderTargetSampleCount(SkColorType, const GrBackendFormat& format) const override {
        if (!format.getMockColorType()) {
            return 0;
        }

        return this->maxRenderTargetSampleCount(*format.getMockColorType());
    }

    int maxRenderTargetSampleCount(GrPixelConfig config) const override {
        GrColorType ct = GrPixelConfigToColorType(config);

        return this->maxRenderTargetSampleCount(ct);
    }

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override {
        return SurfaceReadPixelsSupport::kSupported;
    }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

    GrPixelConfig validateBackendRenderTarget(const GrBackendRenderTarget&,
                                              GrColorType) const override {
        return kUnknown_GrPixelConfig;
    }

    bool areColorTypeAndFormatCompatible(GrColorType ct,
                                         const GrBackendFormat& format) const override {
        const GrColorType* mockColorType = format.getMockColorType();
        if (!mockColorType) {
            return false;
        }

        if (GrColorType::kUnknown == ct) {
            return false;
        }

        return ct == *mockColorType;

#if 0
        switch (ct) {
            case GrColorType::kUnknown:
                return false;
            case GrColorType::kAlpha_8:
                if (kAlpha_8_GrPixelConfig == *mockFormat ||
                    kAlpha_8_as_Alpha_GrPixelConfig == *mockFormat ||
                    kAlpha_8_as_Red_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kBGR_565:
                if (kRGB_565_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kABGR_4444:
                if (kRGBA_4444_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRGBA_8888:
                if (kRGBA_8888_GrPixelConfig == *mockFormat ||
                    kSRGBA_8888_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRGB_888x:
                if (kRGB_888X_GrPixelConfig == *mockFormat ||
                    kRGB_888_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRG_88:
                if (kRG_88_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kBGRA_8888:
                if (kBGRA_8888_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRGBA_1010102:
                if (kRGBA_1010102_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kGray_8:
                if (kGray_8_GrPixelConfig == *mockFormat ||
                    kGray_8_as_Lum_GrPixelConfig == *mockFormat ||
                    kGray_8_as_Red_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kAlpha_F16:
                if (kAlpha_half_GrPixelConfig == *mockFormat) {
                    return true;
                }
            case GrColorType::kRGBA_F16:
                if (kRGBA_half_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRGBA_F16_Clamped:
                if (kRGBA_half_Clamped_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRG_F32:
                if (kRG_float_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRGBA_F32:
                if (kRGBA_float_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kR_16:
                if (kR_16_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRG_1616:
                if (kRG_1616_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRGBA_16161616:
                if (kRGBA_16161616_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
            case GrColorType::kRG_F16:
                if (kRG_half_GrPixelConfig == *mockFormat) {
                    return true;
                }
                break;
        }

        return false;
#endif
    }

    GrPixelConfig getConfigFromBackendFormat(const GrBackendFormat& format,
                                             GrColorType) const override {
        if (!format.getMockColorType() || !format.getMockSRGBEncoded()) {
            return kUnknown_GrPixelConfig;
        }

        GrPixelConfig config = GrColorTypeToPixelConfig(*format.getMockColorType(),
                                                        *format.getMockSRGBEncoded());

        return config;
    }

    GrPixelConfig getYUVAConfigFromBackendFormat(const GrBackendFormat& format) const override {
        if (!format.getMockColorType() || !format.getMockSRGBEncoded()) {
            return kUnknown_GrPixelConfig;
        }

        GrPixelConfig config = GrColorTypeToPixelConfig(*format.getMockColorType(),
                                                        *format.getMockSRGBEncoded());

        return config;
    }

    GrBackendFormat getBackendFormatFromGrColorType(GrColorType ct,
                                                    GrSRGBEncoded srgbEncoded) const override {
        return GrBackendFormat::MakeMock(ct, srgbEncoded);
    }

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override {
        return {};
    }

    GrSwizzle getTextureSwizzle(const GrBackendFormat&, GrColorType) const override {
        return GrSwizzle();
    }
    GrSwizzle getOutputSwizzle(const GrBackendFormat&, GrColorType) const override {
        return GrSwizzle();
    }

private:
    bool onSurfaceSupportsWritePixels(const GrSurface*) const override { return true; }
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override {
        return true;
    }
    size_t onTransferFromOffsetAlignment(GrColorType bufferColorType) const override {
        // arbitrary
        return GrSizeAlignUp(GrColorTypeBytesPerPixel(bufferColorType), 4);
    }

    static const int kMaxSampleCnt = 16;

    GrMockOptions fOptions;
    typedef GrCaps INHERITED;
};

#endif
