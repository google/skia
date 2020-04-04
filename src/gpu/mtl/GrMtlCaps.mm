/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlCaps.h"

#include "include/core/SkRect.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/gpu/GrProcessor.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
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

    this->finishInitialization(contextOptions);
}

void GrMtlCaps::initFeatureSet(MTLFeatureSet featureSet) {
    // Mac OSX
#ifdef SK_BUILD_FOR_MAC
    if (@available(macOS 10.12, *)) {
        if (MTLFeatureSet_OSX_GPUFamily1_v2 == featureSet) {
            fPlatform = Platform::kMac;
            fFamilyGroup = 1;
            fVersion = 2;
            return;
        }
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
    if (@available(iOS 10.0, *)) {
        if (MTLFeatureSet_iOS_GPUFamily3_v2 == featureSet) {
            fPlatform = Platform::kIOS;
            fFamilyGroup = 3;
            fVersion = 2;
            return;
        }
    }
    if (@available(iOS 9.0, *)) {
        if (MTLFeatureSet_iOS_GPUFamily3_v1 == featureSet) {
            fPlatform = Platform::kIOS;
            fFamilyGroup = 3;
            fVersion = 1;
            return;
        }
    }

    // iOS Family group 2
    if (@available(iOS 10.0, *)) {
        if (MTLFeatureSet_iOS_GPUFamily2_v3 == featureSet) {
            fPlatform = Platform::kIOS;
            fFamilyGroup = 2;
            fVersion = 3;
            return;
        }
    }
    if (@available(iOS 9.0, *)) {
        if (MTLFeatureSet_iOS_GPUFamily2_v2 == featureSet) {
            fPlatform = Platform::kIOS;
            fFamilyGroup = 2;
            fVersion = 2;
            return;
        }
    }
    if (MTLFeatureSet_iOS_GPUFamily2_v1 == featureSet) {
        fPlatform = Platform::kIOS;
        fFamilyGroup = 2;
        fVersion = 1;
        return;
    }

    // iOS Family group 1
    if (@available(iOS 10.0, *)) {
        if (MTLFeatureSet_iOS_GPUFamily1_v3 == featureSet) {
            fPlatform = Platform::kIOS;
            fFamilyGroup = 1;
            fVersion = 3;
            return;
        }
    }
    if (@available(iOS 9.0, *)) {
        if (MTLFeatureSet_iOS_GPUFamily1_v2 == featureSet) {
            fPlatform = Platform::kIOS;
            fFamilyGroup = 1;
            fVersion = 2;
            return;
        }
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

bool GrMtlCaps::canCopyAsBlit(GrSurface* dst, int dstSampleCount,
                              GrSurface* src, int srcSampleCount,
                              const SkIRect& srcRect, const SkIPoint& dstPoint,
                              bool areDstSrcSameObj) const {
    id<MTLTexture> dstTex = GrGetMTLTextureFromSurface(dst);
    id<MTLTexture> srcTex = GrGetMTLTextureFromSurface(src);
    if (srcTex.framebufferOnly || dstTex.framebufferOnly) {
        return false;
    }

    MTLPixelFormat dstFormat = dstTex.pixelFormat;
    MTLPixelFormat srcFormat = srcTex.pixelFormat;

    return this->canCopyAsBlit(dstFormat, dstSampleCount, srcFormat, srcSampleCount,
                               srcRect, dstPoint, areDstSrcSameObj);
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

    // TODO: need some way to detect whether the proxy is framebufferOnly

    return this->canCopyAsBlit(GrBackendFormatAsMTLPixelFormat(dst->backendFormat()), dstSampleCnt,
                               GrBackendFormatAsMTLPixelFormat(src->backendFormat()), srcSampleCnt,
                               srcRect, dstPoint, dst == src);
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
    if (@available(iOS 9.0, *)) {
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

    fMipMapSupport = true;   // always available in Metal
    fNPOTTextureTileSupport = true;  // always available in Metal

    fReuseScratchTextures = true; // Assuming this okay

    fTransferBufferSupport = true;

    fTextureBarrierSupport = false; // Need to figure out if we can do this

    fSampleLocationsSupport = false;
    fMultisampleDisableSupport = false;

    if (@available(macOS 10.11, iOS 9.0, *)) {
        if (this->isMac() || 3 == fFamilyGroup) {
            fInstanceAttribSupport = true;
        }
    }

    fMixedSamplesSupport = false;
    fGpuTracingSupport = false;

    fFenceSyncSupport = true;
    bool supportsMTLEvent = false;
    if (@available(macOS 10.14, iOS 12.0, *)) {
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

bool GrMtlCaps::isFormatCompressed(const GrBackendFormat& format,
                                  SkImage::CompressionType* compressionType) const {
#ifdef SK_BUILD_FOR_MAC
    return false;
#else
    SkImage::CompressionType dummyType;
    SkImage::CompressionType* compressionTypePtr = compressionType ? compressionType : &dummyType;

    switch (GrBackendFormatAsMTLPixelFormat(format)) {
        case MTLPixelFormatETC2_RGB8:
            // ETC2 uses the same compression layout as ETC1
            *compressionTypePtr = SkImage::kETC1_CompressionType;
            return true;
        default:
            return false;
    }
#endif
}

bool GrMtlCaps::isFormatTexturableAndUploadable(GrColorType ct,
                                                const GrBackendFormat& format) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);

    uint32_t ctFlags = this->getFormatInfo(mtlFormat).colorTypeFlags(ct);
    return this->isFormatTexturable(mtlFormat) &&
           SkToBool(ctFlags & ColorTypeInfo::kUploadData_Flag);
}

bool GrMtlCaps::isFormatTexturable(const GrBackendFormat& format) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    return this->isFormatTexturable(mtlFormat);
}

bool GrMtlCaps::isFormatTexturable(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTexturable_Flag && formatInfo.fFlags);
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
        return fSampleCounts[fSampleCounts.count() - 1];
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
    requestedCount = SkTMax(requestedCount, 1);
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (!(formatInfo.fFlags & FormatInfo::kRenderable_Flag)) {
        return 0;
    }
    if (formatInfo.fFlags & FormatInfo::kMSAA_Flag) {
        int count = fSampleCounts.count();
        for (int i = 0; i < count; ++i) {
            if (fSampleCounts[i] >= requestedCount) {
                return fSampleCounts[i];
            }
        }
    }
    return 1 == requestedCount ? 1 : 0;
}

size_t GrMtlCaps::bytesPerPixel(const GrBackendFormat& format) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    return this->bytesPerPixel(mtlFormat);
}

size_t GrMtlCaps::bytesPerPixel(MTLPixelFormat format) const {
    return this->getFormatInfo(format).fBytesPerPixel;
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

    if (@available(macOS 10.12, iOS 11.0, *)) {
        shaderCaps->fDualSourceBlendingSupport = true;
    } else {
        shaderCaps->fDualSourceBlendingSupport = false;
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
    MTLPixelFormatRGBA8Unorm,
    MTLPixelFormatR8Unorm,
    MTLPixelFormatA8Unorm,
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
    MTLPixelFormatRGBA8Unorm_sRGB,
    MTLPixelFormatR16Unorm,
    MTLPixelFormatRG16Unorm,
#ifdef SK_BUILD_FOR_IOS
    MTLPixelFormatETC2_RGB8,
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
    static_assert(SK_ARRAY_COUNT(kMtlFormats) == GrMtlCaps::kNumMtlFormats,
                  "Size of kMtlFormats array must match static value in header");
    for (size_t i = 0; i < GrMtlCaps::kNumMtlFormats; ++i) {
        if (kMtlFormats[i] == pixelFormat) {
            return i;
        }
    }
    SK_ABORT("Invalid MTLPixelFormat");
}

void GrMtlCaps::initFormatTable() {
    FormatInfo* info;

    // Format: R8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatR8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fBytesPerPixel = 1;
        info->fColorTypeInfoCount = 2;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: R8Unorm, Surface: kAlpha_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kAlpha_8;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fTextureSwizzle = GrSwizzle::RRRR();
            ctInfo.fOutputSwizzle = GrSwizzle::AAAA();
        }
        // Format: R8Unorm, Surface: kGray_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kGray_8;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            ctInfo.fTextureSwizzle = GrSwizzle("rrr1");
        }
    }

    // Format: A8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatA8Unorm)];
        info->fFlags = FormatInfo::kTexturable_Flag;
        info->fBytesPerPixel = 1;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: A8Unorm, Surface: kAlpha_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kAlpha_8;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fTextureSwizzle = GrSwizzle::AAAA();
        }
    }

#ifdef SK_BUILD_FOR_IOS
    // Format: B5G6R5Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatB5G6R5Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fBytesPerPixel = 2;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: B5G6R5Unorm, Surface: kBGR_565
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kBGR_565;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: ABGR4Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatABGR4Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fBytesPerPixel = 2;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: ABGR4Unorm, Surface: kABGR_4444
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kABGR_4444;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }
#endif

    // Format: RGBA8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fBytesPerPixel = 4;
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
            ctInfo.fTextureSwizzle = GrSwizzle::RGB1();
        }
    }

    // Format: RG8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG8Unorm)];
        info->fFlags = FormatInfo::kTexturable_Flag;
        info->fBytesPerPixel = 2;
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
        info->fBytesPerPixel = 4;
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
        info->fBytesPerPixel = 4;
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
        info->fBytesPerPixel = 4;
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

    // Format: R16Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatR16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fBytesPerPixel = 2;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: R16Float, Surface: kAlpha_F16
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kAlpha_F16;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fTextureSwizzle = GrSwizzle::RRRR();
            ctInfo.fOutputSwizzle = GrSwizzle::AAAA();
        }
    }

    // Format: RGBA16Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fBytesPerPixel = 8;
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
        info->fBytesPerPixel = 2;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: R16Unorm, Surface: kAlpha_16
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = GrColorType::kAlpha_16;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fTextureSwizzle = GrSwizzle::RRRR();
            ctInfo.fOutputSwizzle = GrSwizzle::AAAA();
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
        info->fBytesPerPixel = 4;
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

#ifdef SK_BUILD_FOR_IOS
    // ETC2_RGB8
    info = &fFormatTable[GetFormatIndex(MTLPixelFormatETC2_RGB8)];
    info->fFlags = FormatInfo::kTexturable_Flag;
    // NO supported colorTypes
#endif

    // Format: RGBA16Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA16Unorm)];
        if (this->isMac()) {
            info->fFlags = FormatInfo::kAllFlags;
        } else {
            info->fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kRenderable_Flag;
        }
        info->fBytesPerPixel = 8;
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
        info->fBytesPerPixel = 4;
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

    this->setColorType(GrColorType::kAlpha_8,          { MTLPixelFormatR8Unorm,
                                                         MTLPixelFormatA8Unorm });
#ifdef SK_BUILD_FOR_IOS
    this->setColorType(GrColorType::kBGR_565,          { MTLPixelFormatB5G6R5Unorm });
    this->setColorType(GrColorType::kABGR_4444,        { MTLPixelFormatABGR4Unorm });
#endif
    this->setColorType(GrColorType::kRGBA_8888,        { MTLPixelFormatRGBA8Unorm });
    this->setColorType(GrColorType::kRGBA_8888_SRGB,   { MTLPixelFormatRGBA8Unorm_sRGB });
    this->setColorType(GrColorType::kRGB_888x,         { MTLPixelFormatRGBA8Unorm });
    this->setColorType(GrColorType::kRG_88,            { MTLPixelFormatRG8Unorm });
    this->setColorType(GrColorType::kBGRA_8888,        { MTLPixelFormatBGRA8Unorm });
    this->setColorType(GrColorType::kRGBA_1010102,     { MTLPixelFormatRGB10A2Unorm });
    this->setColorType(GrColorType::kGray_8,           { MTLPixelFormatR8Unorm });
    this->setColorType(GrColorType::kAlpha_F16,        { MTLPixelFormatR16Float });
    this->setColorType(GrColorType::kRGBA_F16,         { MTLPixelFormatRGBA16Float });
    this->setColorType(GrColorType::kRGBA_F16_Clamped, { MTLPixelFormatRGBA16Float });
    this->setColorType(GrColorType::kAlpha_16,         { MTLPixelFormatR16Unorm });
    this->setColorType(GrColorType::kRG_1616,          { MTLPixelFormatRG16Unorm });
    this->setColorType(GrColorType::kRGBA_16161616,    { MTLPixelFormatRGBA16Unorm });
    this->setColorType(GrColorType::kRG_F16,           { MTLPixelFormatRG16Float });
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

static constexpr GrPixelConfig validate_sized_format(GrMTLPixelFormat grFormat, GrColorType ct) {
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
#ifdef SK_BUILD_FOR_IOS
            else if (MTLPixelFormatETC2_RGB8 == format) {
                return kRGB_ETC1_GrPixelConfig;
            }
#endif
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
        case GrColorType::kAlpha_16:
            if (MTLPixelFormatR16Unorm == format) {
                return kAlpha_16_GrPixelConfig;
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
        case GrColorType::kRGBA_F32:
        case GrColorType::kAlpha_8xxx:
        case GrColorType::kAlpha_F32xxx:
        case GrColorType::kGray_8xxx:
        case GrColorType::kRGB_888:
        case GrColorType::kR_8:
        case GrColorType::kR_16:
        case GrColorType::kR_F16:
        case GrColorType::kGray_F16:
            return kUnknown_GrPixelConfig;
    }
    SkUNREACHABLE;
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

GrPixelConfig GrMtlCaps::onGetConfigFromBackendFormat(const GrBackendFormat& format,
                                                      GrColorType ct) const {
    return validate_sized_format(GrBackendFormatAsMTLPixelFormat(format), ct);
}

GrColorType GrMtlCaps::getYUVAColorTypeFromBackendFormat(const GrBackendFormat& format,
                                                         bool isAlphaChannel) const {
    switch (GrBackendFormatAsMTLPixelFormat(format)) {
        case MTLPixelFormatA8Unorm:           // fall through
        case MTLPixelFormatR8Unorm:           return isAlphaChannel ? GrColorType::kAlpha_8
                                                                    : GrColorType::kGray_8;
        case MTLPixelFormatRG8Unorm:          return GrColorType::kRG_88;
        case MTLPixelFormatRGBA8Unorm:        return GrColorType::kRGBA_8888;
        case MTLPixelFormatBGRA8Unorm:        return GrColorType::kBGRA_8888;
        case MTLPixelFormatRGB10A2Unorm:      return GrColorType::kRGBA_1010102;
        case MTLPixelFormatR16Unorm:          return GrColorType::kAlpha_16;
        case MTLPixelFormatR16Float:          return GrColorType::kAlpha_F16;
        case MTLPixelFormatRG16Unorm:         return GrColorType::kRG_1616;
        case MTLPixelFormatRGBA16Unorm:       return GrColorType::kRGBA_16161616;
        case MTLPixelFormatRG16Float:         return GrColorType::kRG_F16;
        default:                              return GrColorType::kUnknown;
    }
}

GrBackendFormat GrMtlCaps::onGetDefaultBackendFormat(GrColorType ct,
                                                     GrRenderable renderable) const {
    MTLPixelFormat format = this->getFormatFromColorType(ct);
    if (!format) {
        return GrBackendFormat();
    }
    return GrBackendFormat::MakeMtl(format);
}

GrBackendFormat GrMtlCaps::getBackendFormatFromCompressionType(
        SkImage::CompressionType compressionType) const {
    switch (compressionType) {
        case SkImage::CompressionType::kNone:
            return {};
        case SkImage::CompressionType::kETC1:
#ifdef SK_BUILD_FOR_MAC
            return {};
#else
            return GrBackendFormat::MakeMtl(MTLPixelFormatETC2_RGB8);
#endif
    }
    SK_ABORT("Invalid compression type");
}

GrSwizzle GrMtlCaps::getTextureSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlFormat != MTLPixelFormatInvalid);
    const auto& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fTextureSwizzle;
        }
    }
    return GrSwizzle::RGBA();
}
GrSwizzle GrMtlCaps::getOutputSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    SkASSERT(mtlFormat != MTLPixelFormatInvalid);
    const auto& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fOutputSwizzle;
        }
    }
    return GrSwizzle::RGBA();
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
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(srcBackendFormat);

    // Metal requires the destination offset for copyFromTexture to be a multiple of the textures
    // pixels size.
    size_t offsetAlignment = GrColorTypeBytesPerPixel(srcColorType);

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
GrProgramDesc GrMtlCaps::makeDesc(const GrRenderTarget* rt,
                                  const GrProgramInfo& programInfo) const {

    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, rt, programInfo, *this)) {
        SkASSERT(!desc.isValid());
        return desc;
    }

    GrProcessorKeyBuilder b(&desc.key());

    b.add32(programInfo.backendFormat().asMtlFormat());

    b.add32(programInfo.numRasterSamples());

#ifdef SK_DEBUG
    if (rt && programInfo.pipeline().isStencilEnabled()) {
        SkASSERT(rt->renderTargetPriv().getStencilAttachment());
    }
#endif

    b.add32(programInfo.pipeline().isStencilEnabled()
                                 ? this->preferredStencilFormat().fInternalFormat
                                 : MTLPixelFormatInvalid);
    b.add32((uint32_t)programInfo.pipeline().isStencilEnabled());
    // Stencil samples don't seem to be tracked in the MTLRenderPipeline

    programInfo.pipeline().genKey(&b, *this);

    b.add32((uint32_t)programInfo.primitiveType());

    return desc;
}


#if GR_TEST_UTILS
std::vector<GrCaps::TestFormatColorTypeCombination> GrMtlCaps::getTestingCombinations() const {
    std::vector<GrCaps::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,          GrBackendFormat::MakeMtl(MTLPixelFormatA8Unorm)         },
        { GrColorType::kAlpha_8,          GrBackendFormat::MakeMtl(MTLPixelFormatR8Unorm)         },
#ifdef SK_BUILD_FOR_IOS
        { GrColorType::kBGR_565,          GrBackendFormat::MakeMtl(MTLPixelFormatB5G6R5Unorm)     },
        { GrColorType::kABGR_4444,        GrBackendFormat::MakeMtl(MTLPixelFormatABGR4Unorm)      },
#endif
        { GrColorType::kRGBA_8888,        GrBackendFormat::MakeMtl(MTLPixelFormatRGBA8Unorm)      },
        { GrColorType::kRGBA_8888_SRGB,   GrBackendFormat::MakeMtl(MTLPixelFormatRGBA8Unorm_sRGB) },
        { GrColorType::kRGB_888x,         GrBackendFormat::MakeMtl(MTLPixelFormatRGBA8Unorm)      },
#ifdef SK_BUILD_FOR_IOS
        { GrColorType::kRGB_888x,         GrBackendFormat::MakeMtl(MTLPixelFormatETC2_RGB8)       },
#endif
        { GrColorType::kRG_88,            GrBackendFormat::MakeMtl(MTLPixelFormatRG8Unorm)        },
        { GrColorType::kBGRA_8888,        GrBackendFormat::MakeMtl(MTLPixelFormatBGRA8Unorm)      },
        { GrColorType::kRGBA_1010102,     GrBackendFormat::MakeMtl(MTLPixelFormatRGB10A2Unorm)    },
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
