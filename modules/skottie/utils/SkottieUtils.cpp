/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/utils/SkottieUtils.h"

namespace skottie_utils {

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
