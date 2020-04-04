/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skresources/include/SkResources.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/utils/SkAnimCodecPlayer.h"
#include "include/utils/SkBase64.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"

namespace skresources {

sk_sp<MultiFrameImageAsset> MultiFrameImageAsset::Make(sk_sp<SkData> data, bool predecode) {
    if (auto codec = SkCodec::MakeFromData(std::move(data))) {
        return sk_sp<MultiFrameImageAsset>(
              new MultiFrameImageAsset(std::make_unique<SkAnimCodecPlayer>(std::move(codec)),
                                                                             predecode));
    }

    return nullptr;
}

MultiFrameImageAsset::MultiFrameImageAsset(std::unique_ptr<SkAnimCodecPlayer> player,
                                           bool predecode)
    : fPlayer(std::move(player))
    , fPreDecode(predecode) {
    SkASSERT(fPlayer);
}

bool MultiFrameImageAsset::isMultiFrame() {
    return fPlayer->duration() > 0;
}

sk_sp<SkImage> MultiFrameImageAsset::getFrame(float t) {
    auto decode = [](sk_sp<SkImage> image) {
        SkASSERT(image->isLazyGenerated());

        static constexpr size_t kMaxArea = 2048 * 2048;
        const auto image_area = SkToSizeT(image->width() * image->height());

        if (image_area > kMaxArea) {
            // When the image is too large, decode and scale down to a reasonable size.
            const auto scale = std::sqrt(static_cast<float>(kMaxArea) / image_area);
            const auto info  = SkImageInfo::MakeN32Premul(scale * image->width(),
                                                          scale * image->height());
            SkBitmap bm;
            if (bm.tryAllocPixels(info, info.minRowBytes()) &&
                    image->scalePixels(bm.pixmap(),
                                       SkFilterQuality::kMedium_SkFilterQuality,
                                       SkImage::kDisallow_CachingHint)) {
                image = SkImage::MakeFromBitmap(bm);
            }
        } else {
            // When the image size is OK, just force-decode.
            image = image->makeRasterImage();
        }

        return image;
    };

    fPlayer->seek(static_cast<uint32_t>(t * 1000));
    auto frame = fPlayer->getFrame();

    if (fPreDecode && frame && frame->isLazyGenerated()) {
        frame = decode(std::move(frame));
    }

    return frame;
}

sk_sp<FileResourceProvider> FileResourceProvider::Make(SkString base_dir, bool predecode) {
    return sk_isdir(base_dir.c_str())
        ? sk_sp<FileResourceProvider>(new FileResourceProvider(std::move(base_dir), predecode))
        : nullptr;
}

FileResourceProvider::FileResourceProvider(SkString base_dir, bool predecode)
    : fDir(std::move(base_dir))
    , fPredecode(predecode) {}

sk_sp<SkData> FileResourceProvider::load(const char resource_path[],
                                         const char resource_name[]) const {
    const auto full_dir  = SkOSPath::Join(fDir.c_str()    , resource_path),
               full_path = SkOSPath::Join(full_dir.c_str(), resource_name);
    return SkData::MakeFromFileName(full_path.c_str());
}

sk_sp<ImageAsset> FileResourceProvider::loadImageAsset(const char resource_path[],
                                                       const char resource_name[],
                                                       const char[]) const {
    return MultiFrameImageAsset::Make(this->load(resource_path, resource_name), fPredecode);
}

ResourceProviderProxyBase::ResourceProviderProxyBase(sk_sp<ResourceProvider> rp)
    : fProxy(std::move(rp)) {}

sk_sp<SkData> ResourceProviderProxyBase::load(const char resource_path[],
                                              const char resource_name[]) const {
    return fProxy ? fProxy->load(resource_path, resource_name)
                  : nullptr;
}

sk_sp<ImageAsset> ResourceProviderProxyBase::loadImageAsset(const char rpath[],
                                                            const char rname[],
                                                            const char rid[]) const {
    return fProxy ? fProxy->loadImageAsset(rpath, rname, rid)
                  : nullptr;
}

sk_sp<SkData> ResourceProviderProxyBase::loadFont(const char name[], const char url[]) const {
    return fProxy ? fProxy->loadFont(name, url)
                  : nullptr;
}

CachingResourceProvider::CachingResourceProvider(sk_sp<ResourceProvider> rp)
    : INHERITED(std::move(rp)) {}

sk_sp<ImageAsset> CachingResourceProvider::loadImageAsset(const char resource_path[],
                                                          const char resource_name[],
                                                          const char resource_id[]) const {
    SkAutoMutexExclusive amx(fMutex);

    const SkString key(resource_id);
    if (const auto* asset = fImageCache.find(key)) {
        return *asset;
    }

    auto asset = this->INHERITED::loadImageAsset(resource_path, resource_name, resource_id);
    fImageCache.set(key, asset);

    return asset;
}

sk_sp<DataURIResourceProviderProxy> DataURIResourceProviderProxy::Make(sk_sp<ResourceProvider> rp,
                                                                       bool predecode) {
    return sk_sp<DataURIResourceProviderProxy>(
            new DataURIResourceProviderProxy(std::move(rp), predecode));
}

DataURIResourceProviderProxy::DataURIResourceProviderProxy(sk_sp<ResourceProvider> rp,
                                                           bool predecode)
    : INHERITED(std::move(rp))
    , fPredecode(predecode) {}

sk_sp<ImageAsset> DataURIResourceProviderProxy::loadImageAsset(const char rpath[],
                                                               const char rname[],
                                                               const char rid[]) const {
    // We only handle B64 encoded image dataURIs: data:image/<type>;base64,<data>
    // (https://en.wikipedia.org/wiki/Data_URI_scheme)
    static constexpr char kDataURIImagePrefix[] = "data:image/",
                          kDataURIEncodingStr[] = ";base64,";

    if (!strncmp(rname, kDataURIImagePrefix, SK_ARRAY_COUNT(kDataURIImagePrefix) - 1)) {
        const char* encoding_start = strstr(rname + SK_ARRAY_COUNT(kDataURIImagePrefix) - 1,
                                            kDataURIEncodingStr);
        if (encoding_start) {
            const char* data_start = encoding_start + SK_ARRAY_COUNT(kDataURIEncodingStr) - 1;

            // TODO: SkBase64::decode ergonomics are... interesting.
            SkBase64 b64;
            if (SkBase64::kNoError == b64.decode(data_start, strlen(data_start))) {
                return MultiFrameImageAsset::Make(SkData::MakeWithProc(b64.getData(),
                                                                       b64.getDataSize(),
                                                      [](const void* ptr, void*) {
                                                          delete[] static_cast<const char*>(ptr);
                                                      }, /*ctx=*/nullptr),
                                                  fPredecode);
            }
        }
    }

    return this->INHERITED::loadImageAsset(rpath, rname, rid);
}

} // namespace skresources
