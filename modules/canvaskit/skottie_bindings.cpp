/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/sksg/include/SkSGInvalidationController.h"

#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>
#include "modules/canvaskit/WasmCommon.h"

#if SK_INCLUDE_MANAGED_SKOTTIE
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "modules/skresources/include/SkResources.h"
#endif // SK_INCLUDE_MANAGED_SKOTTIE

using namespace emscripten;

#if SK_INCLUDE_MANAGED_SKOTTIE
namespace {

// WebTrack wraps a JS object that has a 'seek' method.
// Playback logic is kept there.
class WebTrack final : public skresources::ExternalTrackAsset {
public:
    explicit WebTrack(emscripten::val player) : fPlayer(std::move(player)) {}

private:
    void seek(float t) override {
        fPlayer.call<void>("seek", val(t));
    }

    const emscripten::val fPlayer;
};

class SkottieAssetProvider : public skottie::ResourceProvider {
public:
    ~SkottieAssetProvider() override = default;

    // Tried using a map, but that gave strange errors like
    // https://emscripten.org/docs/porting/guidelines/function_pointer_issues.html
    // Not entirely sure why, but perhaps the iterator in the map was
    // confusing enscripten.
    using AssetVec = std::vector<std::pair<SkString, sk_sp<SkData>>>;

    static sk_sp<SkottieAssetProvider> Make(AssetVec assets, emscripten::val soundMap) {
        return sk_sp<SkottieAssetProvider>(new SkottieAssetProvider(std::move(assets),
                                                                    std::move(soundMap)));
    }

    sk_sp<skottie::ImageAsset> loadImageAsset(const char[] /* path */,
                                              const char name[],
                                              const char[] /* id */) const override {
        // For CK/Skottie we ignore paths & IDs, and identify images based solely on name.
        if (auto data = this->findAsset(name)) {
            return skresources::MultiFrameImageAsset::Make(std::move(data));
        }

        return nullptr;
    }

    sk_sp<skresources::ExternalTrackAsset> loadAudioAsset(const char[] /* path */,
                                                          const char[] /* name */,
                                                          const char id[]) override {
        emscripten::val player = this->findSoundAsset(id);
        if (player.as<bool>()) {
            return sk_make_sp<WebTrack>(std::move(player));
        }

        return nullptr;
    }

    sk_sp<SkData> loadFont(const char name[], const char[] /* url */) const override {
        // Same as images paths, we ignore font URLs.
        return this->findAsset(name);
    }

    sk_sp<SkData> load(const char[]/*path*/, const char name[]) const override {
        // Ignore paths.
        return this->findAsset(name);
    }

private:
    explicit SkottieAssetProvider(AssetVec assets, emscripten::val soundMap)
    : fAssets(std::move(assets))
    , fSoundMap(std::move(soundMap)) {}

    sk_sp<SkData> findAsset(const char name[]) const {
        for (const auto& asset : fAssets) {
            if (asset.first.equals(name)) {
                return asset.second;
            }
        }

        SkDebugf("Could not find %s\n", name);
        return nullptr;
    }

    emscripten::val findSoundAsset(const char name[]) const {
        if (fSoundMap.as<bool>() && fSoundMap.hasOwnProperty("getPlayer")) {
            emscripten::val player = fSoundMap.call<emscripten::val>("getPlayer", val(name));
            if (player.as<bool>() && player.hasOwnProperty("seek")) {
                return player;
            }
        }
        return emscripten::val::null();
    }

    const AssetVec fAssets;
    const emscripten::val fSoundMap;
};

// Wraps a JS object with 'onError' and 'onWarning' methods.
class JSLogger final : public skottie::Logger {
public:
    static sk_sp<JSLogger> Make(emscripten::val logger) {
        return logger.as<bool>()
            && logger.hasOwnProperty(kWrnFunc)
            && logger.hasOwnProperty(kErrFunc)
                ? sk_sp<JSLogger>(new JSLogger(std::move(logger)))
                : nullptr;
    }

private:
    explicit JSLogger(emscripten::val logger) : fLogger(std::move(logger)) {}

    void log(Level lvl, const char msg[], const char* json) override {
        const auto* func = lvl == Level::kError ? kErrFunc : kWrnFunc;
        fLogger.call<void>(func, std::string(msg), std::string(json));
    }

    static constexpr char kWrnFunc[] = "onWarning",
                          kErrFunc[] = "onError";

    const emscripten::val fLogger;
};

class ManagedAnimation final : public SkRefCnt {
public:
    static sk_sp<ManagedAnimation> Make(const std::string& json,
                                        sk_sp<skottie::ResourceProvider> rp,
                                        std::string prop_prefix,
                                        emscripten::val logger) {
        auto mgr = std::make_unique<skottie_utils::CustomPropertyManager>(
                        skottie_utils::CustomPropertyManager::Mode::kCollapseProperties,
                        prop_prefix.empty() ? nullptr : prop_prefix.c_str());
        static constexpr char kInterceptPrefix[] = "__";
        auto pinterceptor =
            sk_make_sp<skottie_utils::ExternalAnimationPrecompInterceptor>(rp, kInterceptPrefix);
        auto animation = skottie::Animation::Builder()
                            .setMarkerObserver(mgr->getMarkerObserver())
                            .setPropertyObserver(mgr->getPropertyObserver())
                            .setResourceProvider(std::move(rp))
                            .setPrecompInterceptor(std::move(pinterceptor))
                            .setLogger(JSLogger::Make(std::move(logger)))
                            .make(json.c_str(), json.size());

        return animation
            ? sk_sp<ManagedAnimation>(new ManagedAnimation(std::move(animation), std::move(mgr)))
            : nullptr;
    }

    ~ManagedAnimation() override = default;

    // skottie::Animation API
    void render(SkCanvas* canvas, const SkRect* dst) const { fAnimation->render(canvas, dst); }
    // Returns a damage rect.
    SkRect seek(SkScalar t) {
        sksg::InvalidationController ic;
        fAnimation->seek(t, &ic);
        return ic.bounds();
    }
    // Returns a damage rect.
    SkRect seekFrame(double t) {
        sksg::InvalidationController ic;
        fAnimation->seekFrame(t, &ic);
        return ic.bounds();
    }
    double duration() const { return fAnimation->duration(); }
    double fps() const { return fAnimation->fps(); }
    const SkSize& size() const { return fAnimation->size(); }
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

    JSArray getTextProps() const {
        JSArray props = emscripten::val::array();

        for (const auto& key : fPropMgr->getTextProps()) {
            const auto txt = fPropMgr->getText(key);
            JSObject txt_val = emscripten::val::object();
            txt_val.set("text", txt.fText.c_str());
            txt_val.set("size", txt.fTextSize);

            JSObject prop = emscripten::val::object();
            prop.set("key", key);
            prop.set("value", std::move(txt_val));

            props.call<void>("push", prop);
        }

        return props;
    }

    bool setColor(const std::string& key, SkColor c) {
        return fPropMgr->setColor(key, c);
    }

    bool setOpacity(const std::string& key, float o) {
        return fPropMgr->setOpacity(key, o);
    }

    bool setText(const std::string& key, std::string text, float size) {
        // preserve all other text fields
        auto t = fPropMgr->getText(key);

        t.fText     = SkString(text);
        t.fTextSize = size;

        return fPropMgr->setText(key, t);
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
        , fPropMgr(std::move(propMgr))
    {}

    const sk_sp<skottie::Animation>                             fAnimation;
    const std::unique_ptr<skottie_utils::CustomPropertyManager> fPropMgr;
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
        .function("_size", optional_override([](skottie::Animation& self,
                                                uintptr_t /* float* */ oPtr)->void {
            SkSize* output = reinterpret_cast<SkSize*>(oPtr);
            *output = self.size();
        }))
        .function("duration", &skottie::Animation::duration)
        .function("fps"     , &skottie::Animation::fps)
        .function("seek", optional_override([](skottie::Animation& self, SkScalar t)->void {
            self.seek(t);
        }))
        .function("seekFrame", optional_override([](skottie::Animation& self, double t)->void {
            self.seekFrame(t);
        }))
        .function("_render", optional_override([](skottie::Animation& self, SkCanvas* canvas,
                                                  uintptr_t /* float* */ fPtr)->void {
            const SkRect* dst = reinterpret_cast<const SkRect*>(fPtr);
            self.render(canvas, dst);
        }), allow_raw_pointers());

    function("MakeAnimation", optional_override([](std::string json)->sk_sp<skottie::Animation> {
        return skottie::Animation::Make(json.c_str(), json.length());
    }));
    constant("skottie", true);

#if SK_INCLUDE_MANAGED_SKOTTIE
    class_<ManagedAnimation>("ManagedAnimation")
        .smart_ptr<sk_sp<ManagedAnimation>>("sk_sp<ManagedAnimation>")
        .function("version"   , &ManagedAnimation::version)
        .function("_size", optional_override([](ManagedAnimation& self,
                                                uintptr_t /* float* */ oPtr)->void {
            SkSize* output = reinterpret_cast<SkSize*>(oPtr);
            *output = self.size();
        }))
        .function("duration"  , &ManagedAnimation::duration)
        .function("fps"       , &ManagedAnimation::fps)
        .function("_render", optional_override([](ManagedAnimation& self, SkCanvas* canvas,
                                                  uintptr_t /* float* */ fPtr)->void {
            const SkRect* dst = reinterpret_cast<const SkRect*>(fPtr);
            self.render(canvas, dst);
        }), allow_raw_pointers())
        .function("_seek", optional_override([](ManagedAnimation& self, SkScalar t,
                                                uintptr_t /* float* */ fPtr) {
            SkRect* damageRect = reinterpret_cast<SkRect*>(fPtr);
            damageRect[0] = self.seek(t);
        }))
        .function("_seekFrame", optional_override([](ManagedAnimation& self, double frame,
                                                     uintptr_t /* float* */ fPtr) {
            SkRect* damageRect = reinterpret_cast<SkRect*>(fPtr);
            damageRect[0] = self.seekFrame(frame);
        }))
        .function("seekFrame" , &ManagedAnimation::seekFrame)
        .function("_setColor"  , optional_override([](ManagedAnimation& self, const std::string& key, uintptr_t /* float* */ cPtr) {
            float* fourFloats = reinterpret_cast<float*>(cPtr);
            SkColor4f color = { fourFloats[0], fourFloats[1], fourFloats[2], fourFloats[3] };
            return self.setColor(key, color.toSkColor());
        }))
        .function("setOpacity", &ManagedAnimation::setOpacity)
        .function("getMarkers", &ManagedAnimation::getMarkers)
        .function("getColorProps"  , &ManagedAnimation::getColorProps)
        .function("getOpacityProps", &ManagedAnimation::getOpacityProps)
        .function("getTextProps"   , &ManagedAnimation::getTextProps)
        .function("setText"        , &ManagedAnimation::setText);

    function("_MakeManagedAnimation", optional_override([](std::string json,
                                                           size_t assetCount,
                                                           uintptr_t /* char**     */ nptr,
                                                           uintptr_t /* uint8_t**  */ dptr,
                                                           uintptr_t /* size_t*    */ sptr,
                                                           std::string prop_prefix,
                                                           emscripten::val soundMap,
                                                           emscripten::val logger)
                                                        ->sk_sp<ManagedAnimation> {
        // See the comment in canvaskit_bindings.cpp about the use of uintptr_t
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

        return ManagedAnimation::Make(json,
                                      skresources::DataURIResourceProviderProxy::Make(
                                          SkottieAssetProvider::Make(std::move(assets),
                                                                     std::move(soundMap))),
                                      prop_prefix, std::move(logger));
    }));
    constant("managed_skottie", true);
#endif // SK_INCLUDE_MANAGED_SKOTTIE
}
