/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTTexture.h"

#include "GrNXTGpu.h"
#include "GrNXTTextureRenderTarget.h"
#include "GrNXTUtil.h"

namespace {
    // This method parallels GrTextureProxy::highestFilterMode
    inline GrSamplerState::Filter highest_filter_mode(GrPixelConfig config) {
        return GrSamplerState::Filter::kMipMap;
    }
}

GrNXTTexture::GrNXTTexture(GrNXTGpu* gpu,
                           nxt::Texture texture,
                           nxt::TextureView textureView,
                           const GrSurfaceDesc& desc,
                           const GrNXTImageInfo& info,
                           GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, desc)
    , GrTexture(gpu, desc, kTexture2DSampler_GrSLType, highest_filter_mode(desc.fConfig),
                mipMapsStatus)
    , fInfo(info)
    , fTexture(texture.Clone())
    , fTextureView(textureView.Clone()) {
}

sk_sp<GrNXTTexture> GrNXTTexture::Make(GrNXTGpu* gpu, const GrSurfaceDesc& desc,
                                       SkBudgeted budgeted, GrMipMapsStatus status) {
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    nxt::TextureFormat textureFormat = GrPixelConfigToNXTFormat(desc.fConfig);

    nxt::TextureUsageBit usage =
        nxt::TextureUsageBit::Sampled |
        nxt::TextureUsageBit::TransferSrc |
        nxt::TextureUsageBit::TransferDst;

    if (renderTarget) {
        usage |= nxt::TextureUsageBit::OutputAttachment;
    }

    nxt::Texture tex = gpu->device().CreateTextureBuilder()
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(desc.fWidth, desc.fHeight, 1)
        .SetFormat(textureFormat)
        .SetMipLevels(1)
        .SetAllowedUsage(usage)
        .GetResult();

    if (!tex) {
        return nullptr;
    }

    nxt::TextureView textureView = tex.CreateTextureViewBuilder().GetResult();

    if (!textureView) {
        return nullptr;
    }

    GrNXTImageInfo info;
    info.fTexture = tex.Get();
    info.fFormat = textureFormat;
    info.fLevelCount = 0; // FIXME
    sk_sp<GrNXTTexture> result;
    if (renderTarget) {
        result = sk_sp<GrNXTTextureRenderTarget>(new GrNXTTextureRenderTarget(gpu,
                                                                              tex.Clone(),
                                                                              textureView.Clone(),
                                                                              desc,
                                                                              info,
                                                                              status));
    } else {
        result = sk_sp<GrNXTTexture>(new GrNXTTexture(gpu, tex.Clone(), textureView.Clone(),
                                                      desc, info, status));
    }
    result->registerWithCache(budgeted);
    return result;
}

GrNXTTexture::~GrNXTTexture() {
}

GrNXTGpu* GrNXTTexture::getNXTGpu() const {
    SkASSERT(!this->wasDestroyed());
    return static_cast<GrNXTGpu*>(this->getGpu());
}

void GrNXTTexture::onRelease() {
    INHERITED::onRelease();
}

void GrNXTTexture::onAbandon() {
    INHERITED::onAbandon();
}

GrBackendObject GrNXTTexture::getTextureHandle() const {
    return (GrBackendObject)&fInfo;
}

GrBackendTexture GrNXTTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo);
}

void GrNXTTexture::upload(const GrMipLevel texels[], int mipLevels,
                          nxt::CommandBufferBuilder copyBuilder) {
    upload(texels, mipLevels, SkIRect::MakeWH(width(), height()), copyBuilder.Clone());
}

void GrNXTTexture::upload(const GrMipLevel texels[], int mipLevels, const SkIRect& rect,
                          nxt::CommandBufferBuilder copyBuilder) {
    nxt::Device device = getNXTGpu()->device();
    int x = rect.x();
    int y = rect.y();
    int width = rect.width();
    int height = rect.height();
    for (int i = 0; i < mipLevels; i++) {
        size_t rowBytes = texels[i].fRowBytes;
        if (rowBytes % 256 != 0) {
            rowBytes = ((texels[i].fRowBytes / 256) + 1) * 256;
        }
        if (!fStagingBuffer) {
            int totalRowBytes = this->width() * GrBytesPerPixel(this->config());
            totalRowBytes = (totalRowBytes + 255) & ~255;
            fStagingBuffer = device.CreateBufferBuilder()
                .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::TransferSrc)
                .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
                .SetSize(totalRowBytes * this->height())
                .GetResult();
        } else {
            fStagingBuffer.TransitionUsage(nxt::BufferUsageBit::TransferDst);
        }
        if (rowBytes != texels[i].fRowBytes) {
            const char* src = static_cast<const char*>(texels[i].fPixels);
            char* buf = new char[rowBytes * height];
            char* dst = buf;
            for (int row = 0; row < height; row++) {
                memcpy(dst, src, texels[i].fRowBytes);
                src += texels[i].fRowBytes;
                dst += rowBytes;
            }
            fStagingBuffer.SetSubData(0, rowBytes * height, static_cast<const uint32_t*>(static_cast<const void*>(buf)));
            delete[] buf;
        } else {
            fStagingBuffer.SetSubData(0, texels[i].fRowBytes * height,
                static_cast<const uint32_t*>(texels[i].fPixels));
        }
        copyBuilder
            .TransitionBufferUsage(fStagingBuffer, nxt::BufferUsageBit::TransferSrc)
            .TransitionTextureUsage(fTexture, nxt::TextureUsageBit::TransferDst)
            .CopyBufferToTexture(fStagingBuffer, 0, rowBytes, fTexture, x, y, 0, width, height, 1,
                                 i);
        x /= 2;
        y /= 2;
        width /= 2;
        height /= 2;
    }
}
