/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkImage.h"
#include "SkMakeUnique.h"
#include "SkTypes.h"
#include "SkString.h"
#include "Skottie.h"

#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>
#include "WasmAliases.h"

#if SK_INCLUDE_MANAGED_SKOTTIE
#include "SkottieProperty.h"
#include "SkottieUtils.h"
#endif // SK_INCLUDE_MANAGED_SKOTTIE

using namespace emscripten;

#if SK_INCLUDE_MANAGED_SKOTTIE
namespace {

class SkottieAssetProvider : public skottie::ResourceProvider {
public:
    ~SkottieAssetProvider() override = default;

    // Tried using a map, but that gave strange errors like
    // https://emscripten.org/docs/porting/guidelines/function_pointer_issues.html
    // Not entirely sure why, but perhaps the iterator in the map was
    // confusing enscripten.
    using AssetVec = std::vector<std::pair<SkString, sk_sp<SkData>>>;

    static sk_sp<SkottieAssetProvider> Make(AssetVec assets) {
        if (assets.empty()) {
            return nullptr;
        }

        return sk_sp<SkottieAssetProvider>(new SkottieAssetProvider(std::move(assets)));
    }

    sk_sp<skottie::ImageAsset> loadImageAsset(const char[] /* path */,
                                              const char name[]) const override {
        // For CK/Skottie we ignore paths and identify images based solely on name.
        if (auto data = this->findAsset(name)) {
            return skottie_utils::MultiFrameImageAsset::Make(std::move(data));
        }

        return nullptr;
    }

    sk_sp<SkData> loadFont(const char name[], const char[] /* url */) const override {
        // Same as images paths, we ignore font URLs.
        return this->findAsset(name);
    }

private:
    explicit SkottieAssetProvider(AssetVec assets) : fAssets(std::move(assets)) {}

    sk_sp<SkData> findAsset(const char name[]) const {
        for (const auto& asset : fAssets) {
            if (asset.first.equals(name)) {
                return asset.second;
            }
        }

        SkDebugf("Could not find %s\n", name);
        return nullptr;
    }

    const AssetVec fAssets;
};

class ManagedAnimation final : public SkRefCnt {
public:
    static sk_sp<ManagedAnimation> Make(const std::string& json, sk_sp<SkottieAssetProvider> ap) {
        auto mgr = skstd::make_unique<skottie_utils::CustomPropertyManager>();
        auto animation = skottie::Animation::Builder()
                            .setMarkerObserver(mgr->getMarkerObserver())
                            .setPropertyObserver(mgr->getPropertyObserver())
                            .setResourceProvider(ap)
                            .make(json.c_str(), json.size());

        return animation
            ? sk_sp<ManagedAnimation>(new ManagedAnimation(std::move(animation), std::move(mgr)))
            : nullptr;
    }

    ~ManagedAnimation() override = default;

    // skottie::Animation API
    void render(SkCanvas* canvas) const { fAnimation->render(canvas, nullptr); }
    void render(SkCanvas* canvas, const SkRect& dst) const { fAnimation->render(canvas, &dst); }
    void seek(SkScalar t) { fAnimation->seek(t); }
    SkScalar duration() const { return fAnimation->duration(); }
    const SkSize&      size() const { return fAnimation->size(); }
    std::string version() const { return std::string(fAnimation->version().c_str()); }

    // CustomPropertyManager API
    JSArray getColorProps() const {
        JSArray props = emscripten::val::array();

        for (const auto& cp : fPropMgr->getColorProps()) {
            JSObject prop = emscripten::val::object();
            prop.set("key", cp);
            prop.set("value", fPropMgr->getColor(cp));
            props.call<void>("push", prop);
        }

        return props;
    }

    JSArray getOpacityProps() const {
        JSArray props = emscripten::val::array();

        for (const auto& op : fPropMgr->getOpacityProps()) {
            JSObject prop = emscripten::val::object();
            prop.set("key", op);
            prop.set("value", fPropMgr->getOpacity(op));
            props.call<void>("push", prop);
        }

        return props;
    }

    bool setColor(const std::string& key, JSColor c) {
        return fPropMgr->setColor(key, static_cast<SkColor>(c));
    }

    bool setOpacity(const std::string& key, float o) {
        return fPropMgr->setOpacity(key, o);
    }

    JSArray getMarkers() const {
        JSArray markers = emscripten::val::array();
        for (const auto& m : fPropMgr->markers()) {
            JSObject marker = emscripten::val::object();
            marker.set("name", m.name);
            marker.set("t0"  , m.t0);
            marker.set("t1"  , m.t1);
            markers.call<void>("push", marker);
        }
        return markers;
    }

private:
    ManagedAnimation(sk_sp<skottie::Animation> animation,
                     std::unique_ptr<skottie_utils::CustomPropertyManager> propMgr)
        : fAnimation(std::move(animation))
        , fPropMgr(std::move(propMgr)) {}

    sk_sp<skottie::Animation>                             fAnimation;
    std::unique_ptr<skottie_utils::CustomPropertyManager> fPropMgr;
};

} // anonymous ns
#endif // SK_INCLUDE_MANAGED_SKOTTIE

EMSCRIPTEN_BINDINGS(Skottie) {
    // Animation things (may eventually go in own library)
    class_<skottie::Animation>("Animation")
        .smart_ptr<sk_sp<skottie::Animation>>("sk_sp<Animation>")
        .function("version", optional_override([](skottie::Animation& self)->std::string {
            return std::string(self.version().c_str());
        }))
        .function("size", &skottie::Animation::size)
        .function("duration", &skottie::Animation::duration)
        .function("seek", &skottie::Animation::seek)
        .function("render", optional_override([](skottie::Animation& self, SkCanvas* canvas)->void {
            self.render(canvas, nullptr);
        }), allow_raw_pointers())
        .function("render", optional_override([](skottie::Animation& self, SkCanvas* canvas,
                                                 const SkRect r)->void {
            self.render(canvas, &r);
        }), allow_raw_pointers());

    function("MakeAnimation", optional_override([](std::string json)->sk_sp<skottie::Animation> {
        return skottie::Animation::Make(json.c_str(), json.length());
    }));
    constant("skottie", true);

#if SK_INCLUDE_MANAGED_SKOTTIE
    class_<ManagedAnimation>("ManagedAnimation")
        .smart_ptr<sk_sp<ManagedAnimation>>("sk_sp<ManagedAnimation>")
        .function("version"   , &ManagedAnimation::version)
        .function("size"      , &ManagedAnimation::size)
        .function("duration"  , &ManagedAnimation::duration)
        .function("seek"      , &ManagedAnimation::seek)
        .function("render"    , select_overload<void(SkCanvas*) const>(&ManagedAnimation::render), allow_raw_pointers())
        .function("render"    , select_overload<void(SkCanvas*, const SkRect&) const>
                                    (&ManagedAnimation::render), allow_raw_pointers())
        .function("setColor"  , &ManagedAnimation::setColor)
        .function("setOpacity", &ManagedAnimation::setOpacity)
        .function("getMarkers", &ManagedAnimation::getMarkers)
        .function("getColorProps"  , &ManagedAnimation::getColorProps)
        .function("getOpacityProps", &ManagedAnimation::getOpacityProps);

    function("_MakeManagedAnimation", optional_override([](std::string json,
                                                           size_t assetCount,
                                                           uintptr_t /* char**     */ nptr,
                                                           uintptr_t /* uint8_t**  */ dptr,
                                                           uintptr_t /* size_t*    */ sptr)
                                                        ->sk_sp<ManagedAnimation> {
        const auto assetNames = reinterpret_cast<char**   >(nptr);
        const auto assetDatas = reinterpret_cast<uint8_t**>(dptr);
        const auto assetSizes = reinterpret_cast<size_t*  >(sptr);

        SkottieAssetProvider::AssetVec assets;
        assets.reserve(assetCount);

        for (size_t i = 0; i < assetCount; i++) {
            auto name  = SkString(assetNames[i]);
            auto bytes = SkData::MakeFromMalloc(assetDatas[i], assetSizes[i]);
            assets.push_back(std::make_pair(std::move(name), std::move(bytes)));
        }

        return ManagedAnimation::Make(json, SkottieAssetProvider::Make(std::move(assets)));
    }));
    constant("managed_skottie", true);
#endif // SK_INCLUDE_MANAGED_SKOTTIE
}
