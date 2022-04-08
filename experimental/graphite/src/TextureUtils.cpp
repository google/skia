/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/TextureUtils.h"

#include "include/core/SkBitmap.h"
#include "src/core/SkMipmap.h"

#include "experimental/graphite/include/Context.h"
#include "experimental/graphite/include/GraphiteTypes.h"
#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/include/Recording.h"
#include "experimental/graphite/src/Buffer.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/CopyTask.h"
#include "experimental/graphite/src/RecorderPriv.h"
#include "experimental/graphite/src/ResourceProvider.h"
#include "experimental/graphite/src/Texture.h"
#include "experimental/graphite/src/UploadTask.h"

namespace skgpu::graphite {

std::tuple<TextureProxyView, SkColorType> MakeBitmapProxyView(Recorder* recorder,
                                                              const SkBitmap& bitmap,
                                                              Mipmapped mipmapped,
                                                              SkBudgeted budgeted) {
    // Adjust params based on input and Caps
    const skgpu::graphite::Caps* caps = recorder->priv().caps();
    SkColorType ct = bitmap.info().colorType();

    if (bitmap.dimensions().area() <= 1) {
        mipmapped = Mipmapped::kNo;
    }
    int mipLevelCount = (mipmapped == Mipmapped::kYes) ?
            SkMipmap::ComputeLevelCount(bitmap.width(), bitmap.height()) + 1 : 1;

    auto textureInfo = caps->getDefaultSampledTextureInfo(ct, mipLevelCount, Protected::kNo,
                                                          Renderable::kNo);
    if (!textureInfo.isValid()) {
        ct = kRGBA_8888_SkColorType;
        textureInfo = caps->getDefaultSampledTextureInfo(ct, mipLevelCount, Protected::kNo,
                                                         Renderable::kNo);
    }
    SkASSERT(textureInfo.isValid());

    // Convert bitmap to texture colortype if necessary
    SkBitmap bmpToUpload;
    if (ct != bitmap.info().colorType()) {
        if (!bmpToUpload.tryAllocPixels(bitmap.info().makeColorType(ct)) ||
            !bitmap.readPixels(bmpToUpload.pixmap())) {
            return {};
        }
        bmpToUpload.setImmutable();
    } else {
        bmpToUpload = bitmap;
    }

    if (!SkImageInfoIsValid(bmpToUpload.info())) {
        return {};
    }

    // setup MipLevels
    std::vector<MipLevel> texels;
    if (mipLevelCount == 1) {
        texels.resize(mipLevelCount);
        texels[0].fPixels = bmpToUpload.getPixels();
        texels[0].fRowBytes = bmpToUpload.rowBytes();
    } else {
        sk_sp<SkMipmap> mipmaps(SkMipmap::Build(bmpToUpload.pixmap(), nullptr));
        if (!mipmaps) {
            return {};
        }

        SkASSERT(mipLevelCount == mipmaps->countLevels() + 1);
        texels.resize(mipLevelCount);

        texels[0].fPixels = bmpToUpload.getPixels();
        texels[0].fRowBytes = bmpToUpload.rowBytes();

        for (int i = 1; i < mipLevelCount; ++i) {
            SkMipmap::Level generatedMipLevel;
            mipmaps->getLevel(i - 1, &generatedMipLevel);
            texels[i].fPixels = generatedMipLevel.fPixmap.addr();
            texels[i].fRowBytes = generatedMipLevel.fPixmap.rowBytes();
            SkASSERT(texels[i].fPixels);
            SkASSERT(generatedMipLevel.fPixmap.colorType() == bmpToUpload.colorType());
        }
    }

    // Create proxy
    sk_sp<TextureProxy> proxy(new TextureProxy(bmpToUpload.dimensions(), textureInfo));
    if (!proxy) {
        return {};
    }
    SkASSERT(caps->areColorTypeAndTextureInfoCompatible(ct, proxy->textureInfo()));
    SkASSERT(mipmapped == Mipmapped::kNo || proxy->mipmapped() == Mipmapped::kYes);

    // Add UploadTask to Recorder
    UploadInstance upload = UploadInstance::Make(
            recorder, proxy, ct, texels, SkIRect::MakeSize(bmpToUpload.dimensions()));
    recorder->priv().add(UploadTask::Make(upload));

    Swizzle swizzle = caps->getReadSwizzle(ct, textureInfo);
    return {{std::move(proxy), swizzle}, ct};
}

bool ReadPixelsHelper(FlushPendingWorkCallback&& flushPendingWork,
                      Context* context,
                      Recorder* recorder,
                      TextureProxy* srcProxy,
                      const SkImageInfo& dstInfo,
                      void* dstPixels,
                      size_t dstRowBytes,
                      int srcX,
                      int srcY) {
    // TODO: Support more formats that we can read back into
    if (dstInfo.colorType() != kRGBA_8888_SkColorType) {
        return false;
    }

    ResourceProvider* resourceProvider = recorder->priv().resourceProvider();
    if (!srcProxy->instantiate(resourceProvider)) {
        return false;
    }
    sk_sp<Texture> srcTexture = srcProxy->refTexture();
    SkASSERT(srcTexture);

    size_t size = dstRowBytes * dstInfo.height();
    sk_sp<Buffer> dstBuffer = resourceProvider->findOrCreateBuffer(size,
                                                                   BufferType::kXferGpuToCpu,
                                                                   PrioritizeGpuReads::kNo);
    if (!dstBuffer) {
        return false;
    }

    SkIRect srcRect = SkIRect::MakeXYWH(srcX, srcY, dstInfo.width(), dstInfo.height());
    sk_sp<CopyTextureToBufferTask> task =
            CopyTextureToBufferTask::Make(std::move(srcTexture),
                                          srcRect,
                                          dstBuffer,
                                          /*bufferOffset=*/0,
                                          dstRowBytes);
    if (!task) {
        return false;
    }

    flushPendingWork();
    recorder->priv().add(std::move(task));

    std::unique_ptr<Recording> recording = recorder->snap();
    if (!recording) {
        return false;
    }
    InsertRecordingInfo info;
    info.fRecording = recording.get();
    context->insertRecording(info);
    context->submit(SyncToCpu::kYes);

    void* mappedMemory = dstBuffer->map();

    memcpy(dstPixels, mappedMemory, size);

    return true;
}

} // namespace skgpu::graphite
