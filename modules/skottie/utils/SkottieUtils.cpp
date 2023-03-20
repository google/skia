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
        const auto key = fMgr->acceptKey(node_name, ".Color");
        if (!key.empty()) {
            fMgr->fColorMap[key].push_back(c());
        }
    }

    void onOpacityProperty(const char node_name[],
                           const LazyHandle<skottie::OpacityPropertyHandle>& o) override {
        const auto key = fMgr->acceptKey(node_name, ".Opacity");
        if (!key.empty()) {
            fMgr->fOpacityMap[key].push_back(o());
        }
    }

    void onTransformProperty(const char node_name[],
                             const LazyHandle<skottie::TransformPropertyHandle>& t) override {
        const auto key = fMgr->acceptKey(node_name, ".Transform");
        if (!key.empty()) {
            fMgr->fTransformMap[key].push_back(t());
        }
    }

    void onTextProperty(const char node_name[],
                        const LazyHandle<skottie::TextPropertyHandle>& t) override {
        const auto key = fMgr->acceptKey(node_name, ".Text");
        if (!key.empty()) {
            fMgr->fTextMap[key].push_back(t());
        }
    }

    void onEnterNode(const char node_name[], PropertyObserver::NodeType node_type) override {
        if (node_name == nullptr) {
            return;
        }
        fMgr->fCurrentNode =
                fMgr->fCurrentNode.empty() ? node_name : fMgr->fCurrentNode + "." + node_name;
    }

    void onLeavingNode(const char node_name[], PropertyObserver::NodeType node_type) override {
        if (node_name == nullptr) {
            return;
        }
        auto length = strlen(node_name);
        fMgr->fCurrentNode =
                fMgr->fCurrentNode.length() > length
                        ? fMgr->fCurrentNode.substr(
                                  0, fMgr->fCurrentNode.length() - strlen(node_name) - 1)
                        : "";
    }

private:
    CustomPropertyManager* fMgr;
};

class CustomPropertyManager::MarkerInterceptor final : public skottie::MarkerObserver {
public:
    explicit MarkerInterceptor(CustomPropertyManager* mgr) : fMgr(mgr) {}

    void onMarker(const char name[], float t0, float t1) override {
        // collect all markers
        fMgr->fMarkers.push_back({ std::string(name), t0, t1 });
    }

private:
    CustomPropertyManager* fMgr;
};

CustomPropertyManager::CustomPropertyManager(Mode mode, const char* prefix)
    : fMode(mode)
    , fPrefix(prefix ? prefix : "$")
    , fPropertyInterceptor(sk_make_sp<PropertyInterceptor>(this))
    , fMarkerInterceptor(sk_make_sp<MarkerInterceptor>(this)) {}

CustomPropertyManager::~CustomPropertyManager() = default;

std::string CustomPropertyManager::acceptKey(const char* name, const char* suffix) const {
    if (!SkStrStartsWith(name, fPrefix.c_str())) {
        return std::string();
    }

    return fMode == Mode::kCollapseProperties
            ? std::string(name)
            : fCurrentNode + suffix;
}

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

skottie::TransformPropertyValue CustomPropertyManager::getTransform(const PropKey& key) const {
    return this->get<skottie::TransformPropertyValue>(key, fTransformMap);
}

bool CustomPropertyManager::setTransform(const PropKey& key,
                                         const skottie::TransformPropertyValue& t) {
    return this->set(key, t, fTransformMap);
}

std::vector<CustomPropertyManager::PropKey>
CustomPropertyManager::getTextProps() const {
    return this->getProps(fTextMap);
}

skottie::TextPropertyValue CustomPropertyManager::getText(const PropKey& key) const {
    return this->get<skottie::TextPropertyValue>(key, fTextMap);
}

bool CustomPropertyManager::setText(const PropKey& key, const skottie::TextPropertyValue& o) {
    return this->set(key, o, fTextMap);
}

namespace {

class ExternalAnimationLayer final : public skottie::ExternalLayer {
public:
    ExternalAnimationLayer(sk_sp<skottie::Animation> anim, const SkSize& size)
        : fAnimation(std::move(anim))
        , fSize(size) {}

private:
    void render(SkCanvas* canvas, double t) override {
        fAnimation->seekFrameTime(t);

        // The main animation will layer-isolate if needed - we don't want the nested animation
        // to override that decision.
        const auto flags = skottie::Animation::RenderFlag::kSkipTopLevelIsolation;
        const auto dst_rect = SkRect::MakeSize(fSize);
        fAnimation->render(canvas, &dst_rect, flags);
    }

    const sk_sp<skottie::Animation> fAnimation;
    const SkSize                    fSize;
};

} // namespace

ExternalAnimationPrecompInterceptor::ExternalAnimationPrecompInterceptor(
        sk_sp<skresources::ResourceProvider> rprovider,
        const char prefixp[])
    : fResourceProvider(std::move(rprovider))
    , fPrefix(prefixp) {}

ExternalAnimationPrecompInterceptor::~ExternalAnimationPrecompInterceptor() = default;

sk_sp<skottie::ExternalLayer> ExternalAnimationPrecompInterceptor::onLoadPrecomp(
        const char[], const char name[], const SkSize& size) {
    if (0 != strncmp(name, fPrefix.c_str(), fPrefix.size())) {
        return nullptr;
    }

    auto data = fResourceProvider->load("", name + fPrefix.size());
    if (!data) {
        return nullptr;
    }

    auto anim = skottie::Animation::Builder()
                    .setPrecompInterceptor(sk_ref_sp(this))
                    .setResourceProvider(fResourceProvider)
                    .make(static_cast<const char*>(data->data()), data->size());

    return anim ? sk_make_sp<ExternalAnimationLayer>(std::move(anim), size)
                : nullptr;
}

/**
 * An implementation of ResourceProvider designed for Lottie template asset substitution (images,
 * audio, etc)
 */
class SlotManager::SlottableResourceProvider final : public skresources::ResourceProvider {
public:
    SlottableResourceProvider() {}

    sk_sp<skresources::ImageAsset> loadImageAsset(const char /*resource_path*/[],
                                                  const char slot_name[],
                                                  const char /*resource_id*/[]) const override {
        const auto it = fImageAssetMap.find(slot_name);
        return it == fImageAssetMap.end() ? nullptr : it->second;
    }

private:
    std::unordered_map<std::string, sk_sp<skresources::ImageAsset>> fImageAssetMap;

    friend class SlotManager;
};

/**
 * An implementation of PropertyObserver designed for Lottie template property substitution (color,
 * text, etc)
 *
 * PropertyObserver looks for slottable nodes then manipulates their PropertyValue on the fly
 *
 */
class SlotManager::SlottablePropertyObserver final : public skottie::PropertyObserver {
public:
    SlottablePropertyObserver() {}

    void onColorProperty(const char node_name[],
                         const LazyHandle<skottie::ColorPropertyHandle>& c) override {
        const auto it = fColorMap.find(node_name);
        if (it != fColorMap.end()) {
            c()->set(it->second);
        }
    }

    void onTextProperty(const char node_name[],
                        const LazyHandle<skottie::TextPropertyHandle>& t) override {
        const auto it = fTextMap.find(node_name);
        if (it != fTextMap.end()) {
            auto value = t()->get();
            value.fText = it->second;
            t()->set(value);
        }
    }

    // TODO(jmbetancourt): add support for other PropertyObserver callbacks
private:
    using SlotID = std::string;

    std::unordered_map<SlotID, skottie::ColorPropertyValue> fColorMap;
    std::unordered_map<SlotID, SkString>                    fTextMap;

    friend class SlotManager;
};

SlotManager::SlotManager() {
    fResourceProvider = sk_make_sp<SlottableResourceProvider>();
    fPropertyObserver = sk_make_sp<SlottablePropertyObserver>();
}

// TODO: consider having a generic setSlotMethod that is overloaded by PropertyValue type
void SlotManager::setColorSlot(std::string slotID, SkColor color) {
    fPropertyObserver->fColorMap[slotID] = color;
}

void SlotManager::setTextStringSlot(std::string slotID, SkString text) {
    fPropertyObserver->fTextMap[slotID] = std::move(text);
}

void SlotManager::setImageSlot(std::string slotID, sk_sp<skresources::ImageAsset> img) {
    fResourceProvider->fImageAssetMap[slotID] = std::move(img);
}

sk_sp<skresources::ResourceProvider> SlotManager::getResourceProvider() {
    return fResourceProvider;
}

sk_sp<skottie::PropertyObserver> SlotManager::getPropertyObserver() {
    return fPropertyObserver;
}

} // namespace skottie_utils
