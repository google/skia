/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnTexture.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "include/private/gpu/graphite/DawnTypesPriv.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

wgpu::Texture DawnTexture::MakeDawnTexture(const DawnSharedContext* sharedContext,
                                           SkISize dimensions,
                                           const TextureInfo& info) {
    const Caps* caps = sharedContext->caps();
    if (dimensions.width() > caps->maxTextureSize() ||
        dimensions.height() > caps->maxTextureSize()) {
        SKGPU_LOG_E("Texture creation failure: dimensions %d x %d too large.",
                    dimensions.width(), dimensions.height());
        return {};
    }

    const DawnTextureSpec& dawnSpec = info.dawnTextureSpec();

    if (dawnSpec.fUsage & wgpu::TextureUsage::TextureBinding && !caps->isTexturable(info)) {
        return {};
    }

    if (dawnSpec.fUsage & wgpu::TextureUsage::RenderAttachment &&
        !(caps->isRenderable(info) || DawnFormatIsDepthOrStencil(dawnSpec.fFormat))) {
        return {};
    }

    if (dawnSpec.fUsage & wgpu::TextureUsage::StorageBinding && !caps->isStorage(info)) {
        return {};
    }

    int numMipLevels = 1;
    if (info.mipmapped() == Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    wgpu::TextureDescriptor desc;
    desc.usage                      = dawnSpec.fUsage;
    desc.dimension                  = wgpu::TextureDimension::e2D;
    desc.size.width                 = dimensions.width();
    desc.size.height                = dimensions.height();
    desc.size.depthOrArrayLayers    = 1;
    desc.format                     = dawnSpec.fFormat;
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

DawnTexture::DawnTexture(const DawnSharedContext* sharedContext,
                         SkISize dimensions,
                         const TextureInfo& info,
                         wgpu::Texture texture,
                         wgpu::TextureView sampleTextureView,
                         wgpu::TextureView renderTextureView,
                         Ownership ownership,
                         skgpu::Budgeted budgeted)
        : Texture(sharedContext,
                  dimensions,
                  info,
                  /*mutableState=*/nullptr,
                  ownership,
                  budgeted)
        , fTexture(std::move(texture))
        , fSampleTextureView(std::move(sampleTextureView))
        , fRenderTextureView(std::move(renderTextureView)) {}

// static
std::pair<wgpu::TextureView, wgpu::TextureView> DawnTexture::CreateTextureViews(
        const wgpu::Texture& texture, const TextureInfo& info) {
    const auto aspect = info.dawnTextureSpec().fAspect;
    if (aspect == wgpu::TextureAspect::All) {
        wgpu::TextureViewDescriptor viewDesc = {};
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        viewDesc.baseArrayLayer = info.dawnTextureSpec().fSlice;
        viewDesc.arrayLayerCount = 1;
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
    planeViewDesc.format = info.dawnTextureSpec().fViewFormat;
    planeViewDesc.dimension = wgpu::TextureViewDimension::e2D;
    planeViewDesc.aspect = aspect;
    planeViewDesc.baseArrayLayer = info.dawnTextureSpec().fSlice;
    planeViewDesc.arrayLayerCount = 1;
    planeTextureView = texture.CreateView(&planeViewDesc);
    return {planeTextureView, planeTextureView};
#endif
}

sk_sp<Texture> DawnTexture::Make(const DawnSharedContext* sharedContext,
                                 SkISize dimensions,
                                 const TextureInfo& info,
                                 skgpu::Budgeted budgeted) {
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
                                          Ownership::kOwned,
                                          budgeted));
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
                                          Ownership::kWrapped,
                                          skgpu::Budgeted::kNo));
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
                                          Ownership::kWrapped,
                                          skgpu::Budgeted::kNo));
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

