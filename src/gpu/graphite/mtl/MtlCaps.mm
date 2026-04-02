/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlCaps.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/gpu/SwizzlePriv.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/mtl/MtlGraphicsPipeline.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::graphite {

MtlCaps::MtlCaps(const id<MTLDevice> device, const ContextOptions& options)
        : Caps()
        , fFormatSupport{} { // zero-initialize format support so everything defaults to unsupported
    // Metal-specific MtlCaps
    this->initGPUFamily(device);
    this->initCaps(device);
    this->initFormatTable(device);

    // Finish base class
    this->finishInitialization(options);
}

void MtlCaps::initGPUFamily(id<MTLDevice> device) {
    static_assert(SKGPU_GRAPHITE_METAL_SDK_VERSION >= 230);

    static constexpr MTLGPUFamily kFamiliesToCheck[] = {
        // Apple silicon shared between iOS and Mac
        MTLGPUFamilyApple9,
        MTLGPUFamilyApple8,
        MTLGPUFamilyApple7,
#ifdef SK_BUILD_FOR_IOS // This hardware is iOS exclusive
        MTLGPUFamilyApple6,
        MTLGPUFamilyApple5,
        MTLGPUFamilyApple4,
        MTLGPUFamilyApple3,
        MTLGPUFamilyApple2,
#endif
        // Legacy mac hardware
        MTLGPUFamilyMac2
    };

    for (MTLGPUFamily family : kFamiliesToCheck) {
        if ([device supportsFamily:family]) {
            fGPUFamily = family;
            return;
        }
    }

    // If we've reached here, we didn't find a supported family and nothing can be trusted.
    SKGPU_LOG_F("Unable to detect supported MTLGPUFamily");
}

void MtlCaps::initCaps(const id<MTLDevice> device) {
#if defined(GPU_TEST_UTILS)
    this->setDeviceName([[device name] UTF8String]);
#endif

    if (this->isMac() || fGPUFamily >= MTLGPUFamilyApple3) {
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

    fResourceBindingReqs.fBackendApi = BackendApi::kMetal;
    fResourceBindingReqs.fUniformBufferLayout = Layout::kMetal;
    fResourceBindingReqs.fStorageBufferLayout = Layout::kMetal;

    // Graphite/Metal does not group resources into different sets or bind groups at this time,
    // though ResourceBindingRequirements still expects valid assignments of these indices.
    // Assigning both to 0 conveys the usage of one single "set" for all resources.
    fResourceBindingReqs.fUniformsSetIdx = 0;
    fResourceBindingReqs.fTextureSamplerSetIdx = 0;

    fResourceBindingReqs.fComputeUsesDistinctIdxRangesForTextures = true;

    fResourceBindingReqs.fIntrinsicBufferBinding =
            MtlGraphicsPipeline::kIntrinsicUniformBufferIndex;
    fResourceBindingReqs.fCombinedUniformBufferBinding = MtlGraphicsPipeline::kCombinedUniformIndex;
    fResourceBindingReqs.fGradientBufferBinding = MtlGraphicsPipeline::kGradientBufferIndex;

    // Metal does not distinguish between uniform and storage buffers.
    fRequiredStorageBufferAlignment = fRequiredUniformBufferAlignment;

    fStorageBufferSupport = true;

    fComputeSupport = true;

    // See https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf, and what Dawn does at
    // https://crsrc.org/c/third_party/dawn/src/dawn/native/metal/PhysicalDeviceMTL.mm?q=maxInterStageShaderVariables
    if (this->isMac() || fGPUFamily >= MTLGPUFamilyApple4) {
        fMaxVaryings = 31;
    } else {
        fMaxVaryings = 15;
    }

    if (@available(macOS 10.12, iOS 14.0, tvOS 14.0, *)) {
        fClampToBorderSupport = (this->isMac() || fGPUFamily >= MTLGPUFamilyApple7);
    } else {
        fClampToBorderSupport = false;
    }

    const bool isIntel = [device.name containsString:@"Intel"];
    if (isIntel) {
        // All supported Intel Macs are of an old enough Intel generation that we can just assume
        // MSAA should be avoided instead of checking its generation.
        fAvoidMSAA = true;
    }

    // ShaderCaps
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // Dual source blending requires Metal 1.2, but our minimum requirements ensure 2.2
    shaderCaps->fDualSourceBlendingSupport = true;

    shaderCaps->fFlatInterpolationSupport = true;
    shaderCaps->fShaderDerivativeSupport = true;
    shaderCaps->fInfinitySupport = true;

    if (this->isApple()) {
        shaderCaps->fFBFetchSupport = true;
        shaderCaps->fFBFetchColorName = "sk_LastFragColor";
    }

    if (isIntel) {
        shaderCaps->fVectorClampMinMaxSupport = false;
    }

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fNonsquareMatrixSupport = true;
    shaderCaps->fInverseHyperbolicSupport = true;

    // Metal uses IEEE floats so assuming those values here.
    shaderCaps->fFloatIs32Bits = true;
}


void MtlCaps::initFormatTable(const id<MTLDevice> device) {
    // Determine supported sample counts (used for all renderable formats that can be resolved).
    SkEnumBitMask<SampleCount> deviceSampleCounts = SampleCount::k1;
    if (!fAvoidMSAA) {
        for (auto sampleCnt : {SampleCount::k2, SampleCount::k4, SampleCount::k8}) {
            if ([device supportsTextureSampleCount: (uint8_t) sampleCnt]) {
                deviceSampleCounts |= sampleCnt;
            }
        }
    }

    for (int i = 0; i < kTextureFormatCount; ++i) {
        TextureFormat tf = static_cast<TextureFormat>(i);
        MTLPixelFormat format = TextureFormatToMTLPixelFormat(tf);
        SkEnumBitMask<MTLFeatureFlag> features = MTLPixelFormatSupport(fGPUFamily, format);

        // Check device capabilities to override `features`
#ifdef SK_BUILD_FOR_MAC
        if ((format == MTLPixelFormatDepth24Unorm_Stencil8 ||
             format == MTLPixelFormatX24_Stencil8) &&
            ![device isDepth24Stencil8PixelFormatSupported]) {
            features = MTLFeatureFlag::NotAvailable;
        }
#endif

        auto& [supportedUsage, supportedSampleCounts] = fFormatSupport[i];
        if (features == MTLFeatureFlag::NotAvailable) {
            SkASSERT(!SkToBool(supportedUsage) && !SkToBool(supportedSampleCounts));
            continue;
        }

        // Convert from the Metal format feature flags to Graphite's TextureUsage.
        SkASSERT(format != MTLPixelFormatInvalid);

        // Every available pixel format can be used as a copy src and dst in Metal.
        // Graphite chooses to exclude kCopySrc on compressed formats
        if (TextureFormatCompressionType(tf) != SkTextureCompressionType::kNone) {
            supportedUsage |= TextureUsage::kCopyDst;
        } else {
            SkASSERT(tf != TextureFormat::kExternal);
            supportedUsage |= TextureUsage::kCopySrc | TextureUsage::kCopyDst;
        }

        // Graphite assumes you can filter in a shader (not just read texels)
        if (features & MTLFeatureFlag::Filter) {
            supportedUsage |= TextureUsage::kSample;
        }

        // Graphite requires textures to be writable to be used for compute storage
        // (they are always readable).
        if (features & MTLFeatureFlag::Write) {
            supportedUsage |= TextureUsage::kStorage;
        }

        SkEnumBitMask<MTLFeatureFlag> msaaFlags = MTLFeatureFlag::MSAA;
        if (TextureFormatIsDepthOrStencil(tf)) {
            // All available DS formats can be rendered into; Graphite never resolves DS attachments
            // so they support MSAA with just that feature flag
            supportedUsage |= TextureUsage::kRender;
        } else if ((features & MTLFeatureFlag::Color) && (features & MTLFeatureFlag::Blend)) {
            // Color formats require both being a color render target feature and having blending to
            // be useful.
            supportedUsage |= TextureUsage::kRender;
            // Graphite will also assume that it can use the texture format as the resolve target
            // for color attachments.
            msaaFlags |= MTLFeatureFlag::Resolve;
        }

        if ((features & msaaFlags) == msaaFlags) {
            SkASSERT(supportedUsage & TextureUsage::kRender);
            supportedSampleCounts = deviceSampleCounts;
        } else {
            supportedSampleCounts = SampleCount::k1;
        }
    }
}

std::pair<SkEnumBitMask<TextureUsage>, Tiling> MtlCaps::getTextureUsage(
        const TextureInfo& info) const {
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(info);

    SkEnumBitMask<TextureUsage> usage;
    if (mtlInfo.fUsage & MTLTextureUsageRenderTarget) {
        usage |= TextureUsage::kRender;
        // NOTE: No support for MSRTSS
    }

    // Other than rendering, every other usage is blocked if it's framebuffer-only
    if (!mtlInfo.fFramebufferOnly) {
        if (mtlInfo.fUsage & MTLTextureUsageShaderRead) {
            usage |= TextureUsage::kSample;

            if (mtlInfo.fUsage & MTLTextureUsageShaderWrite) {
                usage |= TextureUsage::kStorage;
            }
        }

        // Always include CopySrc and CopyDst, relying on the format's supported flags to mask
        // the final capabilities automatically (e.g. if this were compressed)
        usage |= TextureUsage::kCopySrc | TextureUsage::kCopyDst;

        // NOTE: No support for TextureUsage::kHostCopy yet
    }

    return {usage, Tiling::kOptimal};
}

TextureInfo MtlCaps::onGetDefaultTextureInfo(SkEnumBitMask<TextureUsage> usage,
                                             TextureFormat format,
                                             SampleCount sampleCount,
                                             Mipmapped mipmapped,
                                             Protected,
                                             Discardable discardable) const {
    MTLPixelFormat mtlFormat = TextureFormatToMTLPixelFormat(format);
    SkASSERT(mtlFormat != MTLPixelFormatInvalid); // should have been caught by support check first

    // Default to private in the event it's not discardable or memoryless is not available
    MTLStorageMode storageMode = MTLStorageModePrivate;
    MTLTextureUsage mtlUsage = MTLTextureUsageUnknown;

    if (usage & TextureUsage::kSample) {
        mtlUsage |= MTLTextureUsageShaderRead;
    }
    if (usage & TextureUsage::kStorage) {
        mtlUsage |= MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
    }
    if (usage & TextureUsage::kRender) {
        mtlUsage |= MTLTextureUsageRenderTarget;
        // Switch to memoryless for discardable render targets when possible (only on Apple silicon)
        if (discardable == Discardable::kYes && this->isApple()) {
            storageMode = MTLStorageModeMemoryless;
        }
    }
    // NOTE: CopyDst and CopySrc are not MTLTextureUsages that have to be requested, so ignore them.
    // Caps should not be requested anything with HostCopy or MSRTSS since these are not supported.
    SkASSERT(!SkToBool(usage & TextureUsage::kHostCopy) &&
             !SkToBool(usage & TextureUsage::kMSRTSS));

    MtlTextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;
    info.fFormat = mtlFormat;
    info.fUsage = mtlUsage;
    info.fStorageMode = storageMode;
    info.fFramebufferOnly = false;

    return TextureInfos::MakeMetal(info);
}

namespace {

skgpu::UniqueKey::Domain get_domain() {
    static const skgpu::UniqueKey::Domain kMtlGraphicsPipelineDomain =
            skgpu::UniqueKey::GenerateDomain();

    return kMtlGraphicsPipelineDomain;
}

}

static constexpr uint16_t kMtlGraphicsPipelineKeyData32Count = 4;

UniqueKey MtlCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                           const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        // 4 uint32_t's (render step id, paint id, renderpass desc, uint16 write swizzle key)
        UniqueKey::Builder builder(&pipelineKey, get_domain(),
                                   kMtlGraphicsPipelineKeyData32Count, "MtlGraphicsPipeline");
        // add GraphicsPipelineDesc key
        builder[0] = static_cast<uint32_t>(pipelineDesc.renderStepID());
        builder[1] = pipelineDesc.paintParamsID().asUInt();

        // add RenderPassDesc key
        builder[2] = this->getRenderPassDescKey(renderPassDesc);
        builder[3] = renderPassDesc.fWriteSwizzle.asKey();

        builder.finish();
    }

    return pipelineKey;
}

bool MtlCaps::extractGraphicsDescs(const UniqueKey& key,
                                   GraphicsPipelineDesc* pipelineDesc,
                                   RenderPassDesc* renderPassDesc,
                                   const RendererProvider* rendererProvider) const {
    struct UnpackedKeyData {
        // From the GraphicsPipelineDesc
        RenderStep::RenderStepID fRenderStepID = RenderStep::RenderStepID::kInvalid;
        UniquePaintParamsID fPaintParamsID = UniquePaintParamsID::Invalid();

        // From the RenderPassDesc
        TextureFormat fColorFormat = TextureFormat::kUnsupported;
        SampleCount fColorSampleCount = SampleCount::k1;

        TextureFormat fDSFormat = TextureFormat::kUnsupported;
        SampleCount fDSSampleCount = SampleCount::k1;

        Swizzle fWriteSwizzle;
    } keyData;

    SkASSERT(key.domain() == get_domain());
    SkASSERT(key.dataSize() == 4 * kMtlGraphicsPipelineKeyData32Count);

    const uint32_t* rawKeyData = key.data();

    SkASSERT(RenderStep::IsValidRenderStepID(rawKeyData[0]));
    keyData.fRenderStepID = static_cast<RenderStep::RenderStepID>(rawKeyData[0]);
    keyData.fPaintParamsID = rawKeyData[1] ? UniquePaintParamsID(rawKeyData[1])
                                           : UniquePaintParamsID::Invalid();

    keyData.fDSFormat = static_cast<TextureFormat>((rawKeyData[2] >> 8) & 0xFF);
    keyData.fDSSampleCount = static_cast<SampleCount>(rawKeyData[2] & 0xFF);

    keyData.fColorFormat = static_cast<TextureFormat>((rawKeyData[2] >> 24) & 0xFF);
    keyData.fColorSampleCount = static_cast<SampleCount>((rawKeyData[2] >> 16) & 0xFF);

    keyData.fWriteSwizzle = SwizzleCtorAccessor::Make(rawKeyData[3]);

    // Recreate the RenderPassDesc, picking arbitrary load/store ops. Since Metal doesn't need
    // to include resolve attachment details, assume that if color attachment's sample count is > 1
    // that there is a matching resolve attachment (no MSAA-render-to-single-sample support in MTL).
    SkASSERT(keyData.fColorSampleCount == keyData.fDSSampleCount ||
             keyData.fDSFormat == TextureFormat::kUnsupported);
    *renderPassDesc = {};
    renderPassDesc->fColorAttachment = {keyData.fColorFormat,
                                        LoadOp::kClear,
                                        StoreOp::kStore,
                                        keyData.fColorSampleCount};
    renderPassDesc->fDepthStencilAttachment = {keyData.fDSFormat,
                                               LoadOp::kClear,
                                               StoreOp::kDiscard,
                                               keyData.fDSSampleCount};
    if (keyData.fColorSampleCount > SampleCount::k1) {
        renderPassDesc->fColorResolveAttachment = {keyData.fColorFormat,
                                                   LoadOp::kClear,
                                                   StoreOp::kStore,
                                                   SampleCount::k1};
        renderPassDesc->fColorAttachment.fStoreOp = StoreOp::kDiscard;
    }

    renderPassDesc->fSampleCount = keyData.fColorSampleCount;
    renderPassDesc->fWriteSwizzle = keyData.fWriteSwizzle;
    renderPassDesc->fDstReadStrategy = this->getDstReadStrategy();

    // Recreate the GraphicsPipelineDesc
    const RenderStep* renderStep = rendererProvider->lookup(keyData.fRenderStepID);

    UniquePaintParamsID paintID = renderStep->performsShading() ? keyData.fPaintParamsID
                                                                : UniquePaintParamsID::Invalid();

    *pipelineDesc = GraphicsPipelineDesc(renderStep->renderStepID(), paintID);

    return true;
}

uint32_t MtlCaps::getRenderPassDescKey(const RenderPassDesc& renderPassDesc) const {
    static_assert(kTextureFormatCount <= 256);

    // Each attachment format + sample count fits in 16-bits. Load/store ops are ignored.
    auto attachmentKey = [](AttachmentDesc desc) {
        SkASSERT(desc.fFormat != TextureFormat::kUnsupported ||
                 desc.fSampleCount == SampleCount::k1);
        return (static_cast<uint32_t>(desc.fFormat) << 8) |
                static_cast<uint32_t>(desc.fSampleCount);
    };

    // The MtlRenderPassDescriptor requires no information about the resolve attachment
    return (attachmentKey(renderPassDesc.fColorAttachment) << 16) |
            attachmentKey(renderPassDesc.fDepthStencilAttachment);
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

void MtlCaps::buildKeyForTexture(SkISize dimensions,
                                 const TextureInfo& info,
                                 ResourceType type,
                                 GraphiteResourceKey* key) const {
    const auto& mtlInfo = TextureInfoPriv::Get<MtlTextureInfo>(info);

    SkASSERT(!dimensions.isEmpty());

    // A MTLPixelFormat is an NSUInteger type which is documented to be 32 bits in 32 bit
    // applications and 64 bits in 64 bit applications. So it should fit in an uint64_t, but adding
    // the assert heere to make sure.
    static_assert(sizeof(MTLPixelFormat) <= sizeof(uint64_t));
    SkASSERT(mtlInfo.fFormat != MTLPixelFormatInvalid);
    uint64_t formatKey = static_cast<uint64_t>(mtlInfo.fFormat);

    uint32_t samplesKey = SamplesToKey(info.sampleCount());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = mtlInfo.fMipmapped == Mipmapped::kYes;
    Protected isProtected = info.isProtected();
    bool isFBOnly = mtlInfo.fFramebufferOnly;

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32.
    SkASSERT(samplesKey                         < (1u << kNumSampleKeyBits));
    SkASSERT(static_cast<uint32_t>(isMipped)    < (1u << 1));
    SkASSERT(static_cast<uint32_t>(isProtected) < (1u << 1));
    SkASSERT(mtlInfo.fUsage                     < (1u << 5));
    SkASSERT(mtlInfo.fStorageMode               < (1u << 2));
    SkASSERT(static_cast<uint32_t>(isFBOnly)    < (1u << 1));

    // We need two uint32_ts for dimensions, 2 for format, and 1 for the rest of the key;
    static uint16_t kNum32DataCnt = 2 + 2 + 1;

    GraphiteResourceKey::Builder builder(key, type, kNum32DataCnt);

    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey & 0xFFFFFFFF;
    builder[3] = (formatKey >> 32) & 0xFFFFFFFF;
    builder[4] = (samplesKey                                  << 0) |
                 (static_cast<uint32_t>(isMipped)             << kNumSampleKeyBits) |
                 (static_cast<uint32_t>(isProtected)          << 4) |
                 (static_cast<uint32_t>(mtlInfo.fUsage)       << 5) |
                 (static_cast<uint32_t>(mtlInfo.fStorageMode) << 10)|
                 (static_cast<uint32_t>(isFBOnly)             << 12);

}

} // namespace skgpu::graphite
