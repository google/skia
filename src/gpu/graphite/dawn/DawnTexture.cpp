/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnTexture.h"

#include "include/gpu/MutableTextureState.h"
#include "include/gpu/graphite/dawn/DawnTypes.h"
#include "include/private/gpu/graphite/DawnTypesPriv.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {
namespace {

const char* texture_info_to_label(const TextureInfo& info,
                                  const DawnTextureSpec& dawnSpec) {
#ifdef SK_DEBUG
    if (dawnSpec.fUsage & wgpu::TextureUsage::RenderAttachment) {
        if (DawnFormatIsDepthOrStencil(dawnSpec.fFormat)) {
            return "DepthStencil";
        } else {
            if (info.numSamples() > 1) {
                if (dawnSpec.fUsage & wgpu::TextureUsage::TextureBinding) {
                    return "MSAA SampledTexture-ColorAttachment";
                } else {
                    return "MSAA ColorAttachment";
                }
            } else {
                if (dawnSpec.fUsage & wgpu::TextureUsage::TextureBinding) {
                    return "SampledTexture-ColorAttachment";
                } else {
                    return "ColorAttachment";
                }
            }
        }
    } else {
        SkASSERT(dawnSpec.fUsage & wgpu::TextureUsage::TextureBinding);
        return "SampledTexture";
    }
#else
    return nullptr;
#endif
}

}

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
    desc.label                      = texture_info_to_label(info, dawnSpec);
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
        : Texture(sharedContext, dimensions, info, /*mutableState=*/nullptr, ownership, budgeted)
        , fTexture(std::move(texture))
        , fSampleTextureView(std::move(sampleTextureView))
        , fRenderTextureView(std::move(renderTextureView)) {}

std::pair<wgpu::TextureView, wgpu::TextureView> create_texture_views(
        const wgpu::Texture& texture, const TextureInfo& info, const wgpu::TextureAspect aspect) {
    if (aspect == wgpu::TextureAspect::All) {
        wgpu::TextureView sampleTextureView = texture.CreateView();
        wgpu::TextureView renderTextureView;
        if (info.mipmapped() == Mipmapped::kYes) {
            wgpu::TextureViewDescriptor renderViewDesc = {};
            renderViewDesc.baseMipLevel = 0;
            renderViewDesc.mipLevelCount = 1;
            renderTextureView = texture.CreateView(&renderViewDesc);
        } else {
            renderTextureView = sampleTextureView;
        }
        return {sampleTextureView, renderTextureView};
    }

    SkASSERT(aspect == wgpu::TextureAspect::Plane0Only ||
             aspect == wgpu::TextureAspect::Plane1Only ||
             aspect == wgpu::TextureAspect::Plane2Only);
    wgpu::TextureView planeTextureView;
    wgpu::TextureViewDescriptor planeViewDesc = {};
    planeViewDesc.aspect = aspect;
    planeTextureView = texture.CreateView(&planeViewDesc);
    return {planeTextureView, planeTextureView};
}

sk_sp<Texture> DawnTexture::Make(const DawnSharedContext* sharedContext,
                                 SkISize dimensions,
                                 const TextureInfo& info,
                                 skgpu::Budgeted budgeted) {
    auto texture = MakeDawnTexture(sharedContext, dimensions, info);
    if (!texture) {
        return {};
    }
    auto [sampleTextureView, renderTextureView] =
            create_texture_views(texture, info, wgpu::TextureAspect::All);
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

    const wgpu::TextureAspect aspect = info.dawnTextureSpec().fAspect;
    auto [sampleTextureView, renderTextureView] = create_texture_views(texture, info, aspect);
    return sk_sp<Texture>(new DawnTexture(sharedContext,
                                          dimensions,
                                          info,
                                          std::move(texture),
                                          std::move(sampleTextureView),
                                          std::move(renderTextureView),
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

} // namespace skgpu::graphite

