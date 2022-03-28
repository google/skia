/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlCaps.h"

#include "experimental/graphite/include/TextureInfo.h"
#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/GraphiteResourceKey.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::mtl {

Caps::Caps(const id<MTLDevice> device)
        : skgpu::Caps() {
    fShaderCaps = std::make_unique<SkSL::ShaderCaps>();

    this->initGPUFamily(device);
    this->initCaps(device);
    this->initShaderCaps();

    this->initFormatTable();

    // Metal-specific caps
}

// translates from older MTLFeatureSet interface to MTLGPUFamily interface
bool Caps::GetGPUFamilyFromFeatureSet(id<MTLDevice> device, GPUFamily* gpuFamily, int* group) {
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
    if (@available(iOS 12.0, *)) {
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
    if (@available(iOS 11.0, *)) {
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
    if (@available(iOS 10.0, *)) {
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

    // No supported GPU families were found
    return false;
}

bool Caps::GetGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group) {
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

void Caps::initGPUFamily(id<MTLDevice> device) {
    if (!GetGPUFamily(device, &fGPUFamily, &fFamilyGroup) &&
        !GetGPUFamilyFromFeatureSet(device, &fGPUFamily, &fFamilyGroup)) {
        // We don't know what this is, fall back to minimum defaults
#ifdef SK_BUILD_FOR_MAC
        fGPUFamily = GPUFamily::kMac;
        fFamilyGroup = 1;
#else
        fGPUFamily = GPUFamily::kApple;
        fFamilyGroup = 1;
#endif
    }
}

void Caps::initCaps(const id<MTLDevice> device) {
    if (this->isMac() || fFamilyGroup >= 3) {
        fMaxTextureSize = 16384;
    } else {
        fMaxTextureSize = 8192;
    }

    // We use constant address space for our uniform buffers which has various alignment
    // requirements for the offset when binding the buffer. On MacOS the offset must align to 256.
    // On iOS we must align to the max of the data type consumed by the vertex function or 4 bytes.
    // We can ignore the data type and just always use 16 bytes on iOS.
    if (this->isMac()) {
        fRequiredUniformBufferAlignment = 256;
    } else {
        fRequiredUniformBufferAlignment = 16;
    }

    if (@available(macOS 10.12, ios 14.0, *)) {
        fClampToBorderSupport = (this->isMac() || fFamilyGroup >= 7);
    } else {
        fClampToBorderSupport = false;
    }

    // Init sample counts. All devices support 1 (i.e. 0 in skia).
    fColorSampleCounts.push_back(1);
    if (@available(macOS 10.11, iOS 9.0, *)) {
        for (auto sampleCnt : {2, 4, 8}) {
            if ([device supportsTextureSampleCount:sampleCnt]) {
                fColorSampleCounts.push_back(sampleCnt);
            }
        }
    }
}

void Caps::initShaderCaps() {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // Setting this true with the assumption that this cap will eventually mean we support varying
    // precisions and not just via modifiers.
    shaderCaps->fUsesPrecisionModifiers = true;
    shaderCaps->fFlatInterpolationSupport = true;

    shaderCaps->fShaderDerivativeSupport = true;

    // TODO(skia:8270): Re-enable this once bug 8270 is fixed
#if 0
    if (this->isApple()) {
        shaderCaps->fFBFetchSupport = true;
        shaderCaps->fFBFetchNeedsCustomOutput = true; // ??
        shaderCaps->fFBFetchColorName = ""; // Somehow add [[color(0)]] to arguments to frag shader
    }
#endif

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fNonsquareMatrixSupport = true;
    shaderCaps->fInverseHyperbolicSupport = true;

    // Metal uses IEEE floats so assuming those values here.
    // TODO: add fHalfIs32Bits?
    shaderCaps->fFloatIs32Bits = true;
}

// These are all the valid MTLPixelFormats that we currently support in Skia.  They are roughly
// ordered from most frequently used to least to improve look up times in arrays.
static constexpr MTLPixelFormat kMtlFormats[] = {
    MTLPixelFormatRGBA8Unorm,
    MTLPixelFormatR8Unorm,
    MTLPixelFormatA8Unorm,
    MTLPixelFormatBGRA8Unorm,

    MTLPixelFormatStencil8,
    MTLPixelFormatDepth32Float,
    MTLPixelFormatDepth32Float_Stencil8,

    MTLPixelFormatInvalid,
};

void Caps::setColorType(SkColorType colorType, std::initializer_list<MTLPixelFormat> formats) {
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

size_t Caps::GetFormatIndex(MTLPixelFormat pixelFormat) {
    static_assert(SK_ARRAY_COUNT(kMtlFormats) == Caps::kNumMtlFormats,
                  "Size of kMtlFormats array must match static value in header");
    for (size_t i = 0; i < Caps::kNumMtlFormats; ++i) {
        if (kMtlFormats[i] == pixelFormat) {
            return i;
        }
    }
    return GetFormatIndex(MTLPixelFormatInvalid);
}

void Caps::initFormatTable() {
    FormatInfo* info;

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
            ctInfo.fColorType = kRGBA_8888_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
        // Format: RGBA8Unorm, Surface: kRGB_888x
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kRGB_888x_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            ctInfo.fReadSwizzle = skgpu::Swizzle::RGB1();
        }
    }

    // Format: R8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatR8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 3;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: R8Unorm, Surface: kR8_unorm
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kR8_unorm_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
        // Format: R8Unorm, Surface: kAlpha_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kAlpha_8_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
            ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
        }
        // Format: R8Unorm, Surface: kGray_8
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kGray_8_SkColorType;
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
            ctInfo.fColorType = kAlpha_8_SkColorType;
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
            ctInfo.fColorType = kBGRA_8888_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    /*
     * Non-color formats
     */

    // Format: Stencil8
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatStencil8)];
        info->fFlags = FormatInfo::kMSAA_Flag;
        info->fColorTypeInfoCount = 0;
    }

    // Format: Depth32Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatDepth32Float)];
        info->fFlags = FormatInfo::kMSAA_Flag;
        if (this->isMac() || fFamilyGroup >= 3) {
            info->fFlags |= FormatInfo::kResolve_Flag;
        }
        info->fColorTypeInfoCount = 0;
    }

    // Format: Depth32Float_Stencil8
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatDepth32Float_Stencil8)];
        info->fFlags = FormatInfo::kMSAA_Flag;
        if (this->isMac() || fFamilyGroup >= 3) {
            info->fFlags |= FormatInfo::kResolve_Flag;
        }
         info->fColorTypeInfoCount = 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Map SkColorTypes (used for creating SkSurfaces) to MTLPixelFormats. The order in which the
    // formats are passed into the setColorType function indicates the priority in selecting which
    // format we use for a given SkColorType.

    std::fill_n(fColorTypeToFormatTable, kSkColorTypeCnt, MTLPixelFormatInvalid);

    this->setColorType(kAlpha_8_SkColorType,          { MTLPixelFormatR8Unorm,
                                                        MTLPixelFormatA8Unorm });
    this->setColorType(kRGBA_8888_SkColorType,        { MTLPixelFormatRGBA8Unorm });
    this->setColorType(kRGB_888x_SkColorType,         { MTLPixelFormatRGBA8Unorm });
    this->setColorType(kBGRA_8888_SkColorType,        { MTLPixelFormatBGRA8Unorm });
    this->setColorType(kGray_8_SkColorType,           { MTLPixelFormatR8Unorm });
    this->setColorType(kR8_unorm_SkColorType,         { MTLPixelFormatR8Unorm });
}

skgpu::TextureInfo Caps::getDefaultSampledTextureInfo(SkColorType colorType,
                                                      uint32_t levelCount,
                                                      Protected,
                                                      Renderable renderable) const {
    MTLTextureUsage usage = MTLTextureUsageShaderRead;
    if (renderable == Renderable::kYes) {
        usage |= MTLTextureUsageRenderTarget;
    }

    TextureInfo info;
    info.fSampleCount = 1;
    info.fLevelCount = levelCount;
    info.fFormat = this->getFormatFromColorType(colorType);
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

skgpu::TextureInfo Caps::getDefaultMSAATextureInfo(SkColorType colorType,
                                                   uint32_t sampleCount,
                                                   Protected) const {
    MTLTextureUsage usage = MTLTextureUsageRenderTarget;

    TextureInfo info;
    info.fSampleCount = sampleCount;
    info.fLevelCount = 1;
    info.fFormat = this->getFormatFromColorType(colorType);
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

skgpu::TextureInfo Caps::getDefaultDepthStencilTextureInfo(Mask<DepthStencilFlags> depthStencilType,
                                                           uint32_t sampleCount,
                                                           Protected) const {
    TextureInfo info;
    info.fSampleCount = sampleCount;
    info.fLevelCount = 1;
    info.fFormat = DepthStencilFlagsToFormat(depthStencilType);
    info.fUsage = MTLTextureUsageRenderTarget;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

UniqueKey Caps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                        const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kGraphicsPipelineDomain = UniqueKey::GenerateDomain();
        SkSpan<const uint32_t> pipelineDescKey = pipelineDesc.asKey();
        UniqueKey::Builder builder(&pipelineKey, kGraphicsPipelineDomain,
                                   pipelineDescKey.size() + 1, "GraphicsPipeline");
        // add graphicspipelinedesc key
        for (unsigned int i = 0; i < pipelineDescKey.size(); ++i) {
            builder[i] = pipelineDescKey[i];
        }
        // add renderpassdesc key
        mtl::TextureInfo colorInfo, depthStencilInfo;
        renderPassDesc.fColorAttachment.fTextureInfo.getMtlTextureInfo(&colorInfo);
        renderPassDesc.fDepthStencilAttachment.fTextureInfo.getMtlTextureInfo(&depthStencilInfo);
        SkASSERT(colorInfo.fFormat < 65535 && depthStencilInfo.fFormat < 65535);
        uint32_t renderPassKey = colorInfo.fFormat << 16 | depthStencilInfo.fFormat;
        builder[pipelineDescKey.size()] = renderPassKey;
        builder.finish();
    }

    return pipelineKey;
}

bool Caps::onIsTexturable(const skgpu::TextureInfo& info) const {
    if (!(info.mtlTextureSpec().fUsage & MTLTextureUsageShaderRead)) {
        return false;
    }
    if (info.mtlTextureSpec().fFramebufferOnly) {
        return false;
    }
    return this->isTexturable((MTLPixelFormat)info.mtlTextureSpec().fFormat);
}

bool Caps::isTexturable(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTexturable_Flag && formatInfo.fFlags);
}

bool Caps::isRenderable(const skgpu::TextureInfo& info) const {
    return info.mtlTextureSpec().fUsage & MTLTextureUsageRenderTarget &&
    this->isRenderable((MTLPixelFormat)info.mtlTextureSpec().fFormat, info.numSamples());
}

bool Caps::isRenderable(MTLPixelFormat format, uint32_t sampleCount) const {
    return sampleCount <= this->maxRenderTargetSampleCount(format);
}

uint32_t Caps::maxRenderTargetSampleCount(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (!SkToBool(formatInfo.fFlags & FormatInfo::kRenderable_Flag)) {
        return 0;
    }
    if (SkToBool(formatInfo.fFlags & FormatInfo::kMSAA_Flag)) {
        return fColorSampleCounts[fColorSampleCounts.size() - 1];
    } else {
        return 1;
    }
}

bool Caps::onAreColorTypeAndTextureInfoCompatible(SkColorType ct,
                                                  const skgpu::TextureInfo& textureInfo) const {
    MTLPixelFormat mtlFormat = static_cast<MTLPixelFormat>(textureInfo.mtlTextureSpec().fFormat);

    const auto& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        if (info.fColorTypeInfos[i].fColorType == ct) {
            return true;
        }
    }
    return false;
}

size_t Caps::getTransferBufferAlignment(size_t bytesPerPixel) const {
    return std::max(bytesPerPixel, getMinBufferAlignment());
}

// There are only a few possible valid sample counts (1, 2, 4, 8, 16). So we can key on those 5
// options instead of the actual sample value.
uint32_t samples_to_key(uint32_t numSamples) {
    switch (numSamples) {
        case 1:
            return 0;
        case 2:
            return 1;
        case 4:
            return 2;
        case 8:
            return 3;
        case 16:
            return 4;
        default:
            SkUNREACHABLE;
    }
}

void Caps::buildKeyForTexture(SkISize dimensions,
                              const skgpu::TextureInfo& info,
                              ResourceType type,
                              Shareable shareable,
                              GraphiteResourceKey* key) const {
    const TextureSpec& mtlSpec = info.mtlTextureSpec();

    SkASSERT(!dimensions.isEmpty());

    // A MTLPixelFormat is an NSUInteger type which is documented to be 32 bits in 32 bit
    // applications and 64 bits in 64 bit applications. So it should fit in an uint64_t, but adding
    // the assert heere to make sure.
    static_assert(sizeof(MTLPixelFormat) <= sizeof(uint64_t));
    SkASSERT(mtlSpec.fFormat != MTLPixelFormatInvalid);
    uint64_t formatKey = static_cast<uint64_t>(mtlSpec.fFormat);

    uint32_t samplesKey = samples_to_key(info.numSamples());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = info.numMipLevels() > 1;
    Protected isProtected = info.isProtected();
    bool isFBOnly = mtlSpec.fFramebufferOnly;

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32.
    SkASSERT(samplesKey                         < (1u << 3));
    SkASSERT(static_cast<uint32_t>(isMipped)    < (1u << 1));
    SkASSERT(static_cast<uint32_t>(isProtected) < (1u << 1));
    SkASSERT(mtlSpec.fUsage                     < (1u << 5));
    SkASSERT(mtlSpec.fStorageMode               < (1u << 2));
    SkASSERT(static_cast<uint32_t>(isFBOnly)    < (1u << 1));

    // We need two uint32_ts for dimensions, 2 for format, and 1 for the rest of the key;
    static int kNum32DataCnt = 2 + 2 + 1;

    GraphiteResourceKey::Builder builder(key, type, kNum32DataCnt, shareable);

    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey & 0xFFFFFFFF;
    builder[3] = (formatKey >> 32) & 0xFFFFFFFF;
    builder[4] = (samplesKey                                  << 0) |
                 (static_cast<uint32_t>(isMipped)             << 3) |
                 (static_cast<uint32_t>(isProtected)          << 4) |
                 (static_cast<uint32_t>(mtlSpec.fUsage)       << 5) |
                 (static_cast<uint32_t>(mtlSpec.fStorageMode) << 10)|
                 (static_cast<uint32_t>(isFBOnly)             << 12);

}

} // namespace skgpu::mtl
