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
        fMipMapSupport = options.fMipMapSupport;
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
        fShaderCaps->fSampleMaskSupport = true;

        this->finishInitialization(contextOptions);
    }

    bool isFormatSRGB(const GrBackendFormat& format) const override {
        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression != SkImage::CompressionType::kNone) {
            return false;
        }

        auto ct = format.asMockColorType();
        return GrGetColorTypeDesc(ct).encoding() == GrColorTypeEncoding::kSRGBUnorm;
    }

    bool isFormatCompressed(const GrBackendFormat& format,
                            SkImage::CompressionType* compressionType = nullptr) const override {
        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression == SkImage::CompressionType::kNone) {
            return false;
        }

        if (compressionType) {
            *compressionType = compression;
        }
        return true;
    }

    bool isFormatTexturableAndUploadable(GrColorType,
                                         const GrBackendFormat& format) const override {
        return this->isFormatTexturable(format);
    }
    bool isFormatTexturable(const GrBackendFormat& format) const override {
        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression != SkImage::CompressionType::kNone) {
            return fOptions.fCompressedOptions[(int)compression].fTexturable;
        }

        auto index = static_cast<int>(format.asMockColorType());
        return fOptions.fConfigOptions[index].fTexturable;
    }

    bool isFormatCopyable(const GrBackendFormat& format) const override {
        return false;
    }

    bool isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                       int sampleCount = 1) const override {
        // Currently we don't allow RGB_888X to be renderable because we don't have a way to
        // handle blends that reference dst alpha when the values in the dst alpha channel are
        // uninitialized.
        if (ct == GrColorType::kRGB_888x) {
            return false;
        }
        return this->isFormatRenderable(format, sampleCount);
    }

    bool isFormatRenderable(const GrBackendFormat& format, int sampleCount) const override {
        if (format.asMockCompressionType() != SkImage::CompressionType::kNone) {
            return false;  // compressed formats are never renderable
        }

        return sampleCount <= this->maxRenderTargetSampleCount(format.asMockColorType());
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
                                   const GrBackendFormat& format) const override {
        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression != SkImage::CompressionType::kNone) {
            return 0; // no compressed format is renderable
        }

        return this->getRenderTargetSampleCount(requestCount, format.asMockColorType());
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

    int maxRenderTargetSampleCount(const GrBackendFormat& format) const override {
        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression != SkImage::CompressionType::kNone) {
            return 0; // no compressed format is renderable
        }

        return this->maxRenderTargetSampleCount(format.asMockColorType());
    }

    size_t bytesPerPixel(const GrBackendFormat& format) const override {
        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression != SkImage::CompressionType::kNone) {
            return 0;
        }

        return GrColorTypeBytesPerPixel(format.asMockColorType());
    }

    SupportedWrite supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                 const GrBackendFormat& surfaceFormat,
                                                 GrColorType srcColorType) const override {
        return {surfaceColorType, 1};
    }

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override {
        return SurfaceReadPixelsSupport::kSupported;
    }

    GrColorType getYUVAColorTypeFromBackendFormat(const GrBackendFormat& format,
                                                  bool isAlphaChannel) const override {
        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression != SkImage::CompressionType::kNone) {
            return GrColorType::kUnknown;
        }

        return format.asMockColorType();
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

    GrProgramDesc makeDesc(const GrRenderTarget*, const GrProgramInfo&) const override;

#if GR_TEST_UTILS
    std::vector<GrCaps::TestFormatColorTypeCombination> getTestingCombinations() const override;
#endif

private:
    bool onSurfaceSupportsWritePixels(const GrSurface*) const override { return true; }
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override {
        return true;
    }
    GrBackendFormat onGetDefaultBackendFormat(GrColorType ct, GrRenderable) const override {
        return GrBackendFormat::MakeMock(ct, SkImage::CompressionType::kNone);
    }

    GrPixelConfig onGetConfigFromBackendFormat(const GrBackendFormat& format,
                                               GrColorType) const override {
        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression != SkImage::CompressionType::kNone) {
            return GrCompressionTypeToPixelConfig(compression);
        }

        return GrColorTypeToPixelConfig(format.asMockColorType());
    }

    bool onAreColorTypeAndFormatCompatible(GrColorType ct,
                                           const GrBackendFormat& format) const override {
        if (ct == GrColorType::kUnknown) {
            return false;
        }

        SkImage::CompressionType compression = format.asMockCompressionType();
        if (compression == SkImage::CompressionType::kETC1) {
            return ct == GrColorType::kRGB_888x; // TODO: this may be too restrictive
        }

        return ct == format.asMockColorType();
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
