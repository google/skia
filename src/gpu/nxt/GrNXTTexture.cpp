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

    dawn::TextureDescriptor textureDesc;

    textureDesc.usage =
        dawn::TextureUsageBit::Sampled |
        dawn::TextureUsageBit::TransferSrc |
        dawn::TextureUsageBit::TransferDst;

    if (renderTarget) {
        textureDesc.usage |= dawn::TextureUsageBit::OutputAttachment;
    }

    textureDesc.dimension = dawn::TextureDimension::e2D;
    textureDesc.width = desc.fWidth;
    textureDesc.height = desc.fHeight;
    textureDesc.depth = 1;
    textureDesc.arrayLayer = 1;
    textureDesc.format = GrPixelConfigToNXTFormat(desc.fConfig);
    textureDesc.mipLevel = 1;

    dawn::Texture tex = gpu->device().CreateTexture(&textureDesc);

    if (!tex) {
        return nullptr;
    }

    dawn::TextureView textureView = tex.CreateTextureViewBuilder().GetResult();

    if (!textureView) {
        return nullptr;
    }

    GrNXTImageInfo info;
    info.fTexture = tex.Clone();
    info.fFormat = textureDesc.format;
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

sk_sp<GrNXTTexture> GrNXTTexture::MakeWrapped(GrNXTGpu* gpu, const GrSurfaceDesc& desc,
                                              GrMipMapsStatus status,
                                              const GrNXTImageInfo& info) {
    dawn::Texture texture = info.fTexture.Clone();
    dawn::TextureView textureView = texture.CreateTextureViewBuilder().GetResult();

    sk_sp<GrNXTTexture> tex(new GrNXTTexture(gpu, texture.Clone(), textureView.Clone(), desc, info, status));
    tex->registerWithCacheWrapped();
    return tex;
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
        size_t origRowBytes = texels[i].fRowBytes;
        SkBitmap bitmap;
        SkPixmap pixmap;
        const char* src;
        if (kRGBA_4444_GrPixelConfig == this->config() ||
            kRGB_565_GrPixelConfig == this->config() ||
            kGray_8_GrPixelConfig == this->config()) {
            SkImageInfo info;
            info = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
            SkImageInfo srcInfo;
            SkColorType colorType = GrColorTypeToSkColorType(GrPixelConfigToColorType(this->config()));
            srcInfo = SkImageInfo::Make(width, height, colorType, kOpaque_SkAlphaType);
            SkPixmap srcPixmap(srcInfo, texels[i].fPixels, origRowBytes);
            origRowBytes = width * GrBytesPerPixel(kRGBA_8888_GrPixelConfig);
            if ((origRowBytes & 0xFF) != 0) {
                origRowBytes = (origRowBytes + 0xFF) & ~0xFF;
            }
            bitmap.allocPixels(info, origRowBytes);
            bitmap.writePixels(srcPixmap);
            if (!bitmap.peekPixels(&pixmap)) {
                continue;
            }
            src = static_cast<const char*>(pixmap.addr());
        } else {
            src = static_cast<const char*>(texels[i].fPixels);
        }
        size_t rowBytes = origRowBytes;
        if ((rowBytes & 0xFF) != 0) {
            rowBytes = (rowBytes + 0xFF) & ~0xFF;
        }
        size_t size = rowBytes * height;
        GrNXTStagingBuffer* stagingBuffer = getNXTGpu()->getStagingBuffer(size);
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
            .CopyBufferToTexture(buffer, 0, rowBytes, fTexture, x, y, 0, width, height, 1, i, 0);
        x /= 2;
        y /= 2;
        width /= 2;
        height /= 2;
    }
}
