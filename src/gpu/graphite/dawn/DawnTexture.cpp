/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnTexture.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/MutableTextureState.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

wgpu::Texture DawnTexture::MakeDawnTexture(const DawnSharedContext* sharedContext,
                                           SkISize dimensions,
                                           const TextureInfo& info) {
    const auto* caps = sharedContext->dawnCaps();
    if (dimensions.width() > caps->maxTextureSize() ||
        dimensions.height() > caps->maxTextureSize()) {
        SKGPU_LOG_E("Texture creation failure: dimensions %d x %d too large.",
                    dimensions.width(), dimensions.height());
        return {};
    }

    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);

    if (dawnInfo.fUsage & wgpu::TextureUsage::TextureBinding &&
        !caps->isTexturableIgnoreSampleCount(info)) {
        return {};
    }

    if (dawnInfo.fUsage & wgpu::TextureUsage::RenderAttachment && !caps->isRenderable(info)) {
        return {};
    }

    if (dawnInfo.fUsage & wgpu::TextureUsage::StorageBinding && !caps->isStorage(info)) {
        return {};
    }

#if !defined(__EMSCRIPTEN__)
    // If a non-default YCbCr descriptor is provided, either the vkFormat or the externalFormat must
    // be defined.
    if (DawnDescriptorIsValid(dawnInfo.fYcbcrVkDescriptor) &&
    dawnInfo.fYcbcrVkDescriptor.vkFormat == 0 &&
    dawnInfo.fYcbcrVkDescriptor.externalFormat == 0) {
        return {};
    }
#endif

    int numMipLevels = 1;
    if (info.mipmapped() == Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    wgpu::TextureDescriptor desc;
    desc.usage                      = dawnInfo.fUsage;
    desc.dimension                  = wgpu::TextureDimension::e2D;
    desc.size.width                 = dimensions.width();
    desc.size.height                = dimensions.height();
    desc.size.depthOrArrayLayers    = 1;
    desc.format                     = dawnInfo.fFormat;
    desc.mipLevelCount              = numMipLevels;
    desc.sampleCount                = info.numSamples();
    desc.viewFormatCount            = 0;
    desc.viewFormats                = nullptr;

    auto texture = sharedContext->device().CreateTexture(&desc);
    if (!texture) {
        return {};
    }

    return texture;
}

static bool has_transient_usage(const TextureInfo& info) {
#if !defined(__EMSCRIPTEN__)
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);
    return dawnInfo.fUsage & wgpu::TextureUsage::TransientAttachment;
#else
    return false;
#endif
}

DawnTexture::DawnTexture(const DawnSharedContext* sharedContext,
                         SkISize dimensions,
                         const TextureInfo& info,
                         wgpu::Texture texture,
                         wgpu::TextureView sampleTextureView,
                         wgpu::TextureView renderTextureView,
                         Ownership ownership)
        : Texture(sharedContext,
                  dimensions,
                  info,
                  /*isTransient=*/has_transient_usage(info),
                  /*mutableState=*/nullptr,
                  ownership)
        , fTexture(std::move(texture))
        , fSampleTextureView(std::move(sampleTextureView))
        , fRenderTextureView(std::move(renderTextureView)) {}

// static
std::pair<wgpu::TextureView, wgpu::TextureView> DawnTexture::CreateTextureViews(
        const wgpu::Texture& texture, const TextureInfo& info) {
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);
    const auto aspect = dawnInfo.fAspect;
    if (aspect == wgpu::TextureAspect::All) {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseArrayLayer = dawnInfo.fSlice;
        viewDesc.arrayLayerCount = 1;
#if !defined(__EMSCRIPTEN__)
        // Ensure that the TextureView is configured to use YCbCr sampling if the Texture is
        // doing so.
        const wgpu::YCbCrVkDescriptor& ycbcrDesc = dawnInfo.fYcbcrVkDescriptor;
        if (DawnDescriptorIsValid(ycbcrDesc)) {
            viewDesc.nextInChain = &ycbcrDesc;
        }
#endif
        wgpu::TextureView sampleTextureView = texture.CreateView(&viewDesc);
        wgpu::TextureView renderTextureView;
        if (info.mipmapped() == Mipmapped::kYes) {
            viewDesc.baseMipLevel = 0;
            viewDesc.mipLevelCount = 1;
            renderTextureView = texture.CreateView(&viewDesc);
        } else {
            renderTextureView = sampleTextureView;
        }
        return {sampleTextureView, renderTextureView};
    }

#if defined(__EMSCRIPTEN__)
    SkASSERT(false);
    return {};
#else
    SkASSERT(aspect == wgpu::TextureAspect::Plane0Only ||
             aspect == wgpu::TextureAspect::Plane1Only ||
             aspect == wgpu::TextureAspect::Plane2Only);
    wgpu::TextureView planeTextureView;
    wgpu::TextureViewDescriptor planeViewDesc = {};

    planeViewDesc.format = dawnInfo.fViewFormat;
    planeViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    planeViewDesc.aspect = aspect;
    planeViewDesc.baseArrayLayer = dawnInfo.fSlice;
    planeViewDesc.arrayLayerCount = 1;
    planeTextureView = texture.CreateView(&planeViewDesc);
    return {planeTextureView, planeTextureView};
#endif
}

sk_sp<Texture> DawnTexture::Make(const DawnSharedContext* sharedContext,
                                 SkISize dimensions,
                                 const TextureInfo& info) {
    auto texture = MakeDawnTexture(sharedContext, dimensions, info);
    if (!texture) {
        return {};
    }
    auto [sampleTextureView, renderTextureView] = CreateTextureViews(texture, info);
    return sk_sp<Texture>(new DawnTexture(sharedContext,
                                          dimensions,
                                          info,
                                          std::move(texture),
                                          std::move(sampleTextureView),
                                          std::move(renderTextureView),
                                          Ownership::kOwned));
}

sk_sp<Texture> DawnTexture::MakeWrapped(const DawnSharedContext* sharedContext,
                                        SkISize dimensions,
                                        const TextureInfo& info,
                                        wgpu::Texture texture) {
    if (!texture) {
        SKGPU_LOG_E("No valid texture passed into MakeWrapped\n");
        return {};
    }

    auto [sampleTextureView, renderTextureView] = CreateTextureViews(texture, info);
    return sk_sp<Texture>(new DawnTexture(sharedContext,
                                          dimensions,
                                          info,
                                          std::move(texture),
                                          std::move(sampleTextureView),
                                          std::move(renderTextureView),
                                          Ownership::kWrapped));
}

sk_sp<Texture> DawnTexture::MakeWrapped(const DawnSharedContext* sharedContext,
                                        SkISize dimensions,
                                        const TextureInfo& info,
                                        const wgpu::TextureView& textureView) {
    if (!textureView) {
        SKGPU_LOG_E("No valid texture view passed into MakeWrapped\n");
        return {};
    }
    return sk_sp<Texture>(new DawnTexture(sharedContext,
                                          dimensions,
                                          info,
                                          /*texture=*/nullptr,
                                          /*sampleTextureView=*/textureView,
                                          /*renderTextureView=*/textureView,
                                          Ownership::kWrapped));
}

void DawnTexture::freeGpuData() {
    if (this->ownership() != Ownership::kWrapped && fTexture) {
        // Destroy the texture even if it is still referenced by other BindGroup or views.
        // Graphite should already guarantee that all command buffers using this texture (indirectly
        // via BindGroup or views) are already completed.
        fTexture.Destroy();
    }
    fTexture = nullptr;
    fSampleTextureView = nullptr;
    fRenderTextureView = nullptr;
}

void DawnTexture::setBackendLabel(char const* label) {
    if (!sharedContext()->caps()->setBackendLabels()) {
        return;
    }
    SkASSERT(label);
    // Wrapped texture views won't have an associated texture here.
    if (fTexture) {
        fTexture.SetLabel(label);
    }
    // But we always have the texture views available.
    SkASSERT(fSampleTextureView);
    SkASSERT(fRenderTextureView);
    if (fSampleTextureView.Get() == fRenderTextureView.Get()) {
        fSampleTextureView.SetLabel(SkStringPrintf("%s_%s", label, "_TextureView").c_str());
    } else {
        fSampleTextureView.SetLabel(SkStringPrintf("%s_%s", label, "_SampleTextureView").c_str());
        fRenderTextureView.SetLabel(SkStringPrintf("%s_%s", label, "_RenderTextureView").c_str());
    }
}

} // namespace skgpu::graphite
