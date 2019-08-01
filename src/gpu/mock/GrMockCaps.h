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
        fSampleLocationsSupport = true;

        fShaderCaps.reset(new GrShaderCaps(contextOptions));
        fShaderCaps->fGeometryShaderSupport = options.fGeometryShaderSupport;
        fShaderCaps->fIntegerSupport = options.fIntegerSupport;
        fShaderCaps->fFlatInterpolationSupport = options.fFlatInterpolationSupport;
        fShaderCaps->fMaxFragmentSamplers = options.fMaxFragmentSamplers;
        fShaderCaps->fShaderDerivativeSupport = options.fShaderDerivativeSupport;
        fShaderCaps->fDualSourceBlendingSupport = options.fDualSourceBlendingSupport;
        fShaderCaps->fSampleVariablesSupport = true;
        fShaderCaps->fSampleVariablesStencilSupport = true;

        this->applyOptionsOverrides(contextOptions);
    }

    bool isFormatSRGB(const GrBackendFormat& format) const override {
        if (!format.getMockColorType()) {
            return false;
        }
        return *format.getMockColorType() == GrColorType::kRGBA_8888_SRGB;
    }

    // Mock caps doesn't support any compressed formats right now
    bool isFormatCompressed(const GrBackendFormat&) const override {
        return false;
    }

    bool isFormatTexturable(GrColorType, const GrBackendFormat& format) const override {
        if (!format.getMockColorType()) {
            return false;
        }
        return fOptions.fConfigOptions[(int)*format.getMockColorType()].fTexturable;
    }

    bool isConfigTexturable(GrPixelConfig config) const override {
        GrColorType ct = GrPixelConfigToColorType(config);
        return fOptions.fConfigOptions[(int)ct].fTexturable;
    }

    bool isFormatCopyable(const GrBackendFormat& format) const override {
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
                                   GrColorType, const GrBackendFormat& format) const override {
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

    int maxRenderTargetSampleCount(GrColorType, const GrBackendFormat& format) const override {
        if (!format.getMockColorType()) {
            return 0;
        }
        return this->maxRenderTargetSampleCount(*format.getMockColorType());
    }

    int maxRenderTargetSampleCount(GrPixelConfig config) const override {
        GrColorType ct = GrPixelConfigToColorType(config);
        return this->maxRenderTargetSampleCount(ct);
    }

    SupportedWrite supportedWritePixelsColorType(GrPixelConfig config,
                                                 GrColorType srcColorType) const override {
        return {GrPixelConfigToColorType(config), 1};
    }

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override {
        return SurfaceReadPixelsSupport::kSupported;
    }

    GrColorType getYUVAColorTypeFromBackendFormat(const GrBackendFormat& format) const override {
        if (!format.getMockColorType()) {
            return GrColorType::kUnknown;
        }

        return *format.getMockColorType();
    }

    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override {
        return {};
    }

    bool canClearTextureOnCreation() const override { return true; }

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
    GrBackendFormat onGetDefaultBackendFormat(GrColorType ct, GrRenderable) const override {
        return GrBackendFormat::MakeMock(ct);
    }

    GrPixelConfig onGetConfigFromBackendFormat(const GrBackendFormat& format,
                                               GrColorType) const override {
        if (!format.getMockColorType()) {
            return kUnknown_GrPixelConfig;
        }

        return GrColorTypeToPixelConfig(*format.getMockColorType());

    }

    bool onAreColorTypeAndFormatCompatible(GrColorType ct,
                                           const GrBackendFormat& format) const override {
        if (GrColorType::kUnknown == ct) {
            return false;
        }

        const GrColorType* mockColorType = format.getMockColorType();
        if (!mockColorType) {
            return false;
        }

        return ct == *mockColorType;
    }

    SupportedRead onSupportedReadPixelsColorType(GrColorType srcColorType, const GrBackendFormat&,
                                                 GrColorType) const override {
        return SupportedRead{srcColorType, 1};
    }

    static const int kMaxSampleCnt = 16;

    GrMockOptions fOptions;
    typedef GrCaps INHERITED;
};

#endif
