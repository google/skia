/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkMakeUnique.h"
#include "SkTypes.h"
#include "Skottie.h"

#include <string>

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

class ManagedAnimation final : public SkRefCnt {
public:
    static sk_sp<ManagedAnimation> Make(const std::string& json) {
        auto mgr = skstd::make_unique<skottie_utils::CustomPropertyManager>();
        auto animation = skottie::Animation::Builder()
                            .setMarkerObserver(mgr->getMarkerObserver())
                            .setPropertyObserver(mgr->getPropertyObserver())
                            .make(json.c_str(), json.size());

        return animation
            ? sk_sp<ManagedAnimation>(new ManagedAnimation(std::move(animation), std::move(mgr)))
            : nullptr;
    }

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

    function("MakeManagedAnimation", &ManagedAnimation::Make);
    constant("managed_skottie", true);
#endif // SK_INCLUDE_MANAGED_SKOTTIE
}
