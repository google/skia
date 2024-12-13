/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skresources/include/SkResources.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImage.h"
#include "include/private/base/SkTPin.h"
#include "modules/skresources/src/SkAnimCodecPlayer.h"
#include "src/base/SkBase64.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"

#include <cmath>

#if defined(HAVE_VIDEO_DECODER)
    #include "experimental/ffmpeg/SkVideoDecoder.h"
    #include "include/core/SkStream.h"
#endif

namespace skresources {
namespace  {

#if defined(HAVE_VIDEO_DECODER)

class VideoAsset final : public ImageAsset {
public:
    static sk_sp<VideoAsset> Make(sk_sp<SkData> data) {
        auto decoder = std::make_unique<SkVideoDecoder>();

        if (!decoder->loadStream(SkMemoryStream::Make(std::move(data))) ||
            decoder->duration() <= 0) {
            return nullptr;
        }

        return sk_sp<VideoAsset>(new VideoAsset(std::move(decoder)));
    }

private:
    explicit VideoAsset(std::unique_ptr<SkVideoDecoder> decoder)
        : fDecoder(std::move(decoder)) {
    }

    bool isMultiFrame() override { return true; }

    // Each frame has a presentation timestamp
    //   => the timespan for frame N is [stamp_N .. stamp_N+1)
    //   => we use a two-frame sliding window to track the current interval.
    void advance() {
        fWindow[0] = std::move(fWindow[1]);
        fWindow[1].frame = fDecoder->nextImage(&fWindow[1].stamp);
        fEof = !fWindow[1].frame;
    }

    sk_sp<SkImage> getFrame(float t_float) override {
        const auto t = SkTPin(static_cast<double>(t_float), 0.0, fDecoder->duration());

        if (t < fWindow[0].stamp) {
            // seeking back requires a full rewind
            fDecoder->rewind();
            fWindow[0].stamp = fWindow[1].stamp = 0;
            fEof = 0;
        }

        while (!fEof && t >= fWindow[1].stamp) {
            this->advance();
        }

        SkASSERT(fWindow[0].stamp <= t && (fEof || t < fWindow[1].stamp));

        return fWindow[0].frame;
    }

    const std::unique_ptr<SkVideoDecoder> fDecoder;

    struct FrameRec {
        sk_sp<SkImage> frame;
        double         stamp = 0;
    };

    FrameRec fWindow[2];
    bool     fEof = false;
};

#endif // defined(HAVE_VIDEO_DECODER)

} // namespace

sk_sp<SkImage> ImageAsset::getFrame(float t) {
    return nullptr;
}

ImageAsset::FrameData ImageAsset::getFrameData(float t) {
    // legacy behavior
    return {
        this->getFrame(t),
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest),
        SkMatrix::I(),
        SizeFit::kCenter,
    };
}

sk_sp<MultiFrameImageAsset> MultiFrameImageAsset::Make(sk_sp<SkData> data, ImageDecodeStrategy strat) {
    if (auto codec = SkCodec::MakeFromData(std::move(data))) {
        return sk_sp<MultiFrameImageAsset>(new MultiFrameImageAsset(
                std::make_unique<SkAnimCodecPlayer>(std::move(codec)), strat));
    }

    return nullptr;
}

sk_sp<MultiFrameImageAsset> MultiFrameImageAsset::Make(std::unique_ptr<SkCodec> codec, ImageDecodeStrategy strat) {
    SkASSERT(codec);
    return sk_sp<MultiFrameImageAsset>(new MultiFrameImageAsset(
            std::make_unique<SkAnimCodecPlayer>(std::move(codec)), strat));
}

MultiFrameImageAsset::MultiFrameImageAsset(std::unique_ptr<SkAnimCodecPlayer> player,
                                           ImageDecodeStrategy strat)
        : fPlayer(std::move(player)), fStrategy(strat) {
    SkASSERT(fPlayer);
}

bool MultiFrameImageAsset::isMultiFrame() { return fPlayer->duration() > 0; }

float MultiFrameImageAsset::duration() const { return fPlayer->duration(); }

sk_sp<SkImage> MultiFrameImageAsset::generateFrame(float t) {
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
                                       SkSamplingOptions(SkFilterMode::kLinear,
                                                         SkMipmapMode::kNearest),
                                       SkImage::kDisallow_CachingHint)) {
                image = bm.asImage();
            }
        } else {
            // When the image size is OK, just force-decode.
            image = image->makeRasterImage();
        }

        return image;
    };

    fPlayer->seek(static_cast<uint32_t>(t * 1000));
    auto frame = fPlayer->getFrame();

    if (fStrategy == ImageDecodeStrategy::kPreDecode && frame && frame->isLazyGenerated()) {
        // The multi-frame decoder should never return lazy images.
        SkASSERT(!this->isMultiFrame());
        frame = decode(std::move(frame));
    }

    return frame;
}

sk_sp<SkImage> MultiFrameImageAsset::getFrame(float t) {
    // For static images we can reuse the cached frame
    // (which includes the optional pre-decode step).
    if (!fCachedFrame || this->isMultiFrame()) {
        fCachedFrame = this->generateFrame(t);
    }

    return fCachedFrame;
}

sk_sp<FileResourceProvider> FileResourceProvider::Make(SkString base_dir, ImageDecodeStrategy strat) {
    return sk_isdir(base_dir.c_str()) ? sk_sp<FileResourceProvider>(new FileResourceProvider(
                                                std::move(base_dir), strat))
                                      : nullptr;
}

FileResourceProvider::FileResourceProvider(SkString base_dir, ImageDecodeStrategy strat)
        : fDir(std::move(base_dir)), fStrategy(strat) {}

sk_sp<SkData> FileResourceProvider::load(const char resource_path[],
                                         const char resource_name[]) const {
    const auto full_dir  = SkOSPath::Join(fDir.c_str()    , resource_path),
               full_path = SkOSPath::Join(full_dir.c_str(), resource_name);
    return SkData::MakeFromFileName(full_path.c_str());
}

sk_sp<ImageAsset> FileResourceProvider::loadImageAsset(const char resource_path[],
                                                       const char resource_name[],
                                                       const char[]) const {
    auto data = this->load(resource_path, resource_name);

    if (auto image = MultiFrameImageAsset::Make(data, fStrategy)) {
        return std::move(image);
    }

#if defined(HAVE_VIDEO_DECODER)
    if (auto video = VideoAsset::Make(data)) {
        return std::move(video);
    }
#endif

    return nullptr;
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

sk_sp<SkTypeface> ResourceProviderProxyBase::loadTypeface(const char name[],
                                                          const char url[]) const {
    return fProxy ? fProxy->loadTypeface(name, url)
                  : nullptr;
}

sk_sp<SkData> ResourceProviderProxyBase::loadFont(const char name[], const char url[]) const {
    return fProxy ? fProxy->loadFont(name, url)
                  : nullptr;
}

sk_sp<ExternalTrackAsset> ResourceProviderProxyBase::loadAudioAsset(const char path[],
                                                                    const char name[],
                                                                    const char id[]) {
    return fProxy ? fProxy->loadAudioAsset(path, name, id)
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
                                                                       ImageDecodeStrategy strat,
                                                                       sk_sp<const SkFontMgr> mgr) {
    return sk_sp<DataURIResourceProviderProxy>(
            new DataURIResourceProviderProxy(std::move(rp), strat, std::move(mgr)));
}

DataURIResourceProviderProxy::DataURIResourceProviderProxy(sk_sp<ResourceProvider> rp,
                                                           ImageDecodeStrategy strat,
                                                           sk_sp<const SkFontMgr> mgr)
        : INHERITED(std::move(rp)), fStrategy(strat), fFontMgr(std::move(mgr)) {}

static sk_sp<SkData> decode_datauri(const char prefix[], const char uri[]) {
    // We only handle B64 encoded image dataURIs: data:image/<type>;base64,<data>
    // (https://en.wikipedia.org/wiki/Data_URI_scheme)
    static constexpr char kDataURIEncodingStr[] = ";base64,";

    const size_t prefixLen = strlen(prefix);
    if (strncmp(uri, prefix, prefixLen) != 0) {
        return nullptr;
    }

    const char* encoding = strstr(uri + prefixLen, kDataURIEncodingStr);
    if (!encoding) {
        return nullptr;
    }

    const char* b64Data = encoding + std::size(kDataURIEncodingStr) - 1;
    size_t b64DataLen = strlen(b64Data);
    size_t dataLen;
    if (SkBase64::Decode(b64Data, b64DataLen, nullptr, &dataLen) != SkBase64::kNoError) {
        return nullptr;
    }

    sk_sp<SkData> data = SkData::MakeUninitialized(dataLen);
    void* rawData = data->writable_data();
    if (SkBase64::Decode(b64Data, b64DataLen, rawData, &dataLen) != SkBase64::kNoError) {
        return nullptr;
    }

    return data;
}

sk_sp<ImageAsset> DataURIResourceProviderProxy::loadImageAsset(const char rpath[],
                                                               const char rname[],
                                                               const char rid[]) const {
    // First try to decode the data as base64 using codecs registered with SkCodecs::Register()
    if (auto data = decode_datauri("data:image/", rname)) {
        return MultiFrameImageAsset::Make(std::move(data), fStrategy);
    }
    // Fallback to the asking the ProviderProxy to load this image for us.
    return this->INHERITED::loadImageAsset(rpath, rname, rid);
}

sk_sp<SkTypeface> DataURIResourceProviderProxy::loadTypeface(const char name[],
                                                             const char url[]) const {
    if (fFontMgr) {
        if (auto data = decode_datauri("data:font/", url)) {
            return fFontMgr->makeFromData(std::move(data));
        }
    }

    return this->INHERITED::loadTypeface(name, url);
}

} // namespace skresources
