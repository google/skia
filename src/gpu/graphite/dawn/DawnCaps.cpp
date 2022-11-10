/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnCaps.h"

#include "include/gpu/graphite/TextureInfo.h"

namespace skgpu::graphite {

DawnCaps::DawnCaps(const wgpu::Device& device, const ContextOptions& options)
    : Caps() {
    this->initCaps(device);
    this->finishInitialization(options);
}

DawnCaps::~DawnCaps() = default;

bool DawnCaps::onIsTexturable(const TextureInfo& info) const {
    return false;
}

bool DawnCaps::isRenderable(const TextureInfo& info) const {
    return false;
}

TextureInfo DawnCaps::getDefaultSampledTextureInfo(SkColorType colorType,
                                                   Mipmapped mipmapped,
                                                   Protected,
                                                   Renderable renderable) const {
    return {};
}

TextureInfo DawnCaps::getDefaultMSAATextureInfo(const TextureInfo& singleSampledInfo,
                                                Discardable discardable) const {
    return {};
}

TextureInfo DawnCaps::getDefaultDepthStencilTextureInfo(
    SkEnumBitMask<DepthStencilFlags> depthStencilType,
    uint32_t sampleCount,
    Protected) const {
    return {};
}

const Caps::ColorTypeInfo* DawnCaps::getColorTypeInfo(SkColorType colorType,
                                                      const TextureInfo& textureInfo) const {
    return nullptr;
}

size_t DawnCaps::getTransferBufferAlignment(size_t bytesPerPixel) const {
    return std::max(bytesPerPixel, this->getMinBufferAlignment());
}

bool DawnCaps::supportsWritePixels(const TextureInfo& textureInfo) const {
    return false;
}

bool DawnCaps::supportsReadPixels(const TextureInfo& textureInfo) const {
    return false;
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
    SkASSERT(false);
    return kUnknown_SkColorType;
}

void DawnCaps::initCaps(const wgpu::Device& device) {
    wgpu::SupportedLimits limits;
    if (!device.GetLimits(&limits)) {
        SkASSERT(false);
    }
    fMaxTextureSize = limits.limits.maxTextureDimension2D;

    fRequiredUniformBufferAlignment = 256;
    fRequiredStorageBufferAlignment = fRequiredUniformBufferAlignment;

    // TODO: support storage buffer
    fStorageBufferSupport = false;
    fStorageBufferPreferred = false;

    // TODO: support clamp to border.
    fClampToBorderSupport = false;
}

UniqueKey DawnCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                            const RenderPassDesc& renderPassDesc) const {
    return {};
}

UniqueKey DawnCaps::makeComputePipelineKey(const ComputePipelineDesc& pipelineDesc) const {
    return {};
}

void DawnCaps::buildKeyForTexture(SkISize dimensions,
                                  const TextureInfo& info,
                                  ResourceType type,
                                  Shareable shareable,
                                  GraphiteResourceKey* key) const {}

} // namespace skgpu::graphite

