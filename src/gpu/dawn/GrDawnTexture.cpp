/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnTexture.h"

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnTextureRenderTarget.h"
#include "src/gpu/dawn/GrDawnUtil.h"

GrDawnTexture::GrDawnTexture(GrDawnGpu* gpu,
                             const SkISize& size,
                             GrPixelConfig config,
                             dawn::TextureView textureView,
                             const GrDawnImageInfo& info,
                             GrMipMapsStatus mipMapsStatus)
    : GrSurface(gpu, size, config, GrProtected::kNo)
    , GrTexture(gpu, size, config, GrProtected::kNo, GrTextureType::k2D, mipMapsStatus)
    , fInfo(info)
    , fTextureView(textureView) {
}

sk_sp<GrDawnTexture> GrDawnTexture::Make(GrDawnGpu* gpu, const SkISize& size, GrPixelConfig config,
                                         dawn::TextureFormat format, GrRenderable renderable,
                                         int sampleCnt, SkBudgeted budgeted, int mipLevels,
                                         GrMipMapsStatus status) {
    bool renderTarget = renderable == GrRenderable::kYes;
    dawn::TextureDescriptor textureDesc;

    textureDesc.usage =
        dawn::TextureUsageBit::Sampled |
        dawn::TextureUsageBit::CopySrc |
        dawn::TextureUsageBit::CopyDst;

    if (renderTarget) {
        textureDesc.usage |= dawn::TextureUsageBit::OutputAttachment;
    }

    textureDesc.size.width = size.fWidth;
    textureDesc.size.height = size.fHeight;
    textureDesc.size.depth = 1;
    textureDesc.format = format;
    textureDesc.mipLevelCount = std::max(mipLevels, 1);
    textureDesc.sampleCount = sampleCnt;

    dawn::Texture tex = gpu->device().CreateTexture(&textureDesc);

    if (!tex) {
        return nullptr;
    }

    dawn::TextureView textureView = tex.CreateDefaultView();

    if (!textureView) {
        return nullptr;
    }

    GrDawnImageInfo info;
    info.fTexture = tex;
    info.fFormat = textureDesc.format;
    info.fLevelCount = mipLevels;
    sk_sp<GrDawnTexture> result;
    if (renderTarget) {
        result = sk_sp<GrDawnTextureRenderTarget>(new GrDawnTextureRenderTarget(gpu,
                                                                                size,
                                                                                config,
                                                                                textureView,
                                                                                sampleCnt,
                                                                                info,
                                                                                status));
    } else {
        result = sk_sp<GrDawnTexture>(new GrDawnTexture(gpu, size, config, textureView, info,
                                                        status));
    }
    result->registerWithCache(budgeted);
    return result;
}

GrBackendFormat GrDawnTexture::backendFormat() const {
    return GrBackendFormat::MakeDawn(fInfo.fFormat);
}

sk_sp<GrDawnTexture> GrDawnTexture::MakeWrapped(GrDawnGpu* gpu, const SkISize& size,
                                                GrPixelConfig config, GrMipMapsStatus status,
                                                GrWrapCacheable cacheable,
                                                const GrDawnImageInfo& info) {
    dawn::TextureView textureView = info.fTexture.CreateDefaultView();
    if (!textureView) {
        return nullptr;
    }

    sk_sp<GrDawnTexture> tex(new GrDawnTexture(gpu, size, config, textureView, info, status));
    tex->registerWithCacheWrapped(cacheable);
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

void GrDawnTexture::upload(const GrMipLevel texels[], int mipLevels) {
    upload(texels, mipLevels, SkIRect::MakeWH(width(), height()));
}

void GrDawnTexture::upload(const GrMipLevel texels[], int mipLevels, const SkIRect& rect) {
    dawn::Device device = this->getDawnGpu()->device();
    dawn::Queue queue = this->getDawnGpu()->queue();

    uint32_t x = rect.x();
    uint32_t y = rect.y();
    uint32_t width = rect.width();
    uint32_t height = rect.height();

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
            SkColorType colorType =
                GrColorTypeToSkColorType(GrPixelConfigToColorType(this->config()));
            srcInfo = SkImageInfo::Make(width, height, colorType, kOpaque_SkAlphaType);
            SkPixmap srcPixmap(srcInfo, texels[i].fPixels, origRowBytes);
            origRowBytes = width * GrBytesPerPixel(kRGBA_8888_GrPixelConfig);
            origRowBytes = GrDawnRoundRowBytes(origRowBytes);
            bitmap.allocPixels(info, origRowBytes);
            bitmap.writePixels(srcPixmap);
            if (!bitmap.peekPixels(&pixmap)) {
                continue;
            }
            src = static_cast<const char*>(pixmap.addr());
        } else {
            src = static_cast<const char*>(texels[i].fPixels);
        }
        size_t rowBytes = GrDawnRoundRowBytes(origRowBytes);
        size_t size = rowBytes * height;

        dawn::BufferDescriptor desc;
        desc.usage = dawn::BufferUsageBit::CopyDst | dawn::BufferUsageBit::CopySrc;
        desc.size = size;

        dawn::Buffer stagingBuffer = device.CreateBuffer(&desc);

        if (rowBytes == origRowBytes) {
            stagingBuffer.SetSubData(0, size,
                static_cast<const uint8_t*>(static_cast<const void *>(src)));
        } else {
            char* buf = new char[size];
            char* dst = buf;
            for (uint32_t row = 0; row < height; row++) {
                memcpy(dst, src, origRowBytes);
                dst += rowBytes;
                src += texels[i].fRowBytes;
            }
            stagingBuffer.SetSubData(0, size,
                static_cast<const uint8_t*>(static_cast<const void*>(buf)));
            delete[] buf;
        }

        dawn::BufferCopyView srcBuffer;
        srcBuffer.buffer = stagingBuffer;
        srcBuffer.offset = 0;
        srcBuffer.rowPitch = rowBytes;
        srcBuffer.imageHeight = height;

        dawn::TextureCopyView dstTexture;
        dstTexture.texture = fInfo.fTexture;
        dstTexture.mipLevel = i;
        dstTexture.origin = {x, y, 0};

        dawn::Extent3D copySize = {width, height, 1};
        dawn::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        dawn::CommandBuffer copy = encoder.Finish();
        queue.Submit(1, &copy);

        x /= 2;
        y /= 2;
        width /= 2;
        height /= 2;
    }
}
