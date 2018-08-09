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

GrNXTTexture::GrNXTTexture(GrNXTGpu* gpu,
                           dawn::Texture texture,
                           dawn::TextureView textureView,
                           const GrSurfaceDesc& desc,
                           const GrNXTImageInfo& info,
                           GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, desc)
    , GrTexture(gpu, desc, GrTextureType::k2D, mipMapsStatus)
    , fInfo(info)
    , fTexture(texture.Clone())
    , fTextureView(textureView.Clone()) {
}

sk_sp<GrNXTTexture> GrNXTTexture::Make(GrNXTGpu* gpu, const GrSurfaceDesc& desc,
                                       SkBudgeted budgeted, GrMipMapsStatus status) {
    bool renderTarget = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    dawn::TextureFormat textureFormat = GrPixelConfigToNXTFormat(desc.fConfig);

    dawn::TextureUsageBit usage =
        dawn::TextureUsageBit::Sampled |
        dawn::TextureUsageBit::TransferSrc |
        dawn::TextureUsageBit::TransferDst;

    if (renderTarget) {
        usage |= dawn::TextureUsageBit::OutputAttachment;
    }

    dawn::Texture tex = gpu->device().CreateTextureBuilder()
        .SetDimension(dawn::TextureDimension::e2D)
        .SetExtent(desc.fWidth, desc.fHeight, 1)
        .SetFormat(textureFormat)
        .SetMipLevels(1)
        .SetAllowedUsage(usage)
        .GetResult();

    if (!tex) {
        return nullptr;
    }

    dawn::TextureView textureView = tex.CreateTextureViewBuilder().GetResult();

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

GrBackendTexture GrNXTTexture::getBackendTexture() const {
    return GrBackendTexture(this->width(), this->height(), fInfo);
}

void GrNXTTexture::upload(const GrMipLevel texels[], int mipLevels,
                          dawn::CommandBufferBuilder copyBuilder) {
    upload(texels, mipLevels, SkIRect::MakeWH(width(), height()), copyBuilder.Clone());
}

void GrNXTTexture::upload(const GrMipLevel texels[], int mipLevels, const SkIRect& rect,
                          dawn::CommandBufferBuilder copyBuilder) {
    dawn::Device device = getNXTGpu()->device();
    int x = rect.x();
    int y = rect.y();
    int width = rect.width();
    int height = rect.height();
    for (int i = 0; i < mipLevels; i++) {
        size_t rowBytes = width * GrBytesPerPixel(this->config());
        SkASSERT(rowBytes <= texels[i].fRowBytes);
        size_t origRowBytes = rowBytes;
        if ((rowBytes & 0xFF) != 0) {
            rowBytes = (rowBytes + 0xFF) & ~0xFF;
        }
        size_t size = rowBytes * height;
        GrNXTStagingBuffer* stagingBuffer = getNXTGpu()->getStagingBuffer(size);
        const char* src = static_cast<const char*>(texels[i].fPixels);
        if (rowBytes == origRowBytes) {
            memcpy(stagingBuffer->fData, src, size);
        } else {
            char* dst = static_cast<char*>(stagingBuffer->fData);
            for (int row = 0; row < height; row++) {
                memcpy(dst, src, origRowBytes);
                dst += rowBytes;
                src += texels[i].fRowBytes;
            }
        }
        dawn::Buffer buffer = stagingBuffer->fBuffer.Clone();
        buffer.Unmap();
        stagingBuffer->fData = nullptr;
        copyBuilder
            .CopyBufferToTexture(buffer, 0, rowBytes, fTexture, x, y, 0, width, height, 1, i);
        x /= 2;
        y /= 2;
        width /= 2;
        height /= 2;
    }
}
