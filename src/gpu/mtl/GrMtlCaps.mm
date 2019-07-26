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
    this->initFormatTable();
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

bool GrMtlCaps::isFormatTexturable(GrColorType, const GrBackendFormat& format) const {
    if (!format.getMtlFormat()) {
        return false;
    }

    MTLPixelFormat mtlFormat = static_cast<MTLPixelFormat>(*format.getMtlFormat());
    return this->isFormatTexturable(mtlFormat);
}

bool GrMtlCaps::isConfigTexturable(GrPixelConfig config) const {
    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(config, &format)) {
        return false;
    }
    return this->isFormatTexturable(format);
}

bool GrMtlCaps::isFormatTexturable(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTextureable_Flag && formatInfo.fFlags);
}

bool GrMtlCaps::isFormatRenderable(MTLPixelFormat format) const {
    return this->maxRenderTargetSampleCount(format) > 0;
}

int GrMtlCaps::maxRenderTargetSampleCount(GrColorType grColorType,
                                          const GrBackendFormat& format) const {
    if (!format.getMtlFormat()) {
        return 0;
    }

    // Currently we don't allow RGB_888X to be renderable because we don't have a way to
    // handle blends that reference dst alpha when the values in the dst alpha channel are
    // uninitialized.
    if (GrColorType::kRGB_888x == grColorType) {
        return 0;
    }

    MTLPixelFormat mtlFormat = static_cast<MTLPixelFormat>(*format.getMtlFormat());
    return this->maxRenderTargetSampleCount(mtlFormat);
}

int GrMtlCaps::maxRenderTargetSampleCount(GrPixelConfig config) const {
    // Currently we don't allow RGB_888X or RGB_888 to be renderable because we don't have a way to
    // handle blends that reference dst alpha when the values in the dst alpha channel are
    // uninitialized.
    if (config == kRGB_888X_GrPixelConfig || config == kRGB_888_GrPixelConfig) {
        return 0;
    }

    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(config, &format)) {
        return 0;
    }
    return this->maxRenderTargetSampleCount(format);
}

int GrMtlCaps::maxRenderTargetSampleCount(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (formatInfo.fFlags & FormatInfo::kMSAA_Flag) {
        return fSampleCounts[fSampleCounts.count() - 1];
    } else if (formatInfo.fFlags & FormatInfo::kRenderable_Flag) {
        return 1;
    }
    return 0;
}

int GrMtlCaps::getRenderTargetSampleCount(int requestedCount, GrColorType grColorType,
                                          const GrBackendFormat& format) const {
    if (!format.getMtlFormat()) {
        return 0;
    }

    // Currently we don't allow RGB_888X to be renderable because we don't have a way to
    // handle blends that reference dst alpha when the values in the dst alpha channel are
    // uninitialized.
    if (GrColorType::kRGB_888x == grColorType) {
        return 0;
    }

    MTLPixelFormat mtlFormat = static_cast<MTLPixelFormat>(*format.getMtlFormat());
    return this->getRenderTargetSampleCount(requestedCount, mtlFormat);
}

int GrMtlCaps::getRenderTargetSampleCount(int requestedCount, GrPixelConfig config) const {
    // Currently we don't allow RGB_888X or RGB_888 to be renderable because we don't have a way to
    // handle blends that reference dst alpha when the values in the dst alpha channel are
    // uninitialized.
    if (config == kRGB_888X_GrPixelConfig || config == kRGB_888_GrPixelConfig) {
        return 0;
    }

    MTLPixelFormat format;
    if (!GrPixelConfigToMTLFormat(config, &format)) {
        return 0;
    }
    return this->getRenderTargetSampleCount(requestedCount, format);
}

int GrMtlCaps::getRenderTargetSampleCount(int requestedCount, MTLPixelFormat format) const {
    requestedCount = SkTMax(requestedCount, 1);
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (formatInfo.fFlags & FormatInfo::kMSAA_Flag) {
        int count = fSampleCounts.count();
        for (int i = 0; i < count; ++i) {
            if (fSampleCounts[i] >= requestedCount) {
                return fSampleCounts[i];
            }
        }
    } else if (formatInfo.fFlags & FormatInfo::kRenderable_Flag) {
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

// These are all the valid MTLPixelFormats that we support in Skia.  They are roughly ordered from
// most frequently used to least to improve look up times in arrays.
static constexpr MTLPixelFormat kMtlFormats[] = {
    MTLPixelFormatInvalid,

    MTLPixelFormatRGBA8Unorm,
    MTLPixelFormatR8Unorm,
    MTLPixelFormatBGRA8Unorm,
#ifdef SK_BUILD_FOR_IOS
    MTLPixelFormatB5G6R5Unorm,
#endif
    MTLPixelFormatRGBA16Float,
    MTLPixelFormatR16Float,
    MTLPixelFormatRG8Unorm,
    MTLPixelFormatRGB10A2Unorm,
#ifdef SK_BUILD_FOR_IOS
    MTLPixelFormatABGR4Unorm,
#endif
    MTLPixelFormatRGBA32Float,
    MTLPixelFormatRGBA8Unorm_sRGB,
    MTLPixelFormatR16Unorm,
    MTLPixelFormatRG16Unorm,
#ifdef SK_BUILD_FOR_IOS
    MTLPixelFormatETC2_RGB8,
#endif
    // Experimental (for Y416 and mutant P016/P010)
    MTLPixelFormatRGBA16Unorm,
    MTLPixelFormatRG16Float,
};

size_t GrMtlCaps::GetFormatIndex(MTLPixelFormat pixelFormat) {
    static_assert(SK_ARRAY_COUNT(kMtlFormats) == GrMtlCaps::kNumMtlFormats,
                  "Size of kMtlFormats array must match static value in header");
    // Start at 1, 0 is sentinel value (MTLPixelFormatInvalid)
    for (size_t i = 1; i < GrMtlCaps::kNumMtlFormats; ++i) {
        if (kMtlFormats[i] == pixelFormat) {
            return i;
        }
    }
    SK_ABORT("Invalid MTLPixelFormat");
    return 0;
}

void GrMtlCaps::initFormatTable() {
    FormatInfo* info;

    // R8Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatR8Unorm)];
    info->fFlags = FormatInfo::kAllFlags;

#ifdef SK_BUILD_FOR_IOS
    // B5G6R5Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatB5G6R5Unorm)];
    info->fFlags = FormatInfo::kAllFlags;

    // ABGR4Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatABGR4Unorm)];
    info->fFlags = FormatInfo::kAllFlags;
#endif

    // RGBA8Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA8Unorm)];
    info->fFlags = FormatInfo::kAllFlags;

    // RG8Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG8Unorm)];
    info->fFlags = FormatInfo::kTextureable_Flag;

    // BGRA_8888 uses BGRA8Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatBGRA8Unorm)];
    info->fFlags = FormatInfo::kAllFlags;

    // RGBA8Unorm_sRGB
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA8Unorm_sRGB)];
    info->fFlags = FormatInfo::kAllFlags;

    // RGB10A2Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGB10A2Unorm)];
    if (this->isMac() || fFamilyGroup >= 3) {
        info->fFlags = FormatInfo::kAllFlags;
    } else {
        info->fFlags = FormatInfo::kTextureable_Flag;
    }

    // RGBA32Float
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA32Float)];
    if (this->isMac()) {
        info->fFlags = FormatInfo::kAllFlags;
    } else {
        info->fFlags = 0;
    }

    // R16Float
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatR16Float)];
    info->fFlags = FormatInfo::kAllFlags;

    // RGBA16Float
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA16Float)];
    info->fFlags = FormatInfo::kAllFlags;

    // R16Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatR16Unorm)];
    if (this->isMac()) {
        info->fFlags = FormatInfo::kAllFlags;
    } else {
        info->fFlags = FormatInfo::kTextureable_Flag | FormatInfo::kRenderable_Flag;
    }

    // RG16Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG16Unorm)];
    if (this->isMac()) {
        info->fFlags = FormatInfo::kAllFlags;
    } else {
        info->fFlags = FormatInfo::kTextureable_Flag | FormatInfo::kRenderable_Flag;
    }

#ifdef SK_BUILD_FOR_IOS
    // ETC2_RGB8
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatETC2_RGB8)];
    info->fFlags = 0; // TBD
#endif

    // Experimental (for Y416 and mutant P016/P010)

    // RGBA16Unorm
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA16Unorm)];
    if (this->isMac()) {
        info->fFlags = FormatInfo::kAllFlags;
    } else {
        info->fFlags = FormatInfo::kTextureable_Flag | FormatInfo::kRenderable_Flag;
    }

    // RG16Float
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG16Float)];
    info->fFlags = FormatInfo::kAllFlags;
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
                return kAlpha_half_as_Red_GrPixelConfig;
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

GrColorType GrMtlCaps::getYUVAColorTypeFromBackendFormat(const GrBackendFormat& format) const {
    const GrMTLPixelFormat* grMtlFormat = format.getMtlFormat();
    if (!grMtlFormat) {
        return GrColorType::kUnknown;
    }

    MTLPixelFormat mtlFormat = static_cast<MTLPixelFormat>(*grMtlFormat);

    switch (mtlFormat) {
        case MTLPixelFormatA8Unorm:           // fall through
        case MTLPixelFormatR8Unorm:           return GrColorType::kAlpha_8;
        case MTLPixelFormatRG8Unorm:          return GrColorType::kRG_88;
        case MTLPixelFormatRGBA8Unorm:        return GrColorType::kRGBA_8888;
        case MTLPixelFormatBGRA8Unorm:        return GrColorType::kBGRA_8888;
        case MTLPixelFormatRGB10A2Unorm:      return GrColorType::kRGBA_1010102;
        case MTLPixelFormatR16Unorm:          return GrColorType::kR_16;
        case MTLPixelFormatRG16Unorm:         return GrColorType::kRG_1616;
        // Experimental (for Y416 and mutant P016/P010)
        case MTLPixelFormatRGBA16Unorm:       return GrColorType::kRGBA_16161616;
        case MTLPixelFormatRG16Float:         return GrColorType::kRG_F16;
        default:                              return GrColorType::kUnknown;
    }

    SkUNREACHABLE;
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

GrCaps::SupportedWrite GrMtlCaps::supportedWritePixelsColorType(GrPixelConfig config,
                                                                GrColorType srcColorType) const {
    GrColorType ct = GrPixelConfigToColorType(config);
    return {ct, static_cast<size_t>(GrColorTypeBytesPerPixel(ct))};
}

GrCaps::SupportedRead GrMtlCaps::onSupportedReadPixelsColorType(
        GrColorType srcColorType, const GrBackendFormat& srcBackendFormat,
        GrColorType dstColorType) const {
    const GrMTLPixelFormat* grMtlFormat = srcBackendFormat.getMtlFormat();
    if (!grMtlFormat) {
        return {GrSwizzle(), GrColorType::kUnknown, 0};
    }

    GrColorType readCT = GrColorType::kUnknown;
    switch (*grMtlFormat) {
        case MTLPixelFormatRGBA8Unorm:
            readCT = GrColorType::kRGBA_8888;
            break;
        case MTLPixelFormatR8Unorm:
            if (srcColorType == GrColorType::kAlpha_8) {
                readCT = GrColorType::kAlpha_8;
            } else if (srcColorType == GrColorType::kGray_8) {
                readCT = GrColorType::kGray_8;
            }
            break;
        case MTLPixelFormatBGRA8Unorm:
            readCT = GrColorType::kBGRA_8888;
            break;
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatB5G6R5Unorm:
            readCT = GrColorType::kBGR_565;
            break;
#endif
        case MTLPixelFormatRGBA16Float:
            if (srcColorType == GrColorType::kRGBA_F16) {
                readCT = GrColorType::kRGBA_F16;
            } else if (srcColorType == GrColorType::kRGBA_F16_Clamped){
                readCT = GrColorType::kRGBA_F16_Clamped;
            }
            break;
        case MTLPixelFormatR16Float:
            readCT = GrColorType::kAlpha_F16;
            break;
        case MTLPixelFormatRG8Unorm:
            readCT = GrColorType::kRG_88;
            break;
        case MTLPixelFormatRGB10A2Unorm:
            readCT = GrColorType::kRGBA_1010102;
            break;
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatABGR4Unorm:
            readCT = GrColorType::kABGR_4444;
            break;
#endif
        case MTLPixelFormatRGBA32Float:
            readCT = GrColorType::kRGBA_F32;
            break;
        case MTLPixelFormatRGBA8Unorm_sRGB:
            readCT = GrColorType::kRGBA_8888_SRGB;
            break;
        case MTLPixelFormatR16Unorm:
            readCT = GrColorType::kR_16;
            break;
        case MTLPixelFormatRG16Unorm:
            readCT = GrColorType::kRG_1616;
            break;
        // Experimental (for Y416 and mutant P016/P010)
        case MTLPixelFormatRGBA16Unorm:
            readCT = GrColorType::kRGBA_16161616;
            break;
        case MTLPixelFormatRG16Float:
            readCT = GrColorType::kRG_F16;
            break;
        default:
            // readCT stays as kUnknown
            break;
    }
    // Metal requires the destination offset for copyFromTexture to be a multiple of the textures
    // pixels size.
    return {GrSwizzle::RGBA(), readCT, static_cast<size_t>(GrColorTypeBytesPerPixel(readCT))};
}

