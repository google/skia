/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/dawn/GrDawnTexture.h"

#include "src/gpu/ganesh/dawn/GrDawnGpu.h"
#include "src/gpu/ganesh/dawn/GrDawnTextureRenderTarget.h"
#include "src/gpu/ganesh/dawn/GrDawnUtil.h"

GrDawnTexture::GrDawnTexture(GrDawnGpu* gpu,
                             SkISize dimensions,
                             const GrDawnTextureInfo& info,
                             GrMipmapStatus mipmapStatus,
                             std::string_view label)
        : GrSurface(gpu, dimensions, GrProtected::kNo, label)
        , GrTexture(gpu, dimensions, GrProtected::kNo, GrTextureType::k2D, mipmapStatus, label)
        , fInfo(info) {}

sk_sp<GrDawnTexture> GrDawnTexture::Make(GrDawnGpu* gpu,
                                         SkISize dimensions,
                                         wgpu::TextureFormat format,
                                         GrRenderable renderable,
                                         int sampleCnt,
                                         skgpu::Budgeted budgeted,
                                         int mipLevels,
                                         GrMipmapStatus status,
                                         std::string_view label) {
    bool renderTarget = renderable == GrRenderable::kYes;
    wgpu::TextureDescriptor textureDesc;

    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                        wgpu::TextureUsage::CopyDst;

    if (renderTarget) {
        textureDesc.usage |= wgpu::TextureUsage::RenderAttachment;
    }

    textureDesc.size.width = dimensions.fWidth;
    textureDesc.size.height = dimensions.fHeight;
    textureDesc.size.depthOrArrayLayers = 1;
    textureDesc.format = format;
    textureDesc.mipLevelCount = std::max(mipLevels, 1);
    textureDesc.sampleCount = sampleCnt;

    wgpu::Texture tex = gpu->device().CreateTexture(&textureDesc);

    if (!tex) {
        return nullptr;
    }

    GrDawnTextureInfo info;
    info.fTexture = tex;
    info.fFormat = textureDesc.format;
    info.fLevelCount = mipLevels;
    sk_sp<GrDawnTexture> result;
    if (renderTarget) {
        result = sk_sp<GrDawnTextureRenderTarget>(new GrDawnTextureRenderTarget(
                gpu, dimensions, sampleCnt, info, status, label));
    } else {
        result = sk_sp<GrDawnTexture>(
                new GrDawnTexture(gpu, dimensions, info, status, label));
    }
    result->registerWithCache(budgeted);
    return result;
}

GrBackendFormat GrDawnTexture::backendFormat() const {
    return GrBackendFormat::MakeDawn(fInfo.fFormat);
}

sk_sp<GrDawnTexture> GrDawnTexture::MakeWrapped(GrDawnGpu* gpu, SkISize dimensions,
                                                GrRenderable renderable, int sampleCnt,
                                                GrWrapCacheable cacheable, GrIOType ioType,
                                                const GrDawnTextureInfo& info,
                                                std::string_view label) {
    sk_sp<GrDawnTexture> tex;
    GrMipmapStatus status = info.fLevelCount > 1 ? GrMipmapStatus::kValid
                                                 : GrMipmapStatus::kNotAllocated;
    if (GrRenderable::kYes == renderable) {
        tex = sk_sp<GrDawnTexture>(new GrDawnTextureRenderTarget(
                gpu, dimensions, sampleCnt, info, status, label));
    } else {
        tex = sk_sp<GrDawnTexture>(new GrDawnTexture(gpu, dimensions, info, status, label));
    }
    tex->registerWithCacheWrapped(cacheable);
    if (ioType == kRead_GrIOType) {
      tex->setReadOnly();
    }
    return tex;
}

GrDawnTexture::~GrDawnTexture() {
}

GrDawnGpu* GrDawnTexture::getDawnGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrDawnGpu*>(this->getGpu());
}

void GrDawnTexture::onRelease() {
    INHERITED::onRelease();
}

void GrDawnTexture::onAbandon() {
    INHERITED::onAbandon();
}

GrBackendTexture GrDawnTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo);
}
