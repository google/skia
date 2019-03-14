/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlCaps.h"

#include "GrBackendSurface.h"
#include "GrMtlUtil.h"
#include "GrRenderTargetProxy.h"
#include "GrShaderCaps.h"
#include "GrSurfaceProxy.h"
#include "SkRect.h"

GrMtlCaps::GrMtlCaps(const GrContextOptions& contextOptions, const id<MTLDevice> device,
                     MTLFeatureSet featureSet)
        : INHERITED(contextOptions) {
    fShaderCaps.reset(new GrShaderCaps(contextOptions));

    this->initFeatureSet(featureSet);
    this->initGrCaps(device);
    this->initShaderCaps();
    this->initConfigTable();
    this->initStencilFormat(device);

    this->applyOptionsOverrides(contextOptions);
    fShaderCaps->applyOptionsOverrides(contextOptions);

    // The following are disabled due to the unfinished Metal backend, not because Metal itself
    // doesn't support it.
    fBlacklistCoverageCounting = true;   // CCPR shaders have some incompatabilities with SkSLC
    fFenceSyncSupport = false;           // Fences are not implemented yet
    fMipMapSupport = false;              // GrMtlGpu::onRegenerateMipMapLevels() not implemented
    fMultisampleDisableSupport = true;   // MSAA and resolving not implemented yet
    fDiscardRenderTargetSupport = false; // GrMtlGpuCommandBuffer::discard() not implemented
    fCrossContextTextureSupport = false; // GrMtlGpu::prepareTextureForCrossContextUsage() not impl
}

void GrMtlCaps::initFeatureSet(MTLFeatureSet featureSet) {
    // Mac OSX
#ifdef SK_BUILD_FOR_MAC
    if (MTLFeatureSet_OSX_GPUFamily1_v2 == featureSet) {
        fPlatform = Platform::kMac;
        fFamilyGroup = 1;
        fVersion = 2;
        return;
    }
    if (MTLFeatureSet_OSX_GPUFamily1_v1 == featureSet) {
        fPlatform = Platform::kMac;
        fFamilyGroup = 1;
        fVersion = 1;
        return;
    }
#endif

    // iOS Family group 3
#ifdef SK_BUILD_FOR_IOS
    if (MTLFeatureSet_iOS_GPUFamily3_v2 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 3;
        fVersion = 2;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily3_v1 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 3;
        fVersion = 1;
        return;
    }

    // iOS Family group 2
    if (MTLFeatureSet_iOS_GPUFamily2_v3 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 2;
        fVersion = 3;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily2_v2 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 2;
        fVersion = 2;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily2_v1 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 2;
        fVersion = 1;
        return;
    }

    // iOS Family group 1
    if (MTLFeatureSet_iOS_GPUFamily1_v3 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 1;
        fVersion = 3;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily1_v2 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 1;
        fVersion = 2;
        return;
    }
    if (MTLFeatureSet_iOS_GPUFamily1_v1 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 1;
        fVersion = 1;
        return;
    }
#endif
    // No supported feature sets were found
    SK_ABORT("Requested an unsupported feature set");
}

bool GrMtlCaps::canCopyAsBlit(GrPixelConfig dstConfig, int dstSampleCount,
                              GrSurfaceOrigin dstOrigin,
                              GrPixelConfig srcConfig, int srcSampleCount,
                              GrSurfaceOrigin srcOrigin,
                              const SkIRect& srcRect, const SkIPoint& dstPoint,
                              bool areDstSrcSameObj) const {
    if (dstConfig != srcConfig) {
        return false;
    }
    if ((dstSampleCount > 1 || srcSampleCount > 1) && (dstSampleCount != srcSampleCount)) {
        return false;
    }
    if (dstOrigin != srcOrigin) {
        return false;
    }
    if (areDstSrcSameObj) {
        SkIRect dstRect = SkIRect::MakeXYWH(dstPoint.x(), dstPoint.y(),
                                            srcRect.width(), srcRect.height());
        if (dstRect.intersect(srcRect)) {
            return false;
        }
    }
    return true;
}

bool GrMtlCaps::canCopyAsDraw(GrPixelConfig dstConfig, bool dstIsRenderable,
                              GrPixelConfig srcConfig, bool srcIsTextureable) const {
    // TODO: Make copySurfaceAsDraw handle the swizzle
    if (this->shaderCaps()->configOutputSwizzle(srcConfig) !=
        this->shaderCaps()->configOutputSwizzle(dstConfig)) {
        return false;
    }

    if (!dstIsRenderable || !srcIsTextureable) {
        return false;
    }
    return true;
}

bool GrMtlCaps::canCopyAsDrawThenBlit(GrPixelConfig dstConfig, GrPixelConfig srcConfig,
                                      bool srcIsTextureable) const {
    // TODO: Make copySurfaceAsDraw handle the swizzle
    if (this->shaderCaps()->configOutputSwizzle(srcConfig) !=
        this->shaderCaps()->configOutputSwizzle(dstConfig)) {
        return false;
    }
    if (!srcIsTextureable) {
        return false;
    }
    return true;
}

bool GrMtlCaps::onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    GrSurfaceOrigin dstOrigin = dst->origin();
    GrSurfaceOrigin srcOrigin = src->origin();

    int dstSampleCnt = 0;
    int srcSampleCnt = 0;
    if (const GrRenderTargetProxy* rtProxy = dst->asRenderTargetProxy()) {
        dstSampleCnt = rtProxy->numColorSamples();
    }
    if (const GrRenderTargetProxy* rtProxy = src->asRenderTargetProxy()) {
        srcSampleCnt = rtProxy->numColorSamples();
    }
    SkASSERT((dstSampleCnt > 0) == SkToBool(dst->asRenderTargetProxy()));
    SkASSERT((srcSampleCnt > 0) == SkToBool(src->asRenderTargetProxy()));

    return this->canCopyAsBlit(dst->config(), dstSampleCnt, dstOrigin,
                               src->config(), srcSampleCnt, srcOrigin,
                               srcRect, dstPoint, dst == src) ||
           this->canCopyAsDraw(dst->config(), SkToBool(dst->asRenderTargetProxy()),
                               src->config(), SkToBool(src->asTextureProxy())) ||
           this->canCopyAsDrawThenBlit(dst->config(), src->config(),
                                       SkToBool(src->asTextureProxy()));
}

void GrMtlCaps::initGrCaps(const id<MTLDevice> device) {
    // Max vertex attribs is the same on all devices
    fMaxVertexAttributes = 31;

    // Metal does not support scissor + clear
    fPerformPartialClearsAsDraws = true;

    // RenderTarget and Texture size
    if (this->isMac()) {
        fMaxRenderTargetSize = 16384;
    } else {
        if (3 == fFamilyGroup) {
            fMaxRenderTargetSize = 16384;
        } else {
            // Family group 1 and 2 support 8192 for version 2 and above, 4096 for v1
            if (1 == fVersion) {
                fMaxRenderTargetSize = 4096;
            } else {
                fMaxRenderTargetSize = 8192;
            }
        }
    }
    fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;
    fMaxTextureSize = fMaxRenderTargetSize;

    // Init sample counts. All devices support 1 (i.e. 0 in skia).
    fSampleCounts.push_back(1);
    for (auto sampleCnt : {2, 4, 8}) {
        if ([device supportsTextureSampleCount:sampleCnt]) {
            fSampleCounts.push_back(sampleCnt);
        }
    }

    // Clamp to border is supported on Mac 10.12 and higher (gpu family.version >= 1.2). It is not
    // supported on iOS.
    if (this->isMac()) {
        if (fFamilyGroup == 1 && fVersion < 2) {
            fClampToBorderSupport = false;
        }
    } else {
        fClampToBorderSupport = false;
    }

    // Starting with the assumption that there isn't a reason to not map small buffers.
    fBufferMapThreshold = 0;

    // Buffers are always fully mapped.
    fMapBufferFlags = kCanMap_MapFlag;

    fOversizedStencilSupport = true;

    fSRGBSupport = true;   // always available in Metal
    fSRGBWriteControl = false;
    fMipMapSupport = true;   // always available in Metal
    fNPOTTextureTileSupport = true;  // always available in Metal
    fDiscardRenderTargetSupport = true;

    fReuseScratchTextures = true; // Assuming this okay

    fTextureBarrierSupport = false; // Need to figure out if we can do this

    fSampleLocationsSupport = false;
    fMultisampleDisableSupport = false;

    if (this->isMac() || 3 == fFamilyGroup) {
        fInstanceAttribSupport = true;
    }

    fUsesMixedSamples = false;
    fGpuTracingSupport = false;

    fFenceSyncSupport = true;   // always available in Metal
    fCrossContextTextureSupport = false;
    fHalfFloatVertexAttributeSupport = true;
}


int GrMtlCaps::maxRenderTargetSampleCount(GrPixelConfig config) const {
    if (fConfigTable[config].fFlags & ConfigInfo::kMSAA_Flag) {
        return fSampleCounts[fSampleCounts.count() - 1];
    } else if (fConfigTable[config].fFlags & ConfigInfo::kRenderable_Flag) {
        return 1;
    }
    return 0;
}

int GrMtlCaps::getRenderTargetSampleCount(int requestedCount, GrPixelConfig config) const {
    requestedCount = SkTMax(requestedCount, 1);
    if (fConfigTable[config].fFlags & ConfigInfo::kMSAA_Flag) {
        int count = fSampleCounts.count();
        for (int i = 0; i < count; ++i) {
            if (fSampleCounts[i] >= requestedCount) {
                return fSampleCounts[i];
            }
        }
    } else if (fConfigTable[config].fFlags & ConfigInfo::kRenderable_Flag) {
        return 1 == requestedCount ? 1 : 0;
    }
    return 0;
}

void GrMtlCaps::initShaderCaps() {
    GrShaderCaps* shaderCaps = fShaderCaps.get();

    // fConfigOutputSwizzle will default to RGBA so we only need to set it for alpha only config.
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        if (GrPixelConfigIsAlphaOnly(config)) {
            shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RRRR();
            shaderCaps->fConfigOutputSwizzle[i] = GrSwizzle::AAAA();
        } else {
            if (kGray_8_GrPixelConfig == config) {
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RRRA();
            } else if (kRGB_888X_GrPixelConfig == config) {
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RGB1();
            } else {
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RGBA();
            }
        }
    }

    // Setting this true with the assumption that this cap will eventually mean we support varying
    // precisions and not just via modifiers.
    shaderCaps->fUsesPrecisionModifiers = true;
    shaderCaps->fFlatInterpolationSupport = true;
    // We haven't yet tested that using flat attributes perform well.
    shaderCaps->fPreferFlatInterpolation = true;

    shaderCaps->fShaderDerivativeSupport = true;
    shaderCaps->fGeometryShaderSupport = false;

    if ((this->isMac() && fVersion >= 2) ||
        (this->isIOS() && ((1 == fFamilyGroup && 4 == fVersion) ||
                           (2 == fFamilyGroup && 4 == fVersion) ||
                           (3 == fFamilyGroup && 3 == fVersion)))) {
        shaderCaps->fDualSourceBlendingSupport = true;
    }

    // TODO: Re-enable this once skbug:8720 is fixed. Will also need to remove asserts in
    // GrMtlPipelineStateBuilder which assert we aren't using this feature.
#if 0
    if (this->isIOS()) {
        shaderCaps->fFBFetchSupport = true;
        shaderCaps->fFBFetchNeedsCustomOutput = true; // ??
        shaderCaps->fFBFetchColorName = ""; // Somehow add [[color(0)]] to arguments to frag shader
    }
#endif
    shaderCaps->fDstReadInShaderSupport = shaderCaps->fFBFetchSupport;

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fVertexIDSupport = false;
    shaderCaps->fImageLoadStoreSupport = false;

    // Metal uses IEEE float and half floats so assuming those values here.
    shaderCaps->fFloatIs32Bits = true;
    shaderCaps->fHalfIs32Bits = false;

    // Metal supports unsigned integers.
    shaderCaps->fUnsignedSupport = true;

    shaderCaps->fMaxFragmentSamplers = 16;
}

void GrMtlCaps::initConfigTable() {
    ConfigInfo* info;
    // Alpha_8 uses R8Unorm
    info = &fConfigTable[kAlpha_8_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // Gray_8 uses R8Unorm
    info = &fConfigTable[kGray_8_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // RGB_565 uses B5G6R5Unorm, even though written opposite this format packs how we want
    info = &fConfigTable[kRGB_565_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = 0;
    } else {
        info->fFlags = ConfigInfo::kAllFlags;
    }

    // RGBA_4444 uses ABGR4Unorm
    info = &fConfigTable[kRGBA_4444_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = 0;
    } else {
        info->fFlags = ConfigInfo::kAllFlags;
    }

    // RGBA_8888 uses RGBA8Unorm
    info = &fConfigTable[kRGBA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // RGB_888X uses RGBA8Unorm and we will swizzle the 1
    info = &fConfigTable[kRGB_888X_GrPixelConfig];
    info->fFlags = ConfigInfo::kTextureable_Flag;

    // BGRA_8888 uses BGRA8Unorm
    info = &fConfigTable[kBGRA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // SRGBA_8888 uses RGBA8Unorm_sRGB
    info = &fConfigTable[kSRGBA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // SBGRA_8888 uses BGRA8Unorm_sRGB
    info = &fConfigTable[kSBGRA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // RGBA_float uses RGBA32Float
    info = &fConfigTable[kRGBA_float_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = ConfigInfo::kAllFlags;
    } else {
        info->fFlags = 0;
    }

    // RG_float uses RG32Float
    info = &fConfigTable[kRG_float_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = ConfigInfo::kAllFlags;
    } else {
        info->fFlags = ConfigInfo::kTextureable_Flag | ConfigInfo::kRenderable_Flag;
    }

    // Alpha_half uses R16Float
    info = &fConfigTable[kAlpha_half_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // RGBA_half uses RGBA16Float
    info = &fConfigTable[kRGBA_half_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    info = &fConfigTable[kRGBA_half_Clamped_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;
}

void GrMtlCaps::initStencilFormat(id<MTLDevice> physDev) {
    fPreferredStencilFormat = StencilFormat{ MTLPixelFormatStencil8, 8, 8, true };
}

GrPixelConfig validate_sized_format(GrMTLPixelFormat grFormat, SkColorType ct) {
    MTLPixelFormat format = static_cast<MTLPixelFormat>(grFormat);
    switch (ct) {
        case kUnknown_SkColorType:
            return kUnknown_GrPixelConfig;
        case kAlpha_8_SkColorType:
            if (MTLPixelFormatA8Unorm == format) {
                return kAlpha_8_as_Alpha_GrPixelConfig;
            } else if (MTLPixelFormatR8Unorm == format) {
                return kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
#ifdef SK_BUILD_FOR_MAC
        case kRGB_565_SkColorType:
        case kARGB_4444_SkColorType:
            return kUnknown_GrPixelConfig;
            break;
#else
        case kRGB_565_SkColorType:
            if (MTLPixelFormatB5G6R5Unorm == format) {
                return kRGB_565_GrPixelConfig;
            }
            break;
        case kARGB_4444_SkColorType:
            if (MTLPixelFormatABGR4Unorm == format) {
                return kRGBA_4444_GrPixelConfig;
            }
            break;
#endif
        case kRGBA_8888_SkColorType:
            if (MTLPixelFormatRGBA8Unorm == format) {
                return kRGBA_8888_GrPixelConfig;
            } else if (MTLPixelFormatRGBA8Unorm_sRGB == format) {
                return kSRGBA_8888_GrPixelConfig;
            }
            break;
        case kRGB_888x_SkColorType:
            if (MTLPixelFormatRGBA8Unorm == format) {
                return kRGB_888X_GrPixelConfig;
            }
            break;
        case kBGRA_8888_SkColorType:
            if (MTLPixelFormatBGRA8Unorm == format) {
                return kBGRA_8888_GrPixelConfig;
            } else if (MTLPixelFormatBGRA8Unorm_sRGB == format) {
                return kSBGRA_8888_GrPixelConfig;
            }
            break;
        case kRGBA_1010102_SkColorType:
            if (MTLPixelFormatRGB10A2Unorm == format) {
                return kRGBA_1010102_GrPixelConfig;
            }
            break;
        case kRGB_101010x_SkColorType:
            break;
        case kGray_8_SkColorType:
            if (MTLPixelFormatR8Unorm == format) {
                return kGray_8_as_Red_GrPixelConfig;
            }
            break;
        case kRGBA_F16Norm_SkColorType:
            if (MTLPixelFormatRGBA16Float == format) {
                return kRGBA_half_Clamped_GrPixelConfig;
            }
            break;
        case kRGBA_F16_SkColorType:
            if (MTLPixelFormatRGBA16Float == format) {
                return kRGBA_half_GrPixelConfig;
            }
            break;
        case kRGBA_F32_SkColorType:
            if (MTLPixelFormatR32Float == format) {
                return kRGBA_float_GrPixelConfig;
            }
            break;
    }

    return kUnknown_GrPixelConfig;
}

GrPixelConfig GrMtlCaps::validateBackendRenderTarget(const GrBackendRenderTarget& rt,
                                                     SkColorType ct) const {
    GrMtlTextureInfo fbInfo;
    if (!rt.getMtlTextureInfo(&fbInfo)) {
        return kUnknown_GrPixelConfig;
    }

    id<MTLTexture> texture = (__bridge id<MTLTexture>)fbInfo.fTexture;
    return validate_sized_format(texture.pixelFormat, ct);
}

GrPixelConfig GrMtlCaps::getConfigFromBackendFormat(const GrBackendFormat& format,
                                                    SkColorType ct) const {
    const GrMTLPixelFormat* mtlFormat = format.getMtlFormat();
    if (!mtlFormat) {
        return kUnknown_GrPixelConfig;
    }
    return validate_sized_format(*mtlFormat, ct);
}

static GrPixelConfig get_yuva_config(GrMTLPixelFormat grFormat) {
    MTLPixelFormat format = static_cast<MTLPixelFormat>(grFormat);

    switch (format) {
        case MTLPixelFormatA8Unorm:
            return kAlpha_8_as_Alpha_GrPixelConfig;
            break;
        case MTLPixelFormatR8Unorm:
            return kAlpha_8_as_Red_GrPixelConfig;
            break;
        // TODO: Add RG_88 format here
        case MTLPixelFormatRGBA8Unorm:
            return kRGBA_8888_GrPixelConfig;
            break;
        case MTLPixelFormatBGRA8Unorm:
            return kBGRA_8888_GrPixelConfig;
            break;
        default:
            return kUnknown_GrPixelConfig;
            break;
    }
}

GrPixelConfig GrMtlCaps::getYUVAConfigFromBackendFormat(const GrBackendFormat& format) const {
    const GrMTLPixelFormat* mtlFormat = format.getMtlFormat();
    if (!mtlFormat) {
        return kUnknown_GrPixelConfig;
    }
    return get_yuva_config(*mtlFormat);
}

GrBackendFormat GrMtlCaps::getBackendFormatFromGrColorType(GrColorType ct,
                                                           GrSRGBEncoded srgbEncoded) const {
    GrPixelConfig config = GrColorTypeToPixelConfig(ct, srgbEncoded);
    if (config == kUnknown_GrPixelConfig) {
        return GrBackendFormat();
    }
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(config, &format)) {
        return GrBackendFormat();
    }
    return GrBackendFormat::MakeMtl(format);
}

