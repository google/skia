/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlCaps.h"

#include "include/core/SkRect.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/mtl/GrMtlUtil.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

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
    fFenceSyncSupport = false;           // Fences are not implemented yet
    fSemaphoreSupport = false;           // Semaphores are not implemented yet
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
                              GrPixelConfig srcConfig, int srcSampleCount,
                              const SkIRect& srcRect, const SkIPoint& dstPoint,
                              bool areDstSrcSameObj) const {
    if (dstConfig != srcConfig) {
        return false;
    }
    if ((dstSampleCount > 1 || srcSampleCount > 1) && (dstSampleCount != srcSampleCount)) {
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

bool GrMtlCaps::canCopyAsResolve(GrSurface* dst, int dstSampleCount,
                                 GrSurface* src, int srcSampleCount,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    if (dst == src) {
        return false;
    }
    if (dst->backendFormat() != src->backendFormat()) {
        return false;
    }
    if (dstSampleCount > 1 || srcSampleCount == 1 || !src->asRenderTarget()) {
        return false;
    }

    // TODO: Support copying subrectangles
    if (dstPoint != SkIPoint::Make(0, 0)) {
        return false;
    }
    if (srcRect != SkIRect::MakeXYWH(0, 0, src->width(), src->height())) {
        return false;
    }

    return true;
}

bool GrMtlCaps::onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                 const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    int dstSampleCnt = 0;
    int srcSampleCnt = 0;
    if (const GrRenderTargetProxy* rtProxy = dst->asRenderTargetProxy()) {
        dstSampleCnt = rtProxy->numSamples();
    }
    if (const GrRenderTargetProxy* rtProxy = src->asRenderTargetProxy()) {
        srcSampleCnt = rtProxy->numSamples();
    }
    SkASSERT((dstSampleCnt > 0) == SkToBool(dst->asRenderTargetProxy()));
    SkASSERT((srcSampleCnt > 0) == SkToBool(src->asRenderTargetProxy()));

    return this->canCopyAsBlit(dst->config(), dstSampleCnt, src->config(), srcSampleCnt, srcRect,
                               dstPoint, dst == src);
}

void GrMtlCaps::initGrCaps(const id<MTLDevice> device) {
    // Max vertex attribs is the same on all devices
    fMaxVertexAttributes = 31;

    // Metal does not support scissor + clear
    fPerformPartialClearsAsDraws = true;

    // We always copy in/out of a transfer buffer so it's trivial to support row bytes.
    fReadPixelsRowBytesSupport = true;
    fWritePixelsRowBytesSupport = true;

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

    fReuseScratchTextures = true; // Assuming this okay

    fTextureBarrierSupport = false; // Need to figure out if we can do this

    fSampleLocationsSupport = false;
    fMultisampleDisableSupport = false;

    if (this->isMac() || 3 == fFamilyGroup) {
        fInstanceAttribSupport = true;
    }

    fMixedSamplesSupport = false;
    fGpuTracingSupport = false;

    fFenceSyncSupport = true;   // always available in Metal
    fCrossContextTextureSupport = false;
    fHalfFloatVertexAttributeSupport = true;
}

static bool format_is_srgb(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatBGRA8Unorm_sRGB:
            return true;
        default:
            return false;
    }
}

bool GrMtlCaps::isFormatSRGB(const GrBackendFormat& format) const {
    if (!format.getMtlFormat()) {
        return false;
    }

    return format_is_srgb(static_cast<MTLPixelFormat>(*format.getMtlFormat()));
}

bool GrMtlCaps::isFormatTexturable(GrColorType grCT, const GrBackendFormat& format) const {
    if (GrColorType::kUnknown == grCT) {
        return false;
    }

    GrPixelConfig config = this->getConfigFromBackendFormat(format, grCT);
    if (kUnknown_GrPixelConfig == config) {
        return false;
    }

    return this->isConfigTexturable(config);
}

int GrMtlCaps::maxRenderTargetSampleCount(GrColorType grCT, const GrBackendFormat& format) const {
    if (GrColorType::kUnknown == grCT) {
        return 0;
    }

    GrPixelConfig config = this->getConfigFromBackendFormat(format, grCT);
    if (kUnknown_GrPixelConfig == config) {
        return 0;
    }

    return this->maxRenderTargetSampleCount(config);
}

int GrMtlCaps::maxRenderTargetSampleCount(GrPixelConfig config) const {
    if (fConfigTable[config].fFlags & ConfigInfo::kMSAA_Flag) {
        return fSampleCounts[fSampleCounts.count() - 1];
    } else if (fConfigTable[config].fFlags & ConfigInfo::kRenderable_Flag) {
        return 1;
    }
    return 0;
}

int GrMtlCaps::getRenderTargetSampleCount(int requestedCount,
                                          GrColorType grCT, const GrBackendFormat& format) const {
    if (GrColorType::kUnknown == grCT) {
        return 0;
    }

    GrPixelConfig config = this->getConfigFromBackendFormat(format, grCT);
    if (kUnknown_GrPixelConfig == config) {
        return 0;
    }

    return this->getRenderTargetSampleCount(requestedCount, config);
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

    // Metal uses IEEE float and half floats so assuming those values here.
    shaderCaps->fFloatIs32Bits = true;
    shaderCaps->fHalfIs32Bits = false;

    shaderCaps->fMaxFragmentSamplers = 16;
}

void GrMtlCaps::initConfigTable() {
    ConfigInfo* info;
    // Alpha_8 uses R8Unorm
    info = &fConfigTable[kAlpha_8_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // Alpha_8_as_Red uses R8Unorm
    info = &fConfigTable[kAlpha_8_as_Red_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // Gray_8 uses R8Unorm
    info = &fConfigTable[kGray_8_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // Gray_8_as_Red uses R8Unorm
    info = &fConfigTable[kGray_8_as_Red_GrPixelConfig];
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

    // RGB_888 uses RGBA8Unorm and we will swizzle the 1
    info = &fConfigTable[kRGB_888_GrPixelConfig];
    info->fFlags = ConfigInfo::kTextureable_Flag;

    // RG_88 uses RG8Unorm
    info = &fConfigTable[kRG_88_GrPixelConfig];
    info->fFlags = ConfigInfo::kTextureable_Flag;

    // BGRA_8888 uses BGRA8Unorm
    info = &fConfigTable[kBGRA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // SRGBA_8888 uses RGBA8Unorm_sRGB
    info = &fConfigTable[kSRGBA_8888_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;

    // kRGBA_1010102 uses RGB10A2Unorm
    info = &fConfigTable[kRGBA_1010102_GrPixelConfig];
    if (this->isMac() || fFamilyGroup >= 3) {
        info->fFlags = ConfigInfo::kAllFlags;
    } else {
        info->fFlags = ConfigInfo::kTextureable_Flag;
    }

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

    // R_16 uses R16Unorm
    info = &fConfigTable[kR_16_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = ConfigInfo::kAllFlags;
    } else {
        info->fFlags = ConfigInfo::kTextureable_Flag | ConfigInfo::kRenderable_Flag;
    }

    // RG_1616 uses RG16Unorm
    info = &fConfigTable[kRG_1616_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = ConfigInfo::kAllFlags;
    } else {
        info->fFlags = ConfigInfo::kTextureable_Flag | ConfigInfo::kRenderable_Flag;
    }

    // Experimental (for Y416 and mutant P016/P010)

    // RGBA_16161616 uses RGBA16Unorm
    info = &fConfigTable[kRGBA_16161616_GrPixelConfig];
    if (this->isMac()) {
        info->fFlags = ConfigInfo::kAllFlags;
    } else {
        info->fFlags = ConfigInfo::kTextureable_Flag | ConfigInfo::kRenderable_Flag;
    }

    // RG_half uses RG16Float
    info = &fConfigTable[kRG_half_GrPixelConfig];
    info->fFlags = ConfigInfo::kAllFlags;
}

void GrMtlCaps::initStencilFormat(id<MTLDevice> physDev) {
    fPreferredStencilFormat = StencilFormat{ MTLPixelFormatStencil8, 8, 8, true };
}

bool GrMtlCaps::onSurfaceSupportsWritePixels(const GrSurface* surface) const {
    if (auto rt = surface->asRenderTarget()) {
        return rt->numSamples() <= 1 && SkToBool(surface->asTexture());
    }
    return true;
}

// A near clone of format_color_type_valid_pair
GrPixelConfig validate_sized_format(GrMTLPixelFormat grFormat, GrColorType ct) {
    MTLPixelFormat format = static_cast<MTLPixelFormat>(grFormat);
    switch (ct) {
        case GrColorType::kUnknown:
            return kUnknown_GrPixelConfig;
        case GrColorType::kAlpha_8:
            if (MTLPixelFormatA8Unorm == format) {
                return kAlpha_8_as_Alpha_GrPixelConfig;
            } else if (MTLPixelFormatR8Unorm == format) {
                return kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
#ifdef SK_BUILD_FOR_MAC
        case GrColorType::kBGR_565:
        case GrColorType::kABGR_4444:
            return kUnknown_GrPixelConfig;
            break;
#else
        case GrColorType::kBGR_565:
            if (MTLPixelFormatB5G6R5Unorm == format) {
                return kRGB_565_GrPixelConfig;
            }
            break;
        case GrColorType::kABGR_4444:
            if (MTLPixelFormatABGR4Unorm == format) {
                return kRGBA_4444_GrPixelConfig;
            }
            break;
#endif
        case GrColorType::kRGBA_8888:
            if (MTLPixelFormatRGBA8Unorm == format) {
                return kRGBA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_8888_SRGB:
            if (MTLPixelFormatRGBA8Unorm_sRGB == format) {
                return kSRGBA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGB_888x:
            if (MTLPixelFormatRGBA8Unorm == format) {
                return kRGB_888X_GrPixelConfig;
            }
            break;
        case GrColorType::kRG_88:
            if (MTLPixelFormatRG8Unorm == format) {
                return kRG_88_GrPixelConfig;
            }
            break;
        case GrColorType::kBGRA_8888:
            if (MTLPixelFormatBGRA8Unorm == format) {
                return kBGRA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_1010102:
            if (MTLPixelFormatRGB10A2Unorm == format) {
                return kRGBA_1010102_GrPixelConfig;
            }
            break;
        case GrColorType::kGray_8:
            if (MTLPixelFormatR8Unorm == format) {
                return kGray_8_as_Red_GrPixelConfig;
            }
            break;
        case GrColorType::kAlpha_F16:
            if (MTLPixelFormatR16Float == format) {
                return kAlpha_half_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_F16:
            if (MTLPixelFormatRGBA16Float == format) {
                return kRGBA_half_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_F16_Clamped:
            if (MTLPixelFormatRGBA16Float == format) {
                return kRGBA_half_Clamped_GrPixelConfig;
            }
            break;
        case GrColorType::kRG_F32:
            if (MTLPixelFormatRG32Float == format) {
                return kRG_float_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_F32:
            if (MTLPixelFormatRGBA32Float == format) {
                return kRGBA_float_GrPixelConfig;
            }
            break;
        case GrColorType::kR_16:
            if (MTLPixelFormatR16Unorm == format) {
                return kR_16_GrPixelConfig;
            }
            break;
        case GrColorType::kRG_1616:
            if (MTLPixelFormatRG16Unorm == format) {
                return kRG_1616_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_16161616:
            if (MTLPixelFormatRGBA16Unorm == format) {
                return kRGBA_16161616_GrPixelConfig;
            }
            break;
        case GrColorType::kRG_F16:
            if (MTLPixelFormatRG16Float == format) {
                return kRG_half_GrPixelConfig;
            }
            break;
    }

    return kUnknown_GrPixelConfig;
}

GrPixelConfig GrMtlCaps::validateBackendRenderTarget(const GrBackendRenderTarget& rt,
                                                     GrColorType ct) const {
    GrMtlTextureInfo fbInfo;
    if (!rt.getMtlTextureInfo(&fbInfo)) {
        return kUnknown_GrPixelConfig;
    }

    id<MTLTexture> texture = (__bridge id<MTLTexture>) fbInfo.fTexture.get();
    return validate_sized_format(texture.pixelFormat, ct);
}

bool GrMtlCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                  const GrBackendFormat& format) const {
    const GrMTLPixelFormat* mtlFormat = format.getMtlFormat();
    if (!mtlFormat) {
        return false;
    }

    return kUnknown_GrPixelConfig != validate_sized_format(*mtlFormat, ct);
}


GrPixelConfig GrMtlCaps::onGetConfigFromBackendFormat(const GrBackendFormat& format,
                                                      GrColorType ct) const {
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
        case MTLPixelFormatRG8Unorm:
            return kRG_88_GrPixelConfig;
            break;
        case MTLPixelFormatRGBA8Unorm:
            return kRGBA_8888_GrPixelConfig;
            break;
        case MTLPixelFormatBGRA8Unorm:
            return kBGRA_8888_GrPixelConfig;
            break;
        case MTLPixelFormatRGB10A2Unorm:
            return kRGBA_1010102_GrPixelConfig;
            break;
        case MTLPixelFormatR16Unorm:
            return kR_16_GrPixelConfig;
            break;
        case MTLPixelFormatRG16Unorm:
            return kRG_1616_GrPixelConfig;
            break;
        // Experimental (for Y416 and mutant P016/P010)
        case MTLPixelFormatRGBA16Unorm:
            return kRGBA_16161616_GrPixelConfig;
            break;
        case MTLPixelFormatRG16Float:
            return kRG_half_GrPixelConfig;
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

GrBackendFormat GrMtlCaps::getBackendFormatFromColorType(GrColorType ct) const {
    GrPixelConfig config = GrColorTypeToPixelConfig(ct);
    if (config == kUnknown_GrPixelConfig) {
        return GrBackendFormat();
    }
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(config, &format)) {
        return GrBackendFormat();
    }
    return GrBackendFormat::MakeMtl(format);
}

GrBackendFormat GrMtlCaps::getBackendFormatFromCompressionType(
        SkImage::CompressionType compressionType) const {
    switch (compressionType) {
        case SkImage::kETC1_CompressionType:
#ifdef SK_BUILD_FOR_MAC
            return {};
#else
            return GrBackendFormat::MakeMtl(MTLPixelFormatETC2_RGB8);
#endif
    }
    SK_ABORT("Invalid compression type");
    return {};
}

#ifdef SK_DEBUG
static bool format_color_type_valid_pair(MTLPixelFormat format, GrColorType colorType) {
    switch (colorType) {
        case GrColorType::kUnknown:
            return false;
        case GrColorType::kAlpha_8:
            return MTLPixelFormatA8Unorm == format || MTLPixelFormatR8Unorm == format;
        case GrColorType::kBGR_565:
#ifdef SK_BUILD_FOR_MAC
            return false;
#else
            return MTLPixelFormatB5G6R5Unorm == format;
#endif
        case GrColorType::kABGR_4444:
#ifdef SK_BUILD_FOR_MAC
            return false;
#else
            return MTLPixelFormatABGR4Unorm == format;
#endif
        case GrColorType::kRGBA_8888:
            return MTLPixelFormatRGBA8Unorm == format;
        case GrColorType::kRGBA_8888_SRGB:
            return MTLPixelFormatRGBA8Unorm_sRGB == format;
        case GrColorType::kRGB_888x:
            GR_STATIC_ASSERT(GrCompressionTypeClosestColorType(SkImage::kETC1_CompressionType) ==
                             GrColorType::kRGB_888x);
#ifdef SK_BUILD_FOR_MAC
            return MTLPixelFormatRGBA8Unorm == format;
#else
            return MTLPixelFormatRGBA8Unorm == format || MTLPixelFormatETC2_RGB8 == format;
#endif
        case GrColorType::kRG_88:
            return MTLPixelFormatRG8Unorm == format;
        case GrColorType::kBGRA_8888:
            return MTLPixelFormatBGRA8Unorm == format || MTLPixelFormatBGRA8Unorm_sRGB == format;
        case GrColorType::kRGBA_1010102:
            return MTLPixelFormatRGB10A2Unorm == format;
        case GrColorType::kGray_8:
            return MTLPixelFormatR8Unorm == format;
        case GrColorType::kAlpha_F16:
            return MTLPixelFormatR16Float == format;
        case GrColorType::kRGBA_F16:
            return MTLPixelFormatRGBA16Float == format;
        case GrColorType::kRGBA_F16_Clamped:
            return MTLPixelFormatRGBA16Float == format;
        case GrColorType::kRG_F32:
            return MTLPixelFormatRG32Float == format;
        case GrColorType::kRGBA_F32:
            return MTLPixelFormatRGBA32Float == format;
        case GrColorType::kR_16:
            return MTLPixelFormatR16Unorm == format;
        case GrColorType::kRG_1616:
            return MTLPixelFormatRG16Unorm == format;
        // Experimental (for Y416 and mutant P016/P010)
        case GrColorType::kRGBA_16161616:
            return MTLPixelFormatRGBA16Unorm == format;
        case GrColorType::kRG_F16:
            return MTLPixelFormatRG16Float == format;
    }
    SK_ABORT("Unknown color type");
    return false;
}
#endif

static GrSwizzle get_swizzle(const GrBackendFormat& format, GrColorType colorType,
                             bool forOutput) {
    SkASSERT(format.getMtlFormat());
    MTLPixelFormat mtlFormat = static_cast<MTLPixelFormat>(*format.getMtlFormat());

    SkASSERT(format_color_type_valid_pair(mtlFormat, colorType));

    switch (colorType) {
        case GrColorType::kAlpha_8:
            if (mtlFormat == MTLPixelFormatA8Unorm) {
                if (!forOutput) {
                    return GrSwizzle::AAAA();
                }
            } else {
                SkASSERT(mtlFormat == MTLPixelFormatR8Unorm);
                if (forOutput) {
                    return GrSwizzle::AAAA();
                } else {
                    return GrSwizzle::RRRR();
                }
            }
            break;
        case GrColorType::kAlpha_F16:
            if (forOutput) {
                return GrSwizzle::AAAA();
            } else {
                return GrSwizzle::RRRR();
            }
        case GrColorType::kGray_8:
            if (!forOutput) {
                return GrSwizzle::RRRA();
            }
            break;
        case GrColorType::kRGB_888x:
            return GrSwizzle::RGB1();
        default:
            return GrSwizzle::RGBA();
    }
    return GrSwizzle::RGBA();
}

GrSwizzle GrMtlCaps::getTextureSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    return get_swizzle(format, colorType, false);
}
GrSwizzle GrMtlCaps::getOutputSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    return get_swizzle(format, colorType, true);
}
