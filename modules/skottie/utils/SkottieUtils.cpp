/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieUtils.h"

#include "SkAnimCodecPlayer.h"
#include "SkData.h"
#include "SkCodec.h"
#include "SkImage.h"
#include "SkMakeUnique.h"
#include "SkOSFile.h"
#include "SkOSPath.h"

namespace skottie_utils {

sk_sp<MultiFrameImageAsset> MultiFrameImageAsset::Make(sk_sp<SkData> data) {
    if (auto codec = SkCodec::MakeFromData(std::move(data))) {
        return sk_sp<MultiFrameImageAsset>(
              new MultiFrameImageAsset(skstd::make_unique<SkAnimCodecPlayer>(std::move(codec))));
    }

    return nullptr;
}

MultiFrameImageAsset::MultiFrameImageAsset(std::unique_ptr<SkAnimCodecPlayer> player)
    : fPlayer(std::move(player)) {
    SkASSERT(fPlayer);
}

bool MultiFrameImageAsset::isMultiFrame() {
    return fPlayer->duration() > 0;
}

sk_sp<SkImage> MultiFrameImageAsset::getFrame(float t) {
    fPlayer->seek(static_cast<uint32_t>(t * 1000));
    return fPlayer->getFrame();
}

sk_sp<FileResourceProvider> FileResourceProvider::Make(SkString base_dir) {
    return sk_isdir(base_dir.c_str())
        ? sk_sp<FileResourceProvider>(new FileResourceProvider(std::move(base_dir)))
        : nullptr;
}

FileResourceProvider::FileResourceProvider(SkString base_dir) : fDir(std::move(base_dir)) {}

sk_sp<SkData> FileResourceProvider::load(const char resource_path[],
                                         const char resource_name[]) const {
    const auto full_dir  = SkOSPath::Join(fDir.c_str()    , resource_path),
               full_path = SkOSPath::Join(full_dir.c_str(), resource_name);
    return SkData::MakeFromFileName(full_path.c_str());
}

sk_sp<skottie::ImageAsset> FileResourceProvider::loadImageAsset(const char resource_path[],
                                                                const char resource_name[]) const {
    return MultiFrameImageAsset::Make(this->load(resource_path, resource_name));
}

CustomPropertyManagerBuilder::CustomPropertyManagerBuilder() = default;
CustomPropertyManagerBuilder::~CustomPropertyManagerBuilder() = default;

std::unique_ptr<CustomPropertyManager> CustomPropertyManagerBuilder::build() {
    return std::unique_ptr<CustomPropertyManager>(
                new CustomPropertyManager(std::move(fColorMap),
                                          std::move(fOpacityMap),
                                          std::move(fTransformMap)));
}

void CustomPropertyManagerBuilder::onColorProperty(
        const char node_name[],
        const LazyHandle<skottie::ColorPropertyHandle>& c) {
    const auto key = this->acceptProperty(node_name);
    if (!key.empty()) {
        fColorMap[key].push_back(c());
    }
}

void CustomPropertyManagerBuilder::onOpacityProperty(
        const char node_name[],
        const LazyHandle<skottie::OpacityPropertyHandle>& o) {
    const auto key = this->acceptProperty(node_name);
    if (!key.empty()) {
        fOpacityMap[key].push_back(o());
    }
}

void CustomPropertyManagerBuilder::onTransformProperty(
        const char node_name[],
        const LazyHandle<skottie::TransformPropertyHandle>& t) {
    const auto key = this->acceptProperty(node_name);
    if (!key.empty()) {
        fTransformMap[key].push_back(t());
    }
}

CustomPropertyManager::CustomPropertyManager(PropMap<skottie::ColorPropertyHandle> cmap,
                                             PropMap<skottie::OpacityPropertyHandle> omap,
                                             PropMap<skottie::TransformPropertyHandle> tmap)
    : fColorMap(std::move(cmap))
    , fOpacityMap(std::move(omap))
    , fTransformMap(std::move(tmap)) {}

CustomPropertyManager::~CustomPropertyManager() = default;

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
