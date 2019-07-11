/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlCaps_DEFINED
#define GrMtlCaps_DEFINED

#include "include/private/SkTDArray.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/mtl/GrMtlStencilAttachment.h"

#import <Metal/Metal.h>

class GrShaderCaps;

/**
 * Stores some capabilities of a Mtl backend.
 */
class GrMtlCaps : public GrCaps {
public:
    typedef GrMtlStencilAttachment::Format StencilFormat;

    GrMtlCaps(const GrContextOptions& contextOptions, id<MTLDevice> device,
              MTLFeatureSet featureSet);

    bool isFormatSRGB(const GrBackendFormat& format) const override;

    bool isFormatTexturable(GrColorType, const GrBackendFormat&) const override;

    bool isConfigTexturable(GrPixelConfig config) const override {
        return SkToBool(fConfigTable[config].fFlags & ConfigInfo::kTextureable_Flag);
    }

    int getRenderTargetSampleCount(int requestedCount,
                                   GrColorType, const GrBackendFormat&) const override;
    int getRenderTargetSampleCount(int requestedCount, GrPixelConfig) const override;

    int maxRenderTargetSampleCount(GrColorType, const GrBackendFormat&) const override;
    int maxRenderTargetSampleCount(GrPixelConfig) const override;

    SurfaceReadPixelsSupport surfaceSupportsReadPixels(const GrSurface*) const override {
        return SurfaceReadPixelsSupport::kSupported;
    }

    bool isFormatCopyable(GrColorType, const GrBackendFormat&) const override { return true; }
    bool isConfigCopyable(GrPixelConfig) const override { return true; }

    /**
     * Returns both a supported and most prefered stencil format to use in draws.
     */
    const StencilFormat& preferredStencilFormat() const {
        return fPreferredStencilFormat;
    }

    bool canCopyAsBlit(GrPixelConfig dstConfig, int dstSampleCount, GrPixelConfig srcConfig,
                       int srcSampleCount, const SkIRect& srcRect, const SkIPoint& dstPoint,
                       bool areDstSrcSameObj) const;

    bool canCopyAsResolve(GrSurface* dst, int dstSampleCount, GrSurface* src, int srcSampleCount,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const;

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

    GrPixelConfig validateBackendRenderTarget(const GrBackendRenderTarget&,
                                              GrColorType) const override;

    GrPixelConfig getYUVAConfigFromBackendFormat(const GrBackendFormat&) const override;

    GrBackendFormat getBackendFormatFromColorType(GrColorType ct) const override;
    GrBackendFormat getBackendFormatFromCompressionType(SkImage::CompressionType) const override;

    GrSwizzle getTextureSwizzle(const GrBackendFormat&, GrColorType) const override;
    GrSwizzle getOutputSwizzle(const GrBackendFormat&, GrColorType) const override;

private:
    void initFeatureSet(MTLFeatureSet featureSet);

    void initStencilFormat(const id<MTLDevice> device);

    void initGrCaps(const id<MTLDevice> device);
    void initShaderCaps();

    void initConfigTable();

    bool onSurfaceSupportsWritePixels(const GrSurface*) const override;
    bool onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                          const SkIRect& srcRect, const SkIPoint& dstPoint) const override;
    size_t onTransferFromOffsetAlignment(GrColorType bufferColorType) const override {
        // Transfer buffers not yet supported.
        return 0;
    }
    GrPixelConfig onGetConfigFromBackendFormat(const GrBackendFormat&, GrColorType) const override;
    bool onAreColorTypeAndFormatCompatible(GrColorType, const GrBackendFormat&) const override;

    struct ConfigInfo {
        ConfigInfo() : fFlags(0) {}

        enum {
            kTextureable_Flag = 0x1,
            kRenderable_Flag  = 0x2, // Color attachment and blendable
            kMSAA_Flag        = 0x4,
            kResolve_Flag     = 0x8,
        };
        static const uint16_t kAllFlags = kTextureable_Flag | kRenderable_Flag |
                                          kMSAA_Flag | kResolve_Flag;

        uint16_t fFlags;
    };
    ConfigInfo fConfigTable[kGrPixelConfigCnt];

    enum class Platform {
        kMac,
        kIOS
    };
    bool isMac() { return Platform::kMac == fPlatform; }
    bool isIOS() { return Platform::kIOS == fPlatform; }

    Platform fPlatform;
    int fFamilyGroup;
    int fVersion;

    SkTDArray<int> fSampleCounts;

    StencilFormat fPreferredStencilFormat;

    typedef GrCaps INHERITED;
};

#endif
