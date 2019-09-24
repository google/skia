/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/utils/SkottieUtils.h"

#include "include/codec/SkCodec.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/utils/SkAnimCodecPlayer.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"

#include <cmath>

namespace skottie_utils {

sk_sp<MultiFrameImageAsset> MultiFrameImageAsset::Make(sk_sp<SkData> data, bool predecode) {
    if (auto codec = SkCodec::MakeFromData(std::move(data))) {
        return sk_sp<MultiFrameImageAsset>(
              new MultiFrameImageAsset(skstd::make_unique<SkAnimCodecPlayer>(std::move(codec)),
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

sk_sp<skottie::ImageAsset> FileResourceProvider::loadImageAsset(const char resource_path[],
                                                                const char resource_name[],
                                                                const char[]) const {
    return MultiFrameImageAsset::Make(this->load(resource_path, resource_name), fPredecode);
}

CachingResourceProvider::CachingResourceProvider(sk_sp<ResourceProvider> rp)
    : fProxy(std::move(rp)) {}

sk_sp<skottie::ImageAsset> CachingResourceProvider::loadImageAsset(const char resource_path[],
                                                                   const char resource_name[],
                                                                   const char resource_id[]) const {
    SkAutoMutexExclusive amx(fMutex);

    const SkString key(resource_id);
    if (const auto* asset = fImageCache.find(key)) {
        return *asset;
    }

    auto asset = fProxy->loadImageAsset(resource_path, resource_name, resource_id);
    fImageCache.set(key, asset);

    return asset;
}

class CustomPropertyManager::PropertyInterceptor final : public skottie::PropertyObserver {
public:
    explicit PropertyInterceptor(CustomPropertyManager* mgr) : fMgr(mgr) {}

    void onColorProperty(const char node_name[],
                         const LazyHandle<skottie::ColorPropertyHandle>& c) override {
        const auto key = fMgr->acceptKey(node_name);
        if (!key.empty()) {
            fMgr->fColorMap[key].push_back(c());
        }
    }

    void onOpacityProperty(const char node_name[],
                           const LazyHandle<skottie::OpacityPropertyHandle>& o) override {
        const auto key = fMgr->acceptKey(node_name);
        if (!key.empty()) {
            fMgr->fOpacityMap[key].push_back(o());
        }
    }

    void onTransformProperty(const char node_name[],
                             const LazyHandle<skottie::TransformPropertyHandle>& t) override {
        const auto key = fMgr->acceptKey(node_name);
        if (!key.empty()) {
            fMgr->fTransformMap[key].push_back(t());
        }
    }

private:
    CustomPropertyManager* fMgr;
};

class CustomPropertyManager::MarkerInterceptor final : public skottie::MarkerObserver {
public:
    explicit MarkerInterceptor(CustomPropertyManager* mgr) : fMgr(mgr) {}

    void onMarker(const char name[], float t0, float t1) override {
        const auto key = fMgr->acceptKey(name);
        if (!key.empty()) {
            fMgr->fMarkers.push_back({ std::move(key), t0, t1 });
        }
    }

private:
    CustomPropertyManager* fMgr;
};

CustomPropertyManager::CustomPropertyManager()
    : fPropertyInterceptor(sk_make_sp<PropertyInterceptor>(this))
    , fMarkerInterceptor(sk_make_sp<MarkerInterceptor>(this)) {}

CustomPropertyManager::~CustomPropertyManager() = default;

sk_sp<skottie::PropertyObserver> CustomPropertyManager::getPropertyObserver() const {
    return fPropertyInterceptor;
}

sk_sp<skottie::MarkerObserver> CustomPropertyManager::getMarkerObserver() const {
    return fMarkerInterceptor;
}

template <typename T>
std::vector<CustomPropertyManager::PropKey>
CustomPropertyManager::getProps(const PropMap<T>& container) const {
    std::vector<PropKey> props;

    for (const auto& prop_list : container) {
        SkASSERT(!prop_list.second.empty());
        props.push_back(prop_list.first);
    }

    return props;
}

template <typename V, typename T>
V CustomPropertyManager::get(const PropKey& key, const PropMap<T>& container) const {
    auto prop_group = container.find(key);

    return prop_group == container.end()
            ? V()
            : prop_group->second.front()->get();
}

template <typename V, typename T>
bool CustomPropertyManager::set(const PropKey& key, const V& val, const PropMap<T>& container) {
    auto prop_group = container.find(key);

    if (prop_group == container.end()) {
        return false;
    }

    for (auto& handle : prop_group->second) {
        handle->set(val);
    }

    return true;
}

std::vector<CustomPropertyManager::PropKey>
CustomPropertyManager::getColorProps() const {
    return this->getProps(fColorMap);
}

skottie::ColorPropertyValue CustomPropertyManager::getColor(const PropKey& key) const {
    return this->get<skottie::ColorPropertyValue>(key, fColorMap);
}

bool CustomPropertyManager::setColor(const PropKey& key, const skottie::ColorPropertyValue& c) {
    return this->set(key, c, fColorMap);
}

std::vector<CustomPropertyManager::PropKey>
CustomPropertyManager::getOpacityProps() const {
    return this->getProps(fOpacityMap);
}

skottie::OpacityPropertyValue CustomPropertyManager::getOpacity(const PropKey& key) const {
    return this->get<skottie::OpacityPropertyValue>(key, fOpacityMap);
}

bool CustomPropertyManager::setOpacity(const PropKey& key, const skottie::OpacityPropertyValue& o) {
    return this->set(key, o, fOpacityMap);
}

std::vector<CustomPropertyManager::PropKey>
CustomPropertyManager::getTransformProps() const {
    return this->getProps(fTransformMap);
}

bool CustomPropertyManager::setTransform(const PropKey& key,
                                         const skottie::TransformPropertyValue& t) {
    return this->set(key, t, fTransformMap);
}

} // namespace skottie_utils
