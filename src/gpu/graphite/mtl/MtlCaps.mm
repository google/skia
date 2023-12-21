/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlCaps.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtilsPriv.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

MtlCaps::MtlCaps(const id<MTLDevice> device, const ContextOptions& options)
        : Caps() {
    this->initGPUFamily(device);
    this->initCaps(device);
    this->initShaderCaps();

    this->initFormatTable();

    // Metal-specific MtlCaps

    this->finishInitialization(options);
}

// translates from older MTLFeatureSet interface to MTLGPUFamily interface
bool MtlCaps::GetGPUFamilyFromFeatureSet(id<MTLDevice> device, GPUFamily* gpuFamily, int* group) {
// MTLFeatureSet is deprecated for newer versions of the SDK
#if SKGPU_GRAPHITE_METAL_SDK_VERSION < 300

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

#endif // SKGPU_GRAPHITE_METAL_SDK_VERSION < 300

    // No supported GPU families were found
    return false;
}

bool MtlCaps::GetGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group) {
#if SKGPU_GRAPHITE_METAL_SDK_VERSION >= 220
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        // Apple Silicon
#if SKGPU_GRAPHITE_METAL_SDK_VERSION >= 230
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
#if GR_METAL_SDK_VERSION >= 300
        // TODO: replace with Metal 3 definitions
        SkASSERT([device supportsFamily:MTLGPUFamilyMac2]);
        *gpuFamily = GPUFamily::kMac;
        *group = 2;
        return true;
#else
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
#endif
    }
#endif

    // No supported GPU families were found
    return false;
}

void MtlCaps::initGPUFamily(id<MTLDevice> device) {
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        if (GetGPUFamily(device, &fGPUFamily, &fFamilyGroup)) {
            return;
        }
    } else {
        if (GetGPUFamilyFromFeatureSet(device, &fGPUFamily, &fFamilyGroup)) {
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

void MtlCaps::initCaps(const id<MTLDevice> device) {
#if defined(GRAPHITE_TEST_UTILS)
    this->setDeviceName([[device name] UTF8String]);
#endif

    if (this->isMac() || fFamilyGroup >= 3) {
        fMaxTextureSize = 16384;
    } else {
        fMaxTextureSize = 8192;
    }

    // We use constant address space for our uniform buffers which has various alignment
    // requirements for the offset when binding the buffer. On MacOS Intel the offset must align
    // to 256. On iOS or Apple Silicon we must align to the max of the data type consumed by the
    // vertex function or 4 bytes, or we can ignore the data type and just use 16 bytes.
    //
    // On Mac, all copies must be aligned to at least 4 bytes; on iOS there is no alignment.
    if (this->isMac()) {
        fRequiredUniformBufferAlignment = 256;
        fRequiredTransferBufferAlignment = 4;
    } else {
        fRequiredUniformBufferAlignment = 16;
        fRequiredTransferBufferAlignment = 1;
    }

    fResourceBindingReqs.fUniformBufferLayout = Layout::kMetal;
    fResourceBindingReqs.fStorageBufferLayout = Layout::kMetal;
    fResourceBindingReqs.fDistinctIndexRanges = true;

    // Metal does not distinguish between uniform and storage buffers.
    fRequiredStorageBufferAlignment = fRequiredUniformBufferAlignment;

    fStorageBufferSupport = true;
    fStorageBufferPreferred = true;

    fComputeSupport = true;

    if (@available(macOS 10.12, iOS 14.0, tvOS 14.0, *)) {
        fClampToBorderSupport = (this->isMac() || fFamilyGroup >= 7);
    } else {
        fClampToBorderSupport = false;
    }

    // Init sample counts. All devices support 1 (i.e. 0 in skia).
    fColorSampleCounts.push_back(1);
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        for (auto sampleCnt : {2, 4, 8}) {
            if ([device supportsTextureSampleCount:sampleCnt]) {
                fColorSampleCounts.push_back(sampleCnt);
            }
        }
    }
}

void MtlCaps::initShaderCaps() {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // Dual source blending requires Metal 1.2.
    if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
        shaderCaps->fDualSourceBlendingSupport = true;
    }

    // Setting this true with the assumption that this cap will eventually mean we support varying
    // precisions and not just via modifiers.
    shaderCaps->fUsesPrecisionModifiers = true;
    shaderCaps->fFlatInterpolationSupport = true;

    shaderCaps->fShaderDerivativeSupport = true;
    shaderCaps->fInfinitySupport = true;

    if (@available(macOS 11.0, *)) {
        if (this->isApple()) {
            shaderCaps->fFBFetchSupport = true;
            shaderCaps->fFBFetchColorName = "sk_LastFragColor";
        }
    }

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fNonsquareMatrixSupport = true;
    shaderCaps->fInverseHyperbolicSupport = true;

    // Metal uses IEEE floats so assuming those values here.
    shaderCaps->fFloatIs32Bits = true;
}

// Define this so we can use it to initialize arrays and work around
// the fact that these pixel formats are not always available.
#define kMTLPixelFormatB5G6R5Unorm MTLPixelFormat(40)
#define kMTLPixelFormatABGR4Unorm MTLPixelFormat(42)

// These are all the valid MTLPixelFormats that we currently support in Skia.  They are roughly
// ordered from most frequently used to least to improve look up times in arrays.
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
    // MTLPixelFormatBGR10A2Unorm
    kMTLPixelFormatABGR4Unorm,
    MTLPixelFormatRGBA8Unorm_sRGB,
    MTLPixelFormatR16Unorm,
    MTLPixelFormatRG16Unorm,
    // kMTLPixelFormatETC2_RGB8
    // MTLPixelFormatBC1_RGBA
    MTLPixelFormatRGBA16Unorm,
    MTLPixelFormatRG16Float,

    MTLPixelFormatStencil8,
    MTLPixelFormatDepth32Float,
    MTLPixelFormatDepth32Float_Stencil8,

    MTLPixelFormatInvalid,
};

void MtlCaps::setColorType(SkColorType colorType, std::initializer_list<MTLPixelFormat> formats) {
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

size_t MtlCaps::GetFormatIndex(MTLPixelFormat pixelFormat) {
    static_assert(std::size(kMtlFormats) == MtlCaps::kNumMtlFormats,
                  "Size of kMtlFormats array must match static value in header");
    for (size_t i = 0; i < MtlCaps::kNumMtlFormats; ++i) {
        if (kMtlFormats[i] == pixelFormat) {
            return i;
        }
    }
    return GetFormatIndex(MTLPixelFormatInvalid);
}

void MtlCaps::initFormatTable() {
    FormatInfo* info;

    if (@available(macOS 11.0, iOS 8.0, tvOS 9.0, *)) {
        if (this->isApple()) {
            SkASSERT(kMTLPixelFormatB5G6R5Unorm == MTLPixelFormatB5G6R5Unorm);
            SkASSERT(kMTLPixelFormatABGR4Unorm == MTLPixelFormatABGR4Unorm);
        }
    }

    // Format: RGBA8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 2;
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
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
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
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
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
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
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: BGRA8Unorm, Surface: kBGRA_8888
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kBGRA_8888_SkColorType;
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
                info->fColorTypeInfos =
                        std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
                int ctIdx = 0;
                // Format: B5G6R5Unorm, Surface: kBGR_565
                {
                    auto& ctInfo = info->fColorTypeInfos[ctIdx++];
                    ctInfo.fColorType = kRGB_565_SkColorType;
                    ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag |
                                    ColorTypeInfo::kRenderable_Flag;
                }
            }

            // Format: ABGR4Unorm
            {
                info = &fFormatTable[GetFormatIndex(MTLPixelFormatABGR4Unorm)];
                info->fFlags = FormatInfo::kAllFlags;
                info->fColorTypeInfoCount = 1;
                info->fColorTypeInfos =
                        std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
                int ctIdx = 0;
                // Format: ABGR4Unorm, Surface: kABGR_4444
                {
                    auto& ctInfo = info->fColorTypeInfos[ctIdx++];
                    ctInfo.fColorType = kARGB_4444_SkColorType;
                    ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag |
                                    ColorTypeInfo::kRenderable_Flag;
                }
            }
        }
    }

    // Format: RGBA8Unorm_sRGB
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA8Unorm_sRGB)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RGBA8Unorm_sRGB, Surface: kSRGBA_8888
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kSRGBA_8888_SkColorType;
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
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RGB10A2Unorm, Surface: kRGBA_1010102
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kRGBA_1010102_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: RGBA16Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 2;
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RGBA16Float, Surface: RGBA_F16
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kRGBA_F16_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
        // Format: RGBA16Float, Surface: RGBA_F16Norm
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kRGBA_F16Norm_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: R16Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatR16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: R16Float, Surface: kA16_float
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kA16_float_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
            ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
        }
    }

    // Format: RG8Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RG8Unorm, Surface: kR8G8_unorm
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kR8G8_unorm_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: RGBA16Unorm
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRGBA16Unorm)];
        if (this->isMac()) {
            info->fFlags = FormatInfo::kAllFlags;
        } else {
            info->fFlags = FormatInfo::kTexturable_Flag | FormatInfo::kRenderable_Flag;
        }
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RGBA16Unorm, Surface: kR16G16B16A16_unorm
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kR16G16B16A16_unorm_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: RG16Float
    {
        info = &fFormatTable[GetFormatIndex(MTLPixelFormatRG16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RG16Float, Surface: kR16G16_float
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kR16G16_float_SkColorType;
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
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: R16Unorm, Surface: kA16_unorm
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kA16_unorm_SkColorType;
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
        info->fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info->fColorTypeInfoCount);
        int ctIdx = 0;
        // Format: RG16Unorm, Surface: kR16G16_unorm
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kR16G16_unorm_SkColorType;
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
    if (@available(macOS 11.0, iOS 8.0, tvOS 9.0, *)) {
        if (this->isApple()) {
            this->setColorType(kRGB_565_SkColorType,   {MTLPixelFormatB5G6R5Unorm});
            this->setColorType(kARGB_4444_SkColorType, { MTLPixelFormatABGR4Unorm });
        }
    }

    this->setColorType(kRGBA_8888_SkColorType,        { MTLPixelFormatRGBA8Unorm });
    this->setColorType(kRGB_888x_SkColorType,         { MTLPixelFormatRGBA8Unorm });
    this->setColorType(kBGRA_8888_SkColorType,        { MTLPixelFormatBGRA8Unorm });
    this->setColorType(kRGBA_1010102_SkColorType,     { MTLPixelFormatRGB10A2Unorm });
    // kBGRA_1010102_SkColorType
    // kRGB_101010x_SkColorType
    // kBGR_101010x_SkColorType
    // kBGR_101010x_XR_SkColorType
    this->setColorType(kGray_8_SkColorType,           { MTLPixelFormatR8Unorm });
    this->setColorType(kRGBA_F16Norm_SkColorType,     { MTLPixelFormatRGBA16Float });
    this->setColorType(kRGBA_F16_SkColorType,         { MTLPixelFormatRGBA16Float });
    // kRGBA_F32_SkColorType
    this->setColorType(kR8G8_unorm_SkColorType,       { MTLPixelFormatRG8Unorm });
    this->setColorType(kA16_float_SkColorType,        { MTLPixelFormatR16Float });
    this->setColorType(kR16G16_float_SkColorType,     { MTLPixelFormatRG16Float });
    this->setColorType(kA16_unorm_SkColorType,        { MTLPixelFormatR16Unorm });
    this->setColorType(kR16G16_unorm_SkColorType,     { MTLPixelFormatRG16Unorm });
    this->setColorType(kR16G16B16A16_unorm_SkColorType,{ MTLPixelFormatRGBA16Unorm });
    this->setColorType(kSRGBA_8888_SkColorType,       { MTLPixelFormatRGBA8Unorm_sRGB });
    this->setColorType(kR8_unorm_SkColorType,         { MTLPixelFormatR8Unorm });

}

TextureInfo MtlCaps::getDefaultSampledTextureInfo(SkColorType colorType,
                                                  Mipmapped mipmapped,
                                                  Protected,
                                                  Renderable renderable) const {
    MTLTextureUsage usage = MTLTextureUsageShaderRead;
    if (renderable == Renderable::kYes) {
        usage |= MTLTextureUsageRenderTarget;
    }

    MtlPixelFormat format = this->getFormatFromColorType(colorType);
    if (format == MTLPixelFormatInvalid) {
        return {};
    }

    MtlTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fFormat = format;
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

TextureInfo MtlCaps::getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                                  Mipmapped mipmapped) const {
    MtlTextureInfo info;
    if (!textureInfo.getMtlTextureInfo(&info)) {
        return {};
    }

    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fUsage = MTLTextureUsageShaderRead;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

MTLStorageMode MtlCaps::getDefaultMSAAStorageMode(Discardable discardable) const {
    // Try to use memoryless if it's available (only on new Apple silicon)
    if (discardable == Discardable::kYes && this->isApple()) {
        if (@available(macOS 11.0, iOS 10.0, tvOS 10.0, *)) {
            return MTLStorageModeMemoryless;
        }
    }
    // If it's not discardable or not available, private is the best option
    return MTLStorageModePrivate;
}

TextureInfo MtlCaps::getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                               Discardable discardable) const {
    if (fDefaultMSAASamples <= 1) {
        return {};
    }
    const MtlTextureSpec& singleSpec = singleSampledInfo.mtlTextureSpec();

    MTLTextureUsage usage = MTLTextureUsageRenderTarget;

    MtlTextureInfo info;
    info.fSampleCount = fDefaultMSAASamples;
    info.fMipmapped = Mipmapped::kNo;
    info.fFormat = singleSpec.fFormat;
    info.fUsage = usage;
    info.fStorageMode = this->getDefaultMSAAStorageMode(discardable);
    info.fFramebufferOnly = false;

    return info;
}

TextureInfo MtlCaps::getDefaultDepthStencilTextureInfo(
            SkEnumBitMask<DepthStencilFlags> depthStencilType,
            uint32_t sampleCount,
            Protected) const {
    MtlTextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped = Mipmapped::kNo;
    info.fFormat = MtlDepthStencilFlagsToFormat(depthStencilType);
    info.fUsage = MTLTextureUsageRenderTarget;
    info.fStorageMode = this->getDefaultMSAAStorageMode(Discardable::kYes);
    info.fFramebufferOnly = false;

    return info;
}

TextureInfo MtlCaps::getDefaultStorageTextureInfo(SkColorType colorType) const {
    // Storage textures are currently always sampleable from a shader.
    MTLPixelFormat format = static_cast<MTLPixelFormat>(this->getFormatFromColorType(colorType));
    if (format == MTLPixelFormatInvalid) {
        return {};
    }

    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (!SkToBool(FormatInfo::kStorage_Flag & formatInfo.fFlags)) {
        return {};
    }

    MTLTextureUsage usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
    MtlTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = Mipmapped::kNo;
    info.fFormat = format;
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

const Caps::ColorTypeInfo* MtlCaps::getColorTypeInfo(
        SkColorType ct, const TextureInfo& textureInfo) const {
    MTLPixelFormat mtlFormat = static_cast<MTLPixelFormat>(textureInfo.mtlTextureSpec().fFormat);
    if (mtlFormat == MTLPixelFormatInvalid) {
        return nullptr;
    }

    const FormatInfo& info = this->getFormatInfo(mtlFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const ColorTypeInfo& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == ct) {
            return &ctInfo;
        }
    }

    return nullptr;
}

UniqueKey MtlCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                           const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kGraphicsPipelineDomain = UniqueKey::GenerateDomain();
        // 5 uint32_t's (render step id, paint id, uint64 renderpass desc, uint16 write swizzle key)
        UniqueKey::Builder builder(&pipelineKey, kGraphicsPipelineDomain, 5, "GraphicsPipeline");
        // add graphicspipelinedesc key
        builder[0] = pipelineDesc.renderStepID();
        builder[1] = pipelineDesc.paintParamsID().asUInt();

        // add renderpassdesc key
        uint64_t renderPassKey = this->getRenderPassDescKey(renderPassDesc);
        builder[2] = renderPassKey & 0xFFFFFFFF;
        builder[3] = (renderPassKey >> 32) & 0xFFFFFFFF;
        builder[4] = renderPassDesc.fWriteSwizzle.asKey();

        builder.finish();
    }

    return pipelineKey;
}

uint64_t MtlCaps::getRenderPassDescKey(const RenderPassDesc& renderPassDesc) const {
    MtlTextureInfo colorInfo, depthStencilInfo;
    renderPassDesc.fColorAttachment.fTextureInfo.getMtlTextureInfo(&colorInfo);
    renderPassDesc.fDepthStencilAttachment.fTextureInfo.getMtlTextureInfo(&depthStencilInfo);
    SkASSERT(colorInfo.fFormat < 65535 && depthStencilInfo.fFormat < 65535);
    uint32_t colorAttachmentKey = colorInfo.fFormat << 16 | colorInfo.fSampleCount;
    uint32_t dsAttachmentKey = depthStencilInfo.fFormat << 16 | depthStencilInfo.fSampleCount;
    return (((uint64_t) colorAttachmentKey) << 32) | dsAttachmentKey;
}

UniqueKey MtlCaps::makeComputePipelineKey(const ComputePipelineDesc& pipelineDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kComputePipelineDomain = UniqueKey::GenerateDomain();
        // The key is made up of a single uint32_t corresponding to the compute step ID.
        UniqueKey::Builder builder(&pipelineKey, kComputePipelineDomain, 1, "ComputePipeline");
        builder[0] = pipelineDesc.computeStep()->uniqueID();

        // TODO(b/240615224): The local work group size should factor into the key on platforms
        // that don't support specialization constants and require the workgroup/threadgroup size to
        // be specified in the shader text (D3D12, Vulkan 1.0, and OpenGL).

        builder.finish();
    }
    return pipelineKey;
}

uint32_t MtlCaps::channelMask(const TextureInfo& info) const {
    return skgpu::MtlFormatChannels((MTLPixelFormat)info.mtlTextureSpec().fFormat);
}

bool MtlCaps::onIsTexturable(const TextureInfo& info) const {
    if (!info.isValid()) {
        return false;
    }
    if (!(info.mtlTextureSpec().fUsage & MTLTextureUsageShaderRead)) {
        return false;
    }
    if (info.mtlTextureSpec().fFramebufferOnly) {
        return false;
    }
    return this->isTexturable((MTLPixelFormat)info.mtlTextureSpec().fFormat);
}

bool MtlCaps::isTexturable(MTLPixelFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTexturable_Flag & formatInfo.fFlags);
}

bool MtlCaps::isRenderable(const TextureInfo& info) const {
    return info.isValid() &&
           (info.mtlTextureSpec().fUsage & MTLTextureUsageRenderTarget) &&
           this->isRenderable((MTLPixelFormat)info.mtlTextureSpec().fFormat, info.numSamples());
}

bool MtlCaps::isRenderable(MTLPixelFormat format, uint32_t sampleCount) const {
    return sampleCount <= this->maxRenderTargetSampleCount(format);
}

bool MtlCaps::isStorage(const TextureInfo& info) const {
    if (!info.isValid()) {
        return false;
    }
    if (!(info.mtlTextureSpec().fUsage & MTLTextureUsageShaderWrite)) {
        return false;
    }
    if (info.mtlTextureSpec().fFramebufferOnly) {
        return false;
    }
    const FormatInfo& formatInfo =
            this->getFormatInfo((MTLPixelFormat)info.mtlTextureSpec().fFormat);
    return info.numSamples() == 1 && SkToBool(FormatInfo::kStorage_Flag & formatInfo.fFlags);
}

uint32_t MtlCaps::maxRenderTargetSampleCount(MTLPixelFormat format) const {
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

bool MtlCaps::supportsWritePixels(const TextureInfo& texInfo) const {
    MtlTextureInfo mtlInfo;
    texInfo.getMtlTextureInfo(&mtlInfo);
    if (mtlInfo.fFramebufferOnly) {
        return false;
    }

    if (texInfo.numSamples() > 1) {
        return false;
    }

    return true;
}

bool MtlCaps::supportsReadPixels(const TextureInfo& texInfo) const {
    MtlTextureInfo mtlInfo;
    texInfo.getMtlTextureInfo(&mtlInfo);
    if (mtlInfo.fFramebufferOnly) {
        return false;
    }

    // We disallow reading back directly from compressed textures.
    if (MtlFormatIsCompressed((MTLPixelFormat)mtlInfo.fFormat)) {
        return false;
    }

    if (texInfo.numSamples() > 1) {
        return false;
    }

    return true;
}

std::pair<SkColorType, bool /*isRGBFormat*/> MtlCaps::supportedWritePixelsColorType(
        SkColorType dstColorType,
        const TextureInfo& dstTextureInfo,
        SkColorType srcColorType) const {
    MtlTextureInfo mtlInfo;
    dstTextureInfo.getMtlTextureInfo(&mtlInfo);

    const FormatInfo& info = this->getFormatInfo((MTLPixelFormat)mtlInfo.fFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == dstColorType) {
            return {dstColorType, false};
        }
    }
    return {kUnknown_SkColorType, false};
}

std::pair<SkColorType, bool /*isRGBFormat*/> MtlCaps::supportedReadPixelsColorType(
        SkColorType srcColorType,
        const TextureInfo& srcTextureInfo,
        SkColorType dstColorType) const {
    MtlTextureInfo mtlInfo;
    srcTextureInfo.getMtlTextureInfo(&mtlInfo);

    // TODO: handle compressed formats
    if (MtlFormatIsCompressed((MTLPixelFormat)mtlInfo.fFormat)) {
        SkASSERT(this->isTexturable((MTLPixelFormat)mtlInfo.fFormat));
        return {kUnknown_SkColorType, false};
    }

    const FormatInfo& info = this->getFormatInfo((MTLPixelFormat)mtlInfo.fFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return {srcColorType, false};
        }
    }
    return {kUnknown_SkColorType, false};
}

void MtlCaps::buildKeyForTexture(SkISize dimensions,
                                 const TextureInfo& info,
                                 ResourceType type,
                                 Shareable shareable,
                                 GraphiteResourceKey* key) const {
    const MtlTextureSpec& mtlSpec = info.mtlTextureSpec();

    SkASSERT(!dimensions.isEmpty());

    // A MTLPixelFormat is an NSUInteger type which is documented to be 32 bits in 32 bit
    // applications and 64 bits in 64 bit applications. So it should fit in an uint64_t, but adding
    // the assert heere to make sure.
    static_assert(sizeof(MTLPixelFormat) <= sizeof(uint64_t));
    SkASSERT(mtlSpec.fFormat != MTLPixelFormatInvalid);
    uint64_t formatKey = static_cast<uint64_t>(mtlSpec.fFormat);

    uint32_t samplesKey = SamplesToKey(info.numSamples());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = info.mipmapped() == Mipmapped::kYes;
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

} // namespace skgpu::graphite
