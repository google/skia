/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnCaps.h"

#include <algorithm>

#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/dawn/DawnUtilsPriv.h"
#include "src/gpu/graphite/AttachmentTypes.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"
#include "src/sksl/SkSLUtil.h"

namespace {

// These are all the valid wgpu::TextureFormat that we currently support in Skia.
// They are roughly ordered from most frequently used to least to improve lookup times in arrays.
static constexpr wgpu::TextureFormat kFormats[] = {
    wgpu::TextureFormat::RGBA8Unorm,
    wgpu::TextureFormat::R8Unorm,
    wgpu::TextureFormat::BGRA8Unorm,
    wgpu::TextureFormat::RGBA16Float,
    wgpu::TextureFormat::R16Float,
    wgpu::TextureFormat::RG8Unorm,
    wgpu::TextureFormat::RGB10A2Unorm,
    wgpu::TextureFormat::RG16Float,

    wgpu::TextureFormat::Stencil8,
    wgpu::TextureFormat::Depth32Float,
    wgpu::TextureFormat::Depth24PlusStencil8,

    wgpu::TextureFormat::Undefined,
};

}

namespace skgpu::graphite {

DawnCaps::DawnCaps(const wgpu::Device& device, const ContextOptions& options)
    : Caps() {
    this->initCaps(device, options);
    this->initShaderCaps();
    this->initFormatTable(device);
    this->finishInitialization(options);
}

DawnCaps::~DawnCaps() = default;

uint32_t DawnCaps::channelMask(const TextureInfo& info) const {
    return skgpu::DawnFormatChannels(info.dawnTextureSpec().fFormat);
}

bool DawnCaps::onIsTexturable(const TextureInfo& info) const {
    if (!(info.dawnTextureSpec().fUsage & wgpu::TextureUsage::TextureBinding)) {
        return false;
    }
    return this->isTexturable(info.dawnTextureSpec().fFormat);
}

bool DawnCaps::isTexturable(wgpu::TextureFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTexturable_Flag & formatInfo.fFlags);
}

bool DawnCaps::isRenderable(const TextureInfo& info) const {
    return info.dawnTextureSpec().fUsage & wgpu::TextureUsage::RenderAttachment &&
    this->isRenderable(info.dawnTextureSpec().fFormat, info.numSamples());
}

bool DawnCaps::isStorage(const TextureInfo& info) const {
    if (!(info.dawnTextureSpec().fUsage & wgpu::TextureUsage::StorageBinding)) {
        return false;
    }
    const FormatInfo& formatInfo = this->getFormatInfo(info.dawnTextureSpec().fFormat);
    return info.numSamples() == 1 && SkToBool(FormatInfo::kStorage_Flag & formatInfo.fFlags);
}

uint32_t DawnCaps::maxRenderTargetSampleCount(wgpu::TextureFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (!SkToBool(formatInfo.fFlags & FormatInfo::kRenderable_Flag)) {
        return 0;
    }
    if (SkToBool(formatInfo.fFlags & FormatInfo::kMSAA_Flag)) {
        return 8;
    } else {
        return 1;
    }
}

bool DawnCaps::isRenderable(wgpu::TextureFormat format, uint32_t sampleCount) const {
    return sampleCount <= this->maxRenderTargetSampleCount(format);
}

TextureInfo DawnCaps::getDefaultSampledTextureInfo(SkColorType colorType,
                                                   Mipmapped mipmapped,
                                                   Protected,
                                                   Renderable renderable) const {
    wgpu::TextureUsage usage = wgpu::TextureUsage::TextureBinding |
                               wgpu::TextureUsage::CopyDst |
                               wgpu::TextureUsage::CopySrc;
    if (renderable == Renderable::kYes) {
        usage |= wgpu::TextureUsage::RenderAttachment;
    }

    wgpu::TextureFormat format = this->getFormatFromColorType(colorType);
    if (format == wgpu::TextureFormat::Undefined) {
        return {};
    }

    DawnTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fFormat = format;
    info.fUsage = usage;

    return info;
}

TextureInfo DawnCaps::getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                                   Mipmapped mipmapped) const {
    DawnTextureInfo info;
    if (!textureInfo.getDawnTextureInfo(&info)) {
        return {};
    }

    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fUsage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst |
                  wgpu::TextureUsage::CopySrc;

    return info;
}

TextureInfo DawnCaps::getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                                Discardable discardable) const {
    if (fDefaultMSAASamples <= 1) {
        return {};
    }
    const DawnTextureSpec& singleSpec = singleSampledInfo.dawnTextureSpec();

    DawnTextureInfo info;
    info.fSampleCount = fDefaultMSAASamples;
    info.fMipmapped   = Mipmapped::kNo;
    info.fFormat      = singleSpec.fFormat;
    info.fUsage       = wgpu::TextureUsage::RenderAttachment;

    if (fTransientAttachmentSupport && discardable == Discardable::kYes) {
        info.fUsage |= wgpu::TextureUsage::TransientAttachment;
    }

    return info;
}

TextureInfo DawnCaps::getDefaultDepthStencilTextureInfo(
    SkEnumBitMask<DepthStencilFlags> depthStencilType,
    uint32_t sampleCount,
    Protected) const {
    DawnTextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped   = Mipmapped::kNo;
    info.fFormat      = DawnDepthStencilFlagsToFormat(depthStencilType);
    info.fUsage       = wgpu::TextureUsage::RenderAttachment;

    if (fTransientAttachmentSupport) {
        info.fUsage |= wgpu::TextureUsage::TransientAttachment;
    }

    return info;
}

TextureInfo DawnCaps::getDefaultStorageTextureInfo(SkColorType colorType) const {
    // Storage textures are currently always sampleable from a shader.
    wgpu::TextureUsage usage = wgpu::TextureUsage::StorageBinding |
                               wgpu::TextureUsage::TextureBinding |
                               wgpu::TextureUsage::CopySrc;
    wgpu::TextureFormat format = this->getFormatFromColorType(colorType);
    if (format == wgpu::TextureFormat::Undefined) {
        SkDebugf("colorType=%d is not supported\n", static_cast<int>(colorType));
        return {};
    }

    DawnTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = Mipmapped::kNo;
    info.fFormat = format;
    info.fUsage = usage;

    return info;
}

const Caps::ColorTypeInfo* DawnCaps::getColorTypeInfo(SkColorType colorType,
                                                      const TextureInfo& textureInfo) const {
    auto dawnFormat = textureInfo.dawnTextureSpec().fFormat;
    if (dawnFormat == wgpu::TextureFormat::Undefined) {
        SkASSERT(false);
        return nullptr;
    }

    const FormatInfo& info = this->getFormatInfo(dawnFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const ColorTypeInfo& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return &ctInfo;
        }
    }

    return nullptr;
}

bool DawnCaps::supportsWritePixels(const TextureInfo& textureInfo) const {
    const auto& spec = textureInfo.dawnTextureSpec();
    return spec.fUsage & wgpu::TextureUsage::CopyDst;
}

bool DawnCaps::supportsReadPixels(const TextureInfo& textureInfo) const {
    const auto& spec = textureInfo.dawnTextureSpec();
    return spec.fUsage & wgpu::TextureUsage::CopySrc;
}

SkColorType DawnCaps::supportedWritePixelsColorType(SkColorType dstColorType,
                                                    const TextureInfo& dstTextureInfo,
                                                    SkColorType srcColorType) const {
    SkASSERT(false);
    return kUnknown_SkColorType;
}

SkColorType DawnCaps::supportedReadPixelsColorType(SkColorType srcColorType,
                                                   const TextureInfo& srcTextureInfo,
                                                   SkColorType dstColorType) const {
    auto dawnFormat = getFormatFromColorType(srcColorType);
    const FormatInfo& info = this->getFormatInfo(dawnFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return srcColorType;
        }
    }
    return kUnknown_SkColorType;
}

void DawnCaps::initCaps(const wgpu::Device& device, const ContextOptions& options) {
    wgpu::SupportedLimits limits;
    if (!device.GetLimits(&limits)) {
        SkASSERT(false);
    }
    fMaxTextureSize = limits.limits.maxTextureDimension2D;

    fRequiredTransferBufferAlignment = 4;
    fRequiredUniformBufferAlignment = 256;
    fRequiredStorageBufferAlignment = fRequiredUniformBufferAlignment;

    // Dawn requires 256 bytes per row alignment for buffer texture copies.
    fTextureDataRowBytesAlignment = 256;

    fResourceBindingReqs.fUniformBufferLayout = Layout::kStd140;
    // TODO(skia:14639): We cannot use std430 layout for SSBOs until SkSL gracefully handles
    // implicit array stride.
    fResourceBindingReqs.fStorageBufferLayout = Layout::kStd140;
    fResourceBindingReqs.fSeparateTextureAndSamplerBinding = true;

    // TODO: support storage buffer
    fStorageBufferSupport = false;
    fStorageBufferPreferred = false;

    fDrawBufferCanBeMapped = false;

    fComputeSupport = true;

    // TODO: support clamp to border.
    fClampToBorderSupport = false;

    fMSAARenderToSingleSampledSupport =
            device.HasFeature(wgpu::FeatureName::MSAARenderToSingleSampled);

    fTransientAttachmentSupport = device.HasFeature(wgpu::FeatureName::TransientAttachments);
    fEnableWGSL = options.fEnableWGSL;
}

void DawnCaps::initShaderCaps() {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // WGSL does not support infinities regardless of hardware support. There are discussions around
    // enabling it using an extension in the future.
    shaderCaps->fInfinitySupport = false;

    // WGSL supports shader derivatives in the fragment shader
    shaderCaps->fShaderDerivativeSupport = true;
}

void DawnCaps::initFormatTable(const wgpu::Device& device) {
    FormatInfo* info;
    // Format: RGBA8Unorm
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::RGBA8Unorm)];
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

    // TODO(crbug.com/dawn/1856): Support storage binding for compute shader in Dawn.
    // Format: R8Unorm
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::R8Unorm)];
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

    // Format: BGRA8Unorm
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::BGRA8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 2;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: BGRA8Unorm, Surface: kBGRA_8888
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kBGRA_8888_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
        // Format: BGRA8Unorm, Surface: kRGB_888x
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kRGB_888x_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
        }
    }

    // Format: RGBA16Float
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::RGBA16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RGBA16Float, Surface: RGBA_F16
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kRGBA_F16_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: R16Float
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::R16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
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

    // TODO(crbug.com/dawn/1856): Support storage binding for compute shader in Dawn.
    // Format: RG8Unorm
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::RG8Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RG8Unorm, Surface: kR8G8_unorm
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kR8G8_unorm_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: RGB10A2Unorm
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::RGB10A2Unorm)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RGB10A2Unorm, Surface: kRGBA_1010102
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kRGBA_1010102_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    // Format: RG16Float
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::RG16Float)];
        info->fFlags = FormatInfo::kAllFlags;
        info->fColorTypeInfoCount = 1;
        info->fColorTypeInfos.reset(new ColorTypeInfo[info->fColorTypeInfoCount]());
        int ctIdx = 0;
        // Format: RG16Float, Surface: kR16G16_float
        {
            auto& ctInfo = info->fColorTypeInfos[ctIdx++];
            ctInfo.fColorType = kR16G16_float_SkColorType;
            ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
        }
    }

    /*
     * Non-color formats
     */

    // Format: Stencil8
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::Stencil8)];
        info->fFlags = FormatInfo::kMSAA_Flag;
        info->fColorTypeInfoCount = 0;
    }

    // Format: Depth32Float
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::Depth32Float)];
        info->fFlags = FormatInfo::kMSAA_Flag;
        info->fColorTypeInfoCount = 0;
    }

    // Format: Depth24PlusStencil8
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::Depth24PlusStencil8)];
        info->fFlags = FormatInfo::kMSAA_Flag;
        info->fColorTypeInfoCount = 0;
    }

    // Format: Undefined
    {
        info = &fFormatTable[GetFormatIndex(wgpu::TextureFormat::Undefined)];
        info->fFlags = 0;
        info->fColorTypeInfoCount = 0;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Map SkColorTypes (used for creating SkSurfaces) to wgpu::TextureFormat.
    // The order in which the formats are passed into the setColorType function
    // indicates the priority in selecting which format we use for a given
    // SkColorType.

    std::fill_n(fColorTypeToFormatTable, kSkColorTypeCnt, wgpu::TextureFormat::Undefined);

    this->setColorType(kAlpha_8_SkColorType,          { wgpu::TextureFormat::R8Unorm });
    this->setColorType(kRGBA_8888_SkColorType,        { wgpu::TextureFormat::RGBA8Unorm });
    this->setColorType(kRGB_888x_SkColorType,
                       {wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::BGRA8Unorm});
    this->setColorType(kBGRA_8888_SkColorType,        { wgpu::TextureFormat::BGRA8Unorm });
    this->setColorType(kGray_8_SkColorType,           { wgpu::TextureFormat::R8Unorm });
    this->setColorType(kR8_unorm_SkColorType,         { wgpu::TextureFormat::R8Unorm });
    this->setColorType(kRGBA_F16_SkColorType,         { wgpu::TextureFormat::RGBA16Float });
    this->setColorType(kA16_float_SkColorType,        { wgpu::TextureFormat::R16Float });
    this->setColorType(kR8G8_unorm_SkColorType,       { wgpu::TextureFormat::RG8Unorm });
    this->setColorType(kRGBA_1010102_SkColorType,     { wgpu::TextureFormat::RGB10A2Unorm });
    this->setColorType(kR16G16_float_SkColorType,     { wgpu::TextureFormat::RG16Float });
}

// static
size_t DawnCaps::GetFormatIndex(wgpu::TextureFormat format) {
    for (size_t i = 0; i < std::size(kFormats); ++i) {
        if (format == kFormats[i]) {
            return i;
        }
        if (kFormats[i] == wgpu::TextureFormat::Undefined) {
            SkDEBUGFAILF("Unsupported wgpu::TextureFormat: %d\n", static_cast<int>(format));
            return i;
        }
    }
    SkUNREACHABLE;
    return 0;
}

void DawnCaps::setColorType(SkColorType colorType,
                            std::initializer_list<wgpu::TextureFormat> formats) {
    static_assert(std::tuple_size<decltype(fFormatTable)>::value == std::size(kFormats),
                  "Size is not same for DawnCaps::fFormatTable and kFormats");
#ifdef SK_DEBUG
    for (size_t i = 0; i < std::size(fFormatTable); ++i) {
        const auto& formatInfo = fFormatTable[i];
        for (int j = 0; j < formatInfo.fColorTypeInfoCount; ++j) {
            const auto& ctInfo = formatInfo.fColorTypeInfos[j];
            if (ctInfo.fColorType == colorType) {
                bool found = false;
                for (auto it = formats.begin(); it != formats.end(); ++it) {
                    if (kFormats[i] == *it) {
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

uint64_t DawnCaps::getRenderPassDescKey(const RenderPassDesc& renderPassDesc) const {
    DawnTextureInfo colorInfo, depthStencilInfo;
    renderPassDesc.fColorAttachment.fTextureInfo.getDawnTextureInfo(&colorInfo);
    renderPassDesc.fDepthStencilAttachment.fTextureInfo.getDawnTextureInfo(&depthStencilInfo);
    SkASSERT(static_cast<uint32_t>(colorInfo.fFormat) <= 0xffff &&
             static_cast<uint32_t>(depthStencilInfo.fFormat) <= 0xffff);
    uint32_t colorAttachmentKey =
            static_cast<uint32_t>(colorInfo.fFormat) << 16 | colorInfo.fSampleCount;
    uint32_t dsAttachmentKey =
            static_cast<uint32_t>(depthStencilInfo.fFormat) << 16 | depthStencilInfo.fSampleCount;
    return (((uint64_t) colorAttachmentKey) << 32) | dsAttachmentKey;
}

UniqueKey DawnCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                            const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kGraphicsPipelineDomain = UniqueKey::GenerateDomain();
        // 5 uint32_t's (render step id, paint id, uint64 RenderPass desc, uint16 write swizzle)
        UniqueKey::Builder builder(&pipelineKey, kGraphicsPipelineDomain, 5, "GraphicsPipeline");
        // add GraphicsPipelineDesc key
        builder[0] = pipelineDesc.renderStepID();
        builder[1] = pipelineDesc.paintParamsID().asUInt();

        // add RenderPassDesc key
        uint64_t renderPassKey = this->getRenderPassDescKey(renderPassDesc);
        builder[2] = renderPassKey & 0xFFFFFFFF;
        builder[3] = (renderPassKey >> 32) & 0xFFFFFFFF;
        builder[4] = renderPassDesc.fWriteSwizzle.asKey();
        builder.finish();
    }

    return pipelineKey;
}

UniqueKey DawnCaps::makeComputePipelineKey(const ComputePipelineDesc& pipelineDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kComputePipelineDomain = UniqueKey::GenerateDomain();
        // The key is made up of a single uint32_t corresponding to the compute step ID.
        UniqueKey::Builder builder(&pipelineKey, kComputePipelineDomain, 1, "ComputePipeline");
        builder[0] = pipelineDesc.computeStep()->uniqueID();

        // TODO(b/240615224): The local work group size should factor into the key here since it is
        // specified in the shader text on Dawn/SPIR-V. This is not a problem right now since
        // ComputeSteps don't vary their workgroup size dynamically.

        builder.finish();
    }
    return pipelineKey;
}

void DawnCaps::buildKeyForTexture(SkISize dimensions,
                                  const TextureInfo& info,
                                  ResourceType type,
                                  Shareable shareable,
                                  GraphiteResourceKey* key) const {
    const DawnTextureSpec& dawnSpec = info.dawnTextureSpec();

    SkASSERT(!dimensions.isEmpty());

    SkASSERT(dawnSpec.fFormat != wgpu::TextureFormat::Undefined);
    uint32_t formatKey = static_cast<uint32_t>(dawnSpec.fFormat);

    uint32_t samplesKey = SamplesToKey(info.numSamples());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = info.mipmapped() == Mipmapped::kYes;

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32.
    SkASSERT(samplesKey                             < (1u << 3));  // sample key is first 3 bits
    SkASSERT(static_cast<uint32_t>(isMipped)        < (1u << 1));  // isMapped is 4th bit
    SkASSERT(static_cast<uint32_t>(dawnSpec.fUsage) < (1u << 28)); // usage is remaining 28 bits

    // We need two uint32_ts for dimensions, 1 for format, and 1 for the rest of the key;
    static int kNum32DataCnt = 2 + 1 + 1;

    GraphiteResourceKey::Builder builder(key, type, kNum32DataCnt, shareable);

    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey;
    builder[3] = (samplesKey                                   << 0) |
                 (static_cast<uint32_t>(isMipped)              << 3) |
                 (static_cast<uint32_t>(dawnSpec.fUsage)       << 4);
}

} // namespace skgpu::graphite
