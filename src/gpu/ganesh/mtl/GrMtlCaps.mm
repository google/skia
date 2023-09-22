/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlCaps.h"

#include "include/core/SkRect.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkReadBuffer.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrProcessor.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/mtl/GrMtlRenderTarget.h"
#include "src/gpu/ganesh/mtl/GrMtlTexture.h"
#include "src/gpu/ganesh/mtl/GrMtlUtil.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#if defined(GR_TEST_UTILS)
    #include "src/gpu/ganesh/TestFormatColorTypeCombination.h"
#endif

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

GrMtlCaps::GrMtlCaps(const GrContextOptions& contextOptions, const id<MTLDevice> device)
        : INHERITED(contextOptions) {
    fShaderCaps = std::make_unique<GrShaderCaps>();

    this->initGPUFamily(device);
    this->initGrCaps(device);
    this->initShaderCaps();
    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(contextOptions, device);
    }

    this->initFormatTable();
    this->initStencilFormat(device);

    // TODO: appears to be slow with Mac msaa8, disabled for now
    fStoreAndMultisampleResolveSupport = (fGPUFamily == GPUFamily::kApple &&
                                          fFamilyGroup >= 3);
    // Also slow with non-Apple silicon
    fPreferDiscardableMSAAAttachment = (fGPUFamily == GPUFamily::kApple);

    this->finishInitialization(contextOptions);
}

// translates from older MTLFeatureSet interface to MTLGPUFamily interface
bool GrMtlCaps::getGPUFamilyFromFeatureSet(id<MTLDevice> device,
                                           GPUFamily* gpuFamily,
                                           int* group) {
// MTLFeatureSet is deprecated for newer versions of the SDK
#if GR_METAL_SDK_VERSION < 300

#if defined(SK_BUILD_FOR_MAC)
    // Apple Silicon is only available in later OSes
    *gpuFamily = GPUFamily::kMac;
    // Mac OSX 14
    if (@available(macOS 10.14, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily2_v1]) {
            *group = 2;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v4]) {
            *group = 1;
            return true;
        }
    }
    // Mac OSX 13
    if (@available(macOS 10.13, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v3]) {
            *group = 1;
            return true;
        }
    }
    // Mac OSX 12
    if (@available(macOS 10.12, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v2]) {
            *group = 1;
            return true;
        }
    }
    // Mac OSX 11
    if (@available(macOS 10.11, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v1]) {
            *group = 1;
            return true;
        }
    }
#elif defined(SK_BUILD_FOR_IOS)
    // TODO: support tvOS
   *gpuFamily = GPUFamily::kApple;
    // iOS 12
    if (@available(iOS 12.0, tvOS 12.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily5_v1]) {
            *group = 5;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v2]) {
            *group = 4;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v4]) {
            *group = 3;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v5]) {
            *group = 2;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v5]) {
            *group = 1;
            return true;
        }
    }
    // iOS 11
    if (@available(iOS 11.0, tvOS 11.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v1]) {
            *group = 4;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v3]) {
            *group = 3;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v4]) {
            *group = 2;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v4]) {
            *group = 1;
            return true;
        }
    }
    // iOS 10
    if (@available(iOS 10.0, tvOS 10.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2]) {
            *group = 3;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v3]) {
            *group = 2;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v3]) {
            *group = 1;
            return true;
        }
    }
    // We don't support earlier OSes
#endif

#endif // GR_METAL_SDK_VERSION < 300

    // No supported GPU families were found
    return false;
}

bool GrMtlCaps::getGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group) {
#if GR_METAL_SDK_VERSION >= 220
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        // Apple Silicon
#if GR_METAL_SDK_VERSION >= 230
        if ([device supportsFamily:MTLGPUFamilyApple7]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 7;
            return true;
        }
#endif
#ifdef SK_BUILD_FOR_IOS
        if ([device supportsFamily:MTLGPUFamilyApple6]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 6;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple5]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 5;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple4]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 4;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple3]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 3;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple2]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 2;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple1]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 1;
            return true;
        }
#endif

        // Older Macs
        // At the moment MacCatalyst families have the same features as Mac,
        // so we treat them the same
        if ([device supportsFamily:MTLGPUFamilyMac2] ||
            [device supportsFamily:MTLGPUFamilyMacCatalyst2]) {
            *gpuFamily = GPUFamily::kMac;
            *group = 2;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyMac1] ||
            [device supportsFamily:MTLGPUFamilyMacCatalyst1]) {
            *gpuFamily = GPUFamily::kMac;
            *group = 1;
            return true;
        }
    }
#endif

    // No supported GPU families were found
    return false;
}

void GrMtlCaps::initGPUFamily(id<MTLDevice> device) {
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        if (this->getGPUFamily(device, &fGPUFamily, &fFamilyGroup)) {
            return;
        }
    } else {
        if (this->getGPUFamilyFromFeatureSet(device, &fGPUFamily, &fFamilyGroup)) {
            return;
        }
    }
    // We don't know what this is, fall back to minimum defaults
#ifdef SK_BUILD_FOR_MAC
    fGPUFamily = GPUFamily::kMac;
    fFamilyGroup = 1;
#else
    fGPUFamily = GPUFamily::kApple;
    fFamilyGroup = 1;
#endif
}

bool GrMtlCaps::canCopyAsBlit(MTLPixelFormat dstFormat, int dstSampleCount,
                              MTLPixelFormat srcFormat, int srcSampleCount,
                              const SkIRect& srcRect, const SkIPoint& dstPoint,
                              bool areDstSrcSameObj) const {
    if (!dstFormat || dstFormat != srcFormat) {
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

bool GrMtlCaps::canCopyAsResolve(MTLPixelFormat dstFormat, int dstSampleCount,
                                 MTLPixelFormat srcFormat, int srcSampleCount,
                                 bool srcIsRenderTarget, const SkISize srcDimensions,
                                 const SkIRect& srcRect,
                                 const SkIPoint& dstPoint,
                                 bool areDstSrcSameObj) const {
    if (areDstSrcSameObj) {
        return false;
    }
    if (dstFormat != srcFormat) {
        return false;
    }
    if (dstSampleCount > 1 || srcSampleCount == 1 || !srcIsRenderTarget) {
        return false;
    }

    // TODO: Support copying subrectangles
    if (dstPoint != SkIPoint::Make(0, 0)) {
        return false;
    }
    if (srcRect != SkIRect::MakeSize(srcDimensions)) {
        return false;
    }

    return true;
}

bool GrMtlCaps::onCanCopySurface(const GrSurfaceProxy* dst, const SkIRect& dstRect,
                                 const GrSurfaceProxy* src, const SkIRect& srcRect) const {
    // Metal does not support scaling copies
    if (srcRect.size() != dstRect.size()) {
        return false;
    }

    int dstSampleCnt = 1;
    int srcSampleCnt = 1;
    if (const GrRenderTargetProxy* rtProxy = dst->asRenderTargetProxy()) {
        dstSampleCnt = rtProxy->numSamples();
    }
    if (const GrRenderTargetProxy* rtProxy = src->asRenderTargetProxy()) {
        srcSampleCnt = rtProxy->numSamples();
    }

    // TODO: need some way to detect whether the proxy is framebufferOnly

    const SkIPoint dstPoint = dstRect.topLeft();
    if (this->canCopyAsBlit(GrBackendFormatAsMTLPixelFormat(dst->backendFormat()), dstSampleCnt,
                            GrBackendFormatAsMTLPixelFormat(src->backendFormat()), srcSampleCnt,
                            srcRect, dstPoint, dst == src)) {
        return true;
    }
    bool srcIsRenderTarget = src->asRenderTargetProxy();
    MTLPixelFormat dstFormat = GrBackendFormatAsMTLPixelFormat(dst->backendFormat());
    MTLPixelFormat srcFormat = GrBackendFormatAsMTLPixelFormat(src->backendFormat());
    return this->canCopyAsResolve(dstFormat, dstSampleCnt,
                                  srcFormat, srcSampleCnt,
                                  srcIsRenderTarget, src->backingStoreDimensions(), srcRect,
                                  dstPoint,
                                  dst == src);
}

void GrMtlCaps::initGrCaps(id<MTLDevice> device) {
#if defined(GR_TEST_UTILS)
    this->setDeviceName([[device name] UTF8String]);
#endif

    // Max vertex attribs is the same on all devices
    fMaxVertexAttributes = 31;

    // Metal does not support scissor + clear
    fPerformPartialClearsAsDraws = true;

    // We always copy in/out of a transfer buffer so it's trivial to support row bytes.
    fReadPixelsRowBytesSupport = true;
    fWritePixelsRowBytesSupport = true;
    fTransferPixelsToRowBytesSupport = true;

    // RenderTarget and Texture size
    if (this->isMac() || fFamilyGroup >= 3) {
        fMaxRenderTargetSize = 16384;
    } else {
        fMaxRenderTargetSize = 8192;
    }
    fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;
    fMaxTextureSize = fMaxRenderTargetSize;

    fMaxPushConstantsSize = 4*1024;
    fTransferBufferRowBytesAlignment = 1;

    // This is documented to be 4 for all Macs. However, on Apple GPUs on Mac it appears there is
    // no actual alignment requirement
    // https://developer.apple.com/documentation/metal/mtlblitcommandencoder/1400767-copyfrombuffer
    if (this->isMac()) {
        fTransferFromBufferToBufferAlignment = 4;
        // Buffer updates are sometimes implemented through transfers in GrMtlBuffer.
        fBufferUpdateDataPreserveAlignment = 4;
    }

    // Metal buffers are initialized to zero (if not created with initial data)
    fBuffersAreInitiallyZero = true;

    // Init sample counts. All devices support 1 (i.e. 0 in skia).
    fSampleCounts.push_back(1);
    if (@available(iOS 9.0, tvOS 9.0, *)) {
        for (auto sampleCnt : {2, 4, 8}) {
            if ([device supportsTextureSampleCount:sampleCnt]) {
                fSampleCounts.push_back(sampleCnt);
            }
        }
    }

    // Clamp to border is supported on Mac 10.12 and higher. It is not supported on iOS.
    fClampToBorderSupport = false;
#ifdef SK_BUILD_FOR_MAC
    if (@available(macOS 10.12, *)) {
        fClampToBorderSupport = true;
    }
#endif

    // Starting with the assumption that there isn't a reason to not map small buffers.
    fBufferMapThreshold = 0;

    // Buffers are always fully mapped.
    fMapBufferFlags =  kCanMap_MapFlag | kAsyncRead_MapFlag;

    fOversizedStencilSupport = true;

    fNPOTTextureTileSupport = true;  // always available in Metal
    fMipmapSupport = true;   // always available in Metal
    fAnisoSupport = true;   // always available in Metal

    fReuseScratchTextures = true; // Assuming this okay

    fTransferFromBufferToTextureSupport = true;
    fTransferFromSurfaceToBufferSupport = true;
    fTransferFromBufferToBufferSupport  = true;

    fTextureBarrierSupport = false; // Need to figure out if we can do this

    fSampleLocationsSupport = false;

    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        if (this->isMac() || fFamilyGroup >= 3) {
            fDrawInstancedSupport = true;
            fNativeDrawIndirectSupport = true;
        }
    }

    fGpuTracingSupport = false;

    fFenceSyncSupport = true;
    bool supportsMTLEvent = false;
    if (@available(macOS 10.14, iOS 12.0, tvOS 12.0, *)) {
        supportsMTLEvent = true;
    }
    fSemaphoreSupport = supportsMTLEvent;

    fCrossContextTextureSupport = true;
    fHalfFloatVertexAttributeSupport = true;

    fDynamicStateArrayGeometryProcessorTextureSupport = true;
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
    return format_is_srgb(GrBackendFormatAsMTLPixelFormat(format));
}

bool GrMtlCaps::isFormatTexturable(const GrBackendFormat& format, GrTextureType) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    return this->isFormatTexturable(mtlFormat);
}

bool GrMtlCaps::isFormatTexturable(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTexturable_Flag & formatInfo.fFlags);
}

bool GrMtlCaps::isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                              int sampleCount) const {
    if (!this->isFormatRenderable(format, sampleCount)) {
        return false;
    }
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlFormat != MTLPixelFormatInvalid);
    const auto& info = this->getFormatInfo(mtlFormat);
    if (!SkToBool(info.colorTypeFlags(ct) & ColorTypeInfo::kRenderable_Flag)) {
        return false;
    }
    return true;
}

bool GrMtlCaps::isFormatRenderable(const GrBackendFormat& format, int sampleCount) const {
    return this->isFormatRenderable(GrBackendFormatAsMTLPixelFormat(format), sampleCount);
}

bool GrMtlCaps::isFormatRenderable(MTLPixelFormat format, int sampleCount) const {
    return sampleCount <= this->maxRenderTargetSampleCount(format);
}

int GrMtlCaps::maxRenderTargetSampleCount(const GrBackendFormat& format) const {
    return this->maxRenderTargetSampleCount(GrBackendFormatAsMTLPixelFormat(format));
}

int GrMtlCaps::maxRenderTargetSampleCount(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (formatInfo.fFlags & FormatInfo::kMSAA_Flag) {
        return fSampleCounts[fSampleCounts.size() - 1];
    } else if (formatInfo.fFlags & FormatInfo::kRenderable_Flag) {
        return 1;
    }
    return 0;
}

int GrMtlCaps::getRenderTargetSampleCount(int requestedCount,
                                          const GrBackendFormat& format) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);

    return this->getRenderTargetSampleCount(requestedCount, mtlFormat);
}

int GrMtlCaps::getRenderTargetSampleCount(int requestedCount, MTLPixelFormat format) const {
    requestedCount = std::max(requestedCount, 1);
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (!(formatInfo.fFlags & FormatInfo::kRenderable_Flag)) {
        return 0;
    }
    if (formatInfo.fFlags & FormatInfo::kMSAA_Flag) {
        int count = fSampleCounts.size();
        for (int i = 0; i < count; ++i) {
            if (fSampleCounts[i] >= requestedCount) {
                return fSampleCounts[i];
            }
        }
    }
    return 1 == requestedCount ? 1 : 0;
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
    shaderCaps->fExplicitTextureLodSupport = true;

    if (@available(macOS 10.12, iOS 11.0, tvOS 11.0, *)) {
        shaderCaps->fDualSourceBlendingSupport = true;
    } else {
        shaderCaps->fDualSourceBlendingSupport = false;
    }

    // TODO(skia:8270): Re-enable this once bug 8270 is fixed. Will also need to remove asserts in
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
    shaderCaps->fNonsquareMatrixSupport = true;
    shaderCaps->fInverseHyperbolicSupport = true;
    shaderCaps->fVertexIDSupport = true;
    shaderCaps->fInfinitySupport = true;
    shaderCaps->fNonconstantArrayIndexSupport = true;

    // Metal uses IEEE float and half floats so assuming those values here.
    shaderCaps->fFloatIs32Bits = true;
    shaderCaps->fHalfIs32Bits = false;

    shaderCaps->fMaxFragmentSamplers = 16;
}

void GrMtlCaps::applyDriverCorrectnessWorkarounds(const GrContextOptions&, const id<MTLDevice>) {
    // We don't have any active Metal workarounds.
}

// Define these so we can use them to initialize arrays and work around
// the fact that these pixel formats are not always available.
#define kMTLPixelFormatB5G6R5Unorm MTLPixelFormat(40)
#define kMTLPixelFormatABGR4Unorm MTLPixelFormat(42)
#define kMTLPixelFormatETC2_RGB8 MTLPixelFormat(180)

// These are all the valid MTLPixelFormats that we support in Skia.  They are roughly ordered from
// most frequently used to least to improve look up times in arrays.
static constexpr MTLPixelFormat kMtlFormats[] = {
    MTLPixelFormatRGBA8Unorm,
    MTLPixelFormatR8Unorm,
    MTLPixelFormatA8Unorm,
    MTLPixelFormatBGRA8Unorm,
    kMTLPixelFormatB5G6R5Unorm,
    MTLPixelFormatRGBA16Float,
    MTLPixelFormatR16Float,
    MTLPixelFormatRG8Unorm,
    MTLPixelFormatRGB10A2Unorm,
    MTLPixelFormatBGR10A2Unorm,
    kMTLPixelFormatABGR4Unorm,
    MTLPixelFormatRGBA8Unorm_sRGB,
    MTLPixelFormatR16Unorm,
    MTLPixelFormatRG16Unorm,
    kMTLPixelFormatETC2_RGB8,
#ifdef SK_BUILD_FOR_MAC
    MTLPixelFormatBC1_RGBA,
#endif
    MTLPixelFormatRGBA16Unorm,
    MTLPixelFormatRG16Float,

    MTLPixelFormatInvalid,
};

void GrMtlCaps::setColorType(GrColorType colorType, std::initializer_list<MTLPixelFormat> formats) {
#ifdef SK_DEBUG
    for (size_t i = 0; i < kNumMtlFormats; ++i) {
        const auto& formatInfo = fFormatTable[i];
        for (int j = 0; j < formatInfo.fColorTypeInfoCount; ++j) {
            const auto& ctInfo = formatInfo.fColorTypeInfos[j];
            if (ctInfo.fColorType == colorType) {
                bool found = false;
                for (auto it = formats.begin(); it != formats.end(); ++it) {
                    if (kMtlFormats[i] == *it) {
                        found = true;
                    }
                }
                SkASSERT(found);
            }
        }
    }
#endif
    int idx = static_cast<int>(colorType);
    for (auto it = formats.begin(); it != formats.end(); ++it) {
        const auto& info = this->getFormatInfo(*it);
        for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
            if (info.fColorTypeInfos[i].fColorType == colorType) {
                fColorTypeToFormatTable[idx] = *it;
                return;
            }
        }
    }
}

size_t GrMtlCaps::GetFormatIndex(MTLPixelFormat pixelFormat) {
    static_assert(std::size(kMtlFormats) == GrMtlCaps::kNumMtlFormats,
                  "Size of kMtlFormats array must match static value in header");
    for (size_t i = 0; i < GrMtlCaps::kNumMtlFormats; ++i) {
        if (kMtlFormats[i] == pixelFormat) {
            return i;
        }
    }
    SK_ABORT("Invalid MTLPixelFormat: %d", static_cast<int>(pixelFormat));
}

void GrMtlCaps::initFormatTable() {
    FormatInfo* info;

    if (@available(macos 11.0, *)) {
        SkASSERT(kMTLPixelFormatB5G6R5Unorm == MTLPixelFormatB5G6R5Unorm);
        SkASSERT(kMTLPixelFormatABGR4Unorm == MTLPixelFormatABGR4Unorm);
        SkASSERT(kMTLPixelFormatETC2_RGB8 == MTLPixelFormatETC2_RGB8);
    }

    // Format: R8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatR8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 3;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: R8Unorm, Surface: kAlpha_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kR_8;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
        // Format: R8Unorm, Surface: kAlpha_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kAlpha_8;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
            ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
        }
        // Format: R8Unorm, Surface: kGray_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kGray_8;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            ctInfo.fReadSwizzle = skgpu::Swizzle("rrr1");
        }
    }

    // Format: A8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatA8Unorm)];
        info->fFlags = FormatInfo::kTexturable_Flag;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: A8Unorm, Surface: kAlpha_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kAlpha_8;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    if (@available(macOS 11.0, iOS 8.0, tvOS 9.0, *)) {
        if (this->isApple()) {
            // Format: B5G6R5Unorm
            {
                info = &fFormatTable[GetFormatIndex(MTLPixelFormatB5G6R5Unorm)];
                info->fFlags = FormatInfo::kAllFlags;
                info->fColorTypeInfoCount = 1;
                info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
                int ctIdx = 0;
                // Format: B5G6R5Unorm, Surface: kBGR_565
                {
                    auto& ctInfo = info->fColorTypeInfos[ctIdx++];
                    ctInfo.fColorType = GrColorType::kBGR_565;
                    ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag |
                                    ColorTypeInfo::kRenderable_Flag;
                }
            }

            // Format: ABGR4Unorm
            {
                info = &fFormatTable[GetFormatIndex(MTLPixelFormatABGR4Unorm)];
                info->fFlags = FormatInfo::kAllFlags;
                info->fColorTypeInfoCount = 1;
                info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
                int ctIdx = 0;
                // Format: ABGR4Unorm, Surface: kABGR_4444
                {
                    auto& ctInfo = info->fColorTypeInfos[ctIdx++];
                    ctInfo.fColorType = GrColorType::kABGR_4444;
                    ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag |
                                    ColorTypeInfo::kRenderable_Flag;
                }
            }
        }
    }

    // Format: RGBA8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 2;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RGBA8Unorm, Surface: kRGBA_8888
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGBA_8888;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
        // Format: RGBA8Unorm, Surface: kRGB_888x
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGB_888x;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            ctInfo.fReadSwizzle = skgpu::Swizzle::RGB1();
        }
    }

    // Format: RG8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RG8Unorm, Surface: kRG_88
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRG_88;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: BGRA8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatBGRA8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: BGRA8Unorm, Surface: kBGRA_8888
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kBGRA_8888;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: RGBA8Unorm_sRGB
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA8Unorm_sRGB)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RGBA8Unorm_sRGB, Surface: kRGBA_8888_SRGB
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGBA_8888_SRGB;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: RGB10A2Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGB10A2Unorm)];
        if (this->isMac() || fFamilyGroup >= 3) {
            info->fFlags = FormatInfo::kAllFlags;
        } else {
            info->fFlags = FormatInfo::kTexturable_Flag;
        }
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RGB10A2Unorm, Surface: kRGBA_1010102
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGBA_1010102;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: BGR10A2Unorm
    if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatBGR10A2Unorm)];
        if (this->isMac() && fFamilyGroup == 1) {
            info->fFlags = FormatInfo::kTexturable_Flag;
        } else {
            info->fFlags = FormatInfo::kAllFlags;
        }
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: BGR10A2Unorm, Surface: kBGRA_1010102
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kBGRA_1010102;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: R16Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatR16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: R16Float, Surface: kAlpha_F16
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kAlpha_F16;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
            ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
        }
    }

    // Format: RGBA16Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 2;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RGBA16Float, Surface: kRGBA_F16
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGBA_F16;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
        // Format: RGBA16Float, Surface: kRGBA_F16_Clamped
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGBA_F16_Clamped;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: R16Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatR16Unorm)];
        if (this->isMac()) {
            info->fFlags = FormatInfo::kAllFlags;
        } else {
            info->fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kRenderable_Flag;
        }
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: R16Unorm, Surface: kAlpha_16
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kAlpha_16;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
            ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
        }
    }

    // Format: RG16Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG16Unorm)];
        if (this->isMac()) {
            info->fFlags = FormatInfo::kAllFlags;
        } else {
            info->fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kRenderable_Flag;
        }
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RG16Unorm, Surface: kRG_1616
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRG_1616;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    if (@available(macOS 11.0, iOS 8.0, tvOS 9.0, *)) {
        if (this->isApple()) {
            // ETC2_RGB8
            info = &fFormatTable[GetFormatIndex(MTLPixelFormatETC2_RGB8)];
            info->fFlags = FormatInfo::kTexturable_Flag;
            // NO supported colorTypes
        }
    }
#ifdef SK_BUILD_FOR_MAC
    if (this->isMac()) {
        // BC1_RGBA
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatBC1_RGBA)];
        info->fFlags = FormatInfo::kTexturable_Flag;
        // NO supported colorTypes
    }
#endif

    // Format: RGBA16Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA16Unorm)];
        if (this->isMac()) {
            info->fFlags = FormatInfo::kAllFlags;
        } else {
            info->fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kRenderable_Flag;
        }
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RGBA16Unorm, Surface: kRGBA_16161616
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRGBA_16161616;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: RG16Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RG16Float, Surface: kRG_F16
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kRG_F16;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Map GrColorTypes (used for creating GrSurfaces) to MTLPixelFormats. The order in which the
    // formats are passed into the setColorType function indicates the priority in selecting which
    // format we use for a given GrcolorType.

    std::fill_n(fColorTypeToFormatTable, kGrColorTypeCnt, MTLPixelFormatInvalid);

    this->setColorType(GrColorType::kAlpha_8,           { MTLPixelFormatR8Unorm,
                                                          MTLPixelFormatA8Unorm });
    if (@available(macOS 11.0, iOS 8.0, tvOS 9.0, *)) {
        if (this->isApple()) {
            this->setColorType(GrColorType::kBGR_565,   { MTLPixelFormatB5G6R5Unorm });
            this->setColorType(GrColorType::kABGR_4444, { MTLPixelFormatABGR4Unorm });
        }
    }
    this->setColorType(GrColorType::kRGBA_8888,         { MTLPixelFormatRGBA8Unorm });
    this->setColorType(GrColorType::kRGBA_8888_SRGB,    { MTLPixelFormatRGBA8Unorm_sRGB });
    this->setColorType(GrColorType::kRGB_888x,          { MTLPixelFormatRGBA8Unorm });
    this->setColorType(GrColorType::kRG_88,             { MTLPixelFormatRG8Unorm });
    this->setColorType(GrColorType::kBGRA_8888,         { MTLPixelFormatBGRA8Unorm });
    this->setColorType(GrColorType::kRGBA_1010102,      { MTLPixelFormatRGB10A2Unorm });
    if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        this->setColorType(GrColorType::kBGRA_1010102,  { MTLPixelFormatBGR10A2Unorm });
    }
    this->setColorType(GrColorType::kGray_8,            { MTLPixelFormatR8Unorm });
    this->setColorType(GrColorType::kAlpha_F16,         { MTLPixelFormatR16Float });
    this->setColorType(GrColorType::kRGBA_F16,          { MTLPixelFormatRGBA16Float });
    this->setColorType(GrColorType::kRGBA_F16_Clamped,  { MTLPixelFormatRGBA16Float });
    this->setColorType(GrColorType::kAlpha_16,          { MTLPixelFormatR16Unorm });
    this->setColorType(GrColorType::kRG_1616,           { MTLPixelFormatRG16Unorm });
    this->setColorType(GrColorType::kRGBA_16161616,     { MTLPixelFormatRGBA16Unorm });
    this->setColorType(GrColorType::kRG_F16,            { MTLPixelFormatRG16Float });
}

void GrMtlCaps::initStencilFormat(id<MTLDevice> physDev) {
    fPreferredStencilFormat = MTLPixelFormatStencil8;
}

bool GrMtlCaps::onSurfaceSupportsWritePixels(const GrSurface* surface) const {
    if (auto rt = surface->asRenderTarget()) {
        return rt->numSamples() <= 1 && SkToBool(surface->asTexture());
    }
    return true;
}

GrCaps::SurfaceReadPixelsSupport GrMtlCaps::surfaceSupportsReadPixels(
        const GrSurface* surface) const {
    if (auto tex = static_cast<const GrMtlTexture*>(surface->asTexture())) {
        // We disallow reading back directly from compressed textures.
        if (skgpu::MtlFormatIsCompressed(tex->attachment()->mtlFormat())) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
    }

    if (auto mtlRT = static_cast<const GrMtlRenderTarget*>(surface->asRenderTarget())) {
        if (mtlRT->numSamples() > 1 && !mtlRT->resolveAttachment()) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
    }
    return SurfaceReadPixelsSupport::kSupported;
}

GrCaps::DstCopyRestrictions GrMtlCaps::getDstCopyRestrictions(const GrRenderTargetProxy* src,
                                                              GrColorType ct) const {
    // If the src is a MSAA RT then the only supported copy action (not considering falling back
    // to a draw) is to resolve from the MSAA src to the non-MSAA dst. Currently we only support
    // resolving the entire texture to a resolve buffer of the same size.
    DstCopyRestrictions restrictions = {};
    if (auto rtProxy = src->asRenderTargetProxy()) {
        if (rtProxy->numSamples() > 1) {
            restrictions.fMustCopyWholeSrc = true;
            restrictions.fRectsMustMatch = GrSurfaceProxy::RectsMustMatch::kYes;
        }
    }
    return restrictions;
}

bool GrMtlCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                  const GrBackendFormat& format) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);

    const auto& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        if (info.fColorTypeInfos[i].fColorType == ct) {
            return true;
        }
    }
    return false;
}

GrBackendFormat GrMtlCaps::onGetDefaultBackendFormat(GrColorType ct) const {
    MTLPixelFormat format = this->getFormatFromColorType(ct);
    if (!format) {
        return {};
    }
    return GrBackendFormat::MakeMtl(format);
}

GrBackendFormat GrMtlCaps::getBackendFormatFromCompressionType(
        SkTextureCompressionType compressionType) const {
    switch (compressionType) {
        case SkTextureCompressionType::kNone:
            return {};
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
            if (@available(macOS 11.0, *)) {
                if (this->isApple()) {
                    return GrBackendFormat::MakeMtl(MTLPixelFormatETC2_RGB8);
                } else {
                    return {};
                }
            } else {
                return {};
            }
        case SkTextureCompressionType::kBC1_RGB8_UNORM:
            // Metal only supports the RGBA BC1 variant (see following)
            return {};
        case SkTextureCompressionType::kBC1_RGBA8_UNORM:
#ifdef SK_BUILD_FOR_MAC
            if (this->isMac()) {
                return GrBackendFormat::MakeMtl(MTLPixelFormatBC1_RGBA);
            } else {
                return {};
            }
#else
            return {};
#endif

    }
    SK_ABORT("Invalid compression type");
}

skgpu::Swizzle GrMtlCaps::onGetReadSwizzle(const GrBackendFormat& format,
                                           GrColorType colorType) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlFormat != MTLPixelFormatInvalid);
    const auto& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fReadSwizzle;
        }
    }
    SkDEBUGFAILF("Illegal color type (%d) and format (%d) combination.", (int)colorType,
                 static_cast<int>(mtlFormat));
    return {};
}

skgpu::Swizzle GrMtlCaps::getWriteSwizzle(const GrBackendFormat& format,
                                          GrColorType colorType) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlFormat != MTLPixelFormatInvalid);
    const auto& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fWriteSwizzle;
        }
    }
    SkDEBUGFAILF("Illegal color type (%d) and format (%d) combination.", (int)colorType,
                 static_cast<int>(mtlFormat));
    return {};
}

uint64_t GrMtlCaps::computeFormatKey(const GrBackendFormat& format) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlFormat != MTLPixelFormatInvalid);
    // A MTLPixelFormat is an NSUInteger type which is documented to be 32 bits in 32 bit
    // applications and 64 bits in 64 bit applications. So it should fit in an uint64_t, but adding
    // the assert heere to make sure.
    static_assert(sizeof(MTLPixelFormat) <= sizeof(uint64_t));
    return (uint64_t)mtlFormat;
}

GrCaps::SupportedWrite GrMtlCaps::supportedWritePixelsColorType(
        GrColorType surfaceColorType, const GrBackendFormat& surfaceFormat,
        GrColorType srcColorType) const {
    // Metal requires the destination offset for copyFromTexture to be a multiple of the textures
    // pixels size.
    size_t offsetAlignment = GrColorTypeBytesPerPixel(surfaceColorType);

    const auto& info = this->getFormatInfo(GrBackendFormatAsMTLPixelFormat(surfaceFormat));
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == surfaceColorType) {
            return {surfaceColorType, offsetAlignment};
        }
    }
    return {GrColorType::kUnknown, 0};
}

GrCaps::SupportedRead GrMtlCaps::onSupportedReadPixelsColorType(
        GrColorType srcColorType, const GrBackendFormat& srcBackendFormat,
        GrColorType dstColorType) const {
    SkTextureCompressionType compression = GrBackendFormatToCompressionType(srcBackendFormat);
    if (compression != SkTextureCompressionType::kNone) {
#ifdef SK_BUILD_FOR_IOS
        // Reading back to kRGB_888x doesn't work on Metal/iOS (skbug.com/9839)
        return { GrColorType::kUnknown, 0 };
#else
        return { SkTextureCompressionTypeIsOpaque(compression) ? GrColorType::kRGB_888x
                                                        : GrColorType::kRGBA_8888, 0 };
#endif
    }

    // Metal requires the destination offset for copyFromTexture to be a multiple of the textures
    // pixels size.
    size_t offsetAlignment = GrColorTypeBytesPerPixel(srcColorType);
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(srcBackendFormat);

    const auto& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return {srcColorType, offsetAlignment};
        }
    }
    return {GrColorType::kUnknown, 0};
}

/**
 * For Metal we want to cache the entire pipeline for reuse of draws. The Desc here holds all
 * the information needed to differentiate one pipeline from another.
 *
 * The GrProgramDesc contains all the information need to create the actual shaders for the
 * pipeline.
 *
 * For Metal we need to add to the GrProgramDesc to include the rest of the state on the
 * pipeline. This includes blending information and primitive type. The pipeline is immutable
 * so any remaining dynamic state is set via the MtlRenderCmdEncoder.
 */
GrProgramDesc GrMtlCaps::makeDesc(GrRenderTarget*, const GrProgramInfo& programInfo,
                                  ProgramDescOverrideFlags overrideFlags) const {
    SkASSERT(overrideFlags == ProgramDescOverrideFlags::kNone);
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, programInfo, *this);

    skgpu::KeyBuilder b(desc.key());

    // If ordering here is changed, update getStencilPixelFormat() below
    b.add32(programInfo.backendFormat().asMtlFormat());

    b.add32(programInfo.numSamples());

    b.add32(programInfo.needsStencil() ? this->preferredStencilFormat() : MTLPixelFormatInvalid);
    b.add32((uint32_t)programInfo.isStencilEnabled());
    // Stencil samples don't seem to be tracked in the MTLRenderPipeline

    programInfo.pipeline().genKey(&b, *this);

    b.add32(programInfo.primitiveTypeKey());

    b.flush();
    return desc;
}

MTLPixelFormat GrMtlCaps::getStencilPixelFormat(const GrProgramDesc& desc) {
    // Set up read buffer to point to platform-dependent part of the key
    SkReadBuffer readBuffer(desc.asKey() + desc.initialKeyLength()/sizeof(uint32_t),
                            desc.keyLength() - desc.initialKeyLength());
    // skip backend format
    readBuffer.readUInt();
    // skip raster samples
    readBuffer.readUInt();

    return (MTLPixelFormat) readBuffer.readUInt();
}

bool GrMtlCaps::renderTargetSupportsDiscardableMSAA(const GrMtlRenderTarget* rt) const {
    return rt->resolveAttachment() &&
           !rt->resolveAttachment()->framebufferOnly() &&
           (rt->numSamples() > 1 && this->preferDiscardableMSAAAttachment());
}

#if defined(GR_TEST_UTILS)
std::vector<GrTest::TestFormatColorTypeCombination> GrMtlCaps::getTestingCombinations() const {
    std::vector<GrTest::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,          GrBackendFormat::MakeMtl(MTLPixelFormatA8Unorm)         },
        { GrColorType::kAlpha_8,          GrBackendFormat::MakeMtl(MTLPixelFormatR8Unorm)         },
        { GrColorType::kBGR_565,          GrBackendFormat::MakeMtl(kMTLPixelFormatB5G6R5Unorm)    },
        { GrColorType::kABGR_4444,        GrBackendFormat::MakeMtl(kMTLPixelFormatABGR4Unorm)     },
        { GrColorType::kRGBA_8888,        GrBackendFormat::MakeMtl(MTLPixelFormatRGBA8Unorm)      },
        { GrColorType::kRGBA_8888_SRGB,   GrBackendFormat::MakeMtl(MTLPixelFormatRGBA8Unorm_sRGB) },
        { GrColorType::kRGB_888x,         GrBackendFormat::MakeMtl(MTLPixelFormatRGBA8Unorm)      },
        { GrColorType::kRGB_888x,         GrBackendFormat::MakeMtl(kMTLPixelFormatETC2_RGB8)      },
#ifdef SK_BUILD_FOR_MAC
        { GrColorType::kRGBA_8888,        GrBackendFormat::MakeMtl(MTLPixelFormatBC1_RGBA)        },
#endif
        { GrColorType::kRG_88,            GrBackendFormat::MakeMtl(MTLPixelFormatRG8Unorm)        },
        { GrColorType::kBGRA_8888,        GrBackendFormat::MakeMtl(MTLPixelFormatBGRA8Unorm)      },
        { GrColorType::kRGBA_1010102,     GrBackendFormat::MakeMtl(MTLPixelFormatRGB10A2Unorm)    },
        { GrColorType::kBGRA_1010102,     GrBackendFormat::MakeMtl(MTLPixelFormatBGR10A2Unorm)    },
        { GrColorType::kGray_8,           GrBackendFormat::MakeMtl(MTLPixelFormatR8Unorm)         },
        { GrColorType::kAlpha_F16,        GrBackendFormat::MakeMtl(MTLPixelFormatR16Float)        },
        { GrColorType::kRGBA_F16,         GrBackendFormat::MakeMtl(MTLPixelFormatRGBA16Float)     },
        { GrColorType::kRGBA_F16_Clamped, GrBackendFormat::MakeMtl(MTLPixelFormatRGBA16Float)     },
        { GrColorType::kAlpha_16,         GrBackendFormat::MakeMtl(MTLPixelFormatR16Unorm)        },
        { GrColorType::kRG_1616,          GrBackendFormat::MakeMtl(MTLPixelFormatRG16Unorm)       },
        { GrColorType::kRGBA_16161616,    GrBackendFormat::MakeMtl(MTLPixelFormatRGBA16Unorm)     },
        { GrColorType::kRG_F16,           GrBackendFormat::MakeMtl(MTLPixelFormatRG16Float)       },
    };

    return combos;
}
#endif

#ifdef SK_ENABLE_DUMP_GPU
#include "src/utils/SkJSONWriter.h"
void GrMtlCaps::onDumpJSON(SkJSONWriter* writer) const {

    // We are called by the base class, which has already called beginObject(). We choose to nest
    // all of our caps information in a named sub-object.
    writer->beginObject("Metal caps");

    writer->beginObject("Preferred Stencil Format");
    writer->appendS32("stencil bits", GrMtlFormatStencilBits(fPreferredStencilFormat));
    writer->appendS32("total bytes", skgpu::MtlFormatBytesPerBlock(fPreferredStencilFormat));
    writer->endObject();

    switch (fGPUFamily) {
        case GPUFamily::kMac:
            writer->appendNString("GPU Family", "Mac");
            break;
        case GPUFamily::kApple:
            writer->appendNString("GPU Family", "Apple");
            break;
        default:
            writer->appendNString("GPU Family", "unknown");
            break;
    }
    writer->appendS32("Family Group", fFamilyGroup);

    writer->beginArray("Sample Counts");
    for (int v : fSampleCounts) {
        writer->appendS32(nullptr, v);
    }
    writer->endArray();

    writer->endObject();
}
#else
void GrMtlCaps::onDumpJSON(SkJSONWriter* writer) const { }
#endif

GR_NORETAIN_END
