/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureUtils.h"

#include "include/core/SkBitmap.h"
#include "src/core/SkMipmap.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/UploadTask.h"

namespace skgpu::graphite {

std::tuple<TextureProxyView, SkColorType> MakeBitmapProxyView(Recorder* recorder,
                                                              const SkBitmap& bitmap,
                                                              sk_sp<SkMipmap> mipmapsIn,
                                                              Mipmapped mipmapped,
                                                              skgpu::Budgeted budgeted) {
    // Adjust params based on input and Caps
    const skgpu::graphite::Caps* caps = recorder->priv().caps();
    SkColorType ct = bitmap.info().colorType();

    if (bitmap.dimensions().area() <= 1) {
        mipmapped = Mipmapped::kNo;
    }

    auto textureInfo = caps->getDefaultSampledTextureInfo(ct, mipmapped, Protected::kNo,
                                                          Renderable::kNo);
    if (!textureInfo.isValid()) {
        ct = kRGBA_8888_SkColorType;
        textureInfo = caps->getDefaultSampledTextureInfo(ct, mipmapped, Protected::kNo,
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

    int mipLevelCount = (mipmapped == Mipmapped::kYes) ?
            SkMipmap::ComputeLevelCount(bitmap.width(), bitmap.height()) + 1 : 1;


    // setup MipLevels
    sk_sp<SkMipmap> mipmaps;
    std::vector<MipLevel> texels;
    if (mipLevelCount == 1) {
        texels.resize(mipLevelCount);
        texels[0].fPixels = bmpToUpload.getPixels();
        texels[0].fRowBytes = bmpToUpload.rowBytes();
    } else {
        mipmaps = SkToBool(mipmapsIn) ? mipmapsIn
                                      : sk_ref_sp(SkMipmap::Build(bmpToUpload.pixmap(), nullptr));
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
    sk_sp<TextureProxy> proxy(new TextureProxy(bmpToUpload.dimensions(), textureInfo, budgeted));
    if (!proxy) {
        return {};
    }
    SkASSERT(caps->areColorTypeAndTextureInfoCompatible(ct, proxy->textureInfo()));
    SkASSERT(mipmapped == Mipmapped::kNo || proxy->mipmapped() == Mipmapped::kYes);

    // Src and dst colorInfo are the same
    const SkColorInfo& colorInfo = bmpToUpload.info().colorInfo();
    // Add UploadTask to Recorder
    UploadInstance upload = UploadInstance::Make(
            recorder, proxy, colorInfo, colorInfo, texels,
            SkIRect::MakeSize(bmpToUpload.dimensions()), nullptr);
    if (!upload.isValid()) {
        SKGPU_LOG_E("MakeBitmapProxyView: Could not create UploadInstance");
        return {};
    }
    recorder->priv().add(UploadTask::Make(std::move(upload)));

    Swizzle swizzle = caps->getReadSwizzle(ct, textureInfo);
    // If the color type is alpha-only, propagate the alpha value to the other channels.
    if (colorInfo.colorType() == kAlpha_8_SkColorType) {
        swizzle = Swizzle::Concat(swizzle, Swizzle("aaaa"));
    }
    return {{std::move(proxy), swizzle}, ct};
}

sk_sp<SkImage> MakeFromBitmap(Recorder* recorder,
                              const SkColorInfo& colorInfo,
                              const SkBitmap& bitmap,
                              sk_sp<SkMipmap> mipmaps,
                              skgpu::Budgeted budgeted,
                              SkImage::RequiredImageProperties requiredProps) {
    auto [ view, ct ] = MakeBitmapProxyView(recorder, bitmap, std::move(mipmaps),
                                            requiredProps.fMipmapped, budgeted);
    if (!view) {
        return nullptr;
    }

    SkASSERT(requiredProps.fMipmapped == skgpu::Mipmapped::kNo ||
             view.proxy()->mipmapped() == skgpu::Mipmapped::kYes);
    return sk_make_sp<skgpu::graphite::Image>(std::move(view),
                                              colorInfo.makeColorType(ct));
}

} // namespace skgpu::graphite
