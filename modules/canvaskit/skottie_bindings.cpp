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
#include "modules/canvaskit/WasmCommon.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "modules/skottie/utils/TextEditor.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/sksg/include/SkSGInvalidationController.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/base/SkUTF.h"
#include "tools/skui/InputState.h"
#include "tools/skui/ModifierKey.h"

#include <string>
#include <vector>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;
namespace para = skia::textlayout;
namespace {

struct SimpleSlottableTextProperty {
    sk_sp<SkTypeface> typeface;
    std::string text;

    float textSize;
    float minTextSize;
    float maxTextSize;
    float strokeWidth;
    float lineHeight;
    float lineShift;
    float ascent;
    float maxLines;

    para::TextAlign horizAlign;
    skottie::Shaper::VAlign vertAlign;
    skottie::Shaper::ResizePolicy resize;
    SkUnicode::LineBreakType lineBreak;
    para::TextDirection direction;
    SkPaint::Join strokeJoin;

    WASMPointerF32 boundingBoxPtr;
    WASMPointerF32 fillColorPtr;
    WASMPointerF32 strokeColorPtr;

    operator skottie::TextPropertyValue() const {
        skottie::TextPropertyValue textProperty;

        textProperty.fTypeface = this->typeface;
        textProperty.fText = SkString(this->text);
        textProperty.fTextSize = this->textSize;
        textProperty.fMinTextSize = this->minTextSize;
        textProperty.fMaxTextSize = this->maxTextSize;
        textProperty.fStrokeWidth = this->strokeWidth;
        textProperty.fLineHeight = this->lineHeight;
        textProperty.fLineShift = this->lineShift;
        textProperty.fAscent = this->ascent;
        textProperty.fMaxLines = this->maxLines;

        switch (this->horizAlign) {
        case para::TextAlign::kLeft:
            textProperty.fHAlign = SkTextUtils::Align::kLeft_Align;
            break;
        case para::TextAlign::kCenter:
            textProperty.fHAlign = SkTextUtils::Align::kCenter_Align;
            break;
        case para::TextAlign::kRight:
            textProperty.fHAlign = SkTextUtils::Align::kRight_Align;
            break;
        default:
            textProperty.fHAlign = SkTextUtils::Align::kLeft_Align;
            break;
        }

        textProperty.fVAlign = this->vertAlign;
        textProperty.fResize = this->resize;

        if (this->lineBreak == SkUnicode::LineBreakType::kSoftLineBreak) {
            textProperty.fLineBreak = skottie::Shaper::LinebreakPolicy::kParagraph;
        } else {
            textProperty.fLineBreak = skottie::Shaper::LinebreakPolicy::kExplicit;
        }

        if (this->direction == para::TextDirection::kRtl) {
            textProperty.fDirection = skottie::Shaper::Direction::kRTL;
        } else {
            textProperty.fDirection = skottie::Shaper::Direction::kLTR;
        }

        textProperty.fStrokeJoin = this->strokeJoin;

        textProperty.fBox = reinterpret_cast<SkRect*>(this->boundingBoxPtr)[0];
        textProperty.fFillColor = ptrToSkColor4f(this->fillColorPtr).toSkColor();
        textProperty.fStrokeColor = ptrToSkColor4f(this->strokeColorPtr).toSkColor();

        return textProperty;
    }
};

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

    inline static constexpr char kWrnFunc[] = "onWarning",
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
                        prop_prefix.c_str());
        static constexpr char kInterceptPrefix[] = "__";
        auto pinterceptor =
            sk_make_sp<skottie_utils::ExternalAnimationPrecompInterceptor>(rp, kInterceptPrefix);
        skottie::Animation::Builder builder;
        builder.setMarkerObserver(mgr->getMarkerObserver())
               .setPropertyObserver(mgr->getPropertyObserver())
               .setResourceProvider(rp)
               .setPrecompInterceptor(std::move(pinterceptor))
               .setLogger(JSLogger::Make(std::move(logger)));
        auto animation = builder.make(json.c_str(), json.size());
        auto slotManager = builder.getSlotManager();

        return animation
            ? sk_sp<ManagedAnimation>(new ManagedAnimation(std::move(animation), std::move(mgr),
                                                           std::move(slotManager), std::move(rp)))
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

    JSArray getTransformProps() const {
        JSArray props = emscripten::val::array();

        for (const auto& key : fPropMgr->getTransformProps()) {
            const auto transform = fPropMgr->getTransform(key);
            JSObject trans_val = emscripten::val::object();
            const float anchor[] = {transform.fAnchorPoint.fX, transform.fAnchorPoint.fY};
            const float position[] = {transform.fPosition.fX, transform.fPosition.fY};
            const float scale[] = {transform.fScale.fX, transform.fScale.fY};
            trans_val.set("anchor", MakeTypedArray(2, anchor));
            trans_val.set("position", MakeTypedArray(2, position));
            trans_val.set("scale", MakeTypedArray(2, scale));
            trans_val.set("rotation", transform.fRotation);
            trans_val.set("skew", transform.fSkew);
            trans_val.set("skew_axis", transform.fSkewAxis);

            JSObject prop = emscripten::val::object();
            prop.set("key", key);
            prop.set("value", trans_val);
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

    bool setTransform(const std::string& key, SkScalar anchorX, SkScalar anchorY,
                                              SkScalar posX, SkScalar posY,
                                              SkScalar scaleX, SkScalar scaleY,
                                              SkScalar rotation, SkScalar skew, SkScalar skewAxis) {
        skottie::TransformPropertyValue transform;
        transform.fAnchorPoint = {anchorX, anchorY};
        transform.fPosition = {posX, posY};
        transform.fScale = {scaleX, scaleY};
        transform.fRotation = rotation;
        transform.fSkew = skew;
        transform.fSkewAxis = skewAxis;
        return fPropMgr->setTransform(key, transform);
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

    JSArray copyStringArrayToJSArray(skia_private::TArray<SkString> slotIDs) const {
        JSArray retVal = emscripten::val::array();
        for (auto slotID : slotIDs) {
            retVal.call<void>("push", emscripten::val(slotID.c_str()));
        }
        return retVal;
    }

    // Slot Manager API
    JSObject getSlotInfo() const {
        JSObject slotInfoJS = emscripten::val::object();
        auto slotInfo = fSlotMgr->getSlotInfo();

        slotInfoJS.set("colorSlotIDs", copyStringArrayToJSArray(slotInfo.fColorSlotIDs));
        slotInfoJS.set("scalarSlotIDs", copyStringArrayToJSArray(slotInfo.fScalarSlotIDs));
        slotInfoJS.set("vec2SlotIDs", copyStringArrayToJSArray(slotInfo.fVec2SlotIDs));
        slotInfoJS.set("imageSlotIDs", copyStringArrayToJSArray(slotInfo.fImageSlotIDs));
        slotInfoJS.set("textSlotIDs", copyStringArrayToJSArray(slotInfo.fTextSlotIDs));

        return slotInfoJS;
    }

    void getColorSlot(const std::string& slotID, WASMPointerF32 outPtr) {
        SkColor4f c4f;
        if (auto c = fSlotMgr->getColorSlot(SkString(slotID))) {
            c4f = SkColor4f::FromColor(*c);
        } else {
            c4f = {-1, -1, -1, -1};
        }
        memcpy(reinterpret_cast<float*>(outPtr), &c4f, 4 * sizeof(float));
    }

    emscripten::val getScalarSlot(const std::string& slotID) {
        if (auto s = fSlotMgr->getScalarSlot(SkString(slotID))) {
           return emscripten::val(*s);
        }
        return emscripten::val::null();
    }

    void getVec2Slot(const std::string& slotID, WASMPointerF32 outPtr) {
        // [x, y, sentinel]
        SkV3 vec3;
        if (auto v = fSlotMgr->getVec2Slot(SkString(slotID))) {
            vec3 = {v->x, v->y, 1};
        } else {
            vec3 = {0, 0, -1};
        }
        memcpy(reinterpret_cast<float*>(outPtr), vec3.ptr(), 3 * sizeof(float));
    }

    JSObject getTextSlot(const std::string& slotID) const {
        if (auto textProp = fSlotMgr->getTextSlot(SkString(slotID))){
            JSObject text_val = emscripten::val::object();

            text_val.set("typeface", textProp->fTypeface);
            text_val.set("text", emscripten::val(textProp->fText.c_str()));
            text_val.set("textSize", textProp->fTextSize);
            text_val.set("minTextSize", textProp->fMinTextSize);
            text_val.set("maxTextSize", textProp->fMaxTextSize);
            text_val.set("strokeWidth", textProp->fStrokeWidth);
            text_val.set("lineHeight", textProp->fLineHeight);
            text_val.set("lineShift", textProp->fLineShift);
            text_val.set("ascent", textProp->fAscent);
            text_val.set("maxLines", textProp->fMaxLines);

            switch (textProp->fHAlign) {
            case SkTextUtils::Align::kLeft_Align:
                text_val.set("horizAlign", para::TextAlign::kLeft);
                break;
            case SkTextUtils::Align::kRight_Align:
                text_val.set("horizAlign", para::TextAlign::kRight);
                break;
            case SkTextUtils::Align::kCenter_Align:
                text_val.set("horizAlign", para::TextAlign::kCenter);
                break;
            default:
                text_val.set("horizAlign", para::TextAlign::kLeft);
                break;
            }

            text_val.set("vertAlign", textProp->fVAlign);
            text_val.set("resize", textProp->fResize);

            if (textProp->fLineBreak == skottie::Shaper::LinebreakPolicy::kParagraph) {
                text_val.set("linebreak", SkUnicode::LineBreakType::kSoftLineBreak);
            } else {
                text_val.set("linebreak", SkUnicode::LineBreakType::kHardLineBreak);
            }

            if (textProp->fDirection == skottie::Shaper::Direction::kLTR) {
                text_val.set("direction", para::TextDirection::kLtr);
            } else {
                text_val.set("direction", para::TextDirection::kRtl);
            }
            text_val.set("strokeJoin", textProp->fStrokeJoin);

            text_val.set("fillColor", MakeTypedArray(4, SkColor4f::FromColor(textProp->fFillColor)
                                                            .vec()));

            text_val.set("strokeColor", MakeTypedArray(4, SkColor4f::FromColor(textProp->fStrokeColor)
                                                            .vec()));

            const float box[] = {textProp->fBox.fLeft, textProp->fBox.fTop,
                                 textProp->fBox.fRight, textProp->fBox.fBottom};
            text_val.set("boundingBox", MakeTypedArray(4, box));
            return text_val;
        }
        return emscripten::val::null();
    }

    bool setImageSlot(const std::string& slotID, const std::string& assetName) {
        // look for resource in preloaded SkottieAssetProvider
        return fSlotMgr->setImageSlot(SkString(slotID), fResourceProvider->loadImageAsset(nullptr,
                                                                            assetName.data(),
                                                                            nullptr));
    }

    bool setColorSlot(const std::string& slotID, SkColor c) {
        return fSlotMgr->setColorSlot(SkString(slotID), c);
    }

    bool setScalarSlot(const std::string& slotID, float s) {
        return fSlotMgr->setScalarSlot(SkString(slotID), s);
    }

    bool setVec2Slot(const std::string& slotID, SkV2 v) {
        return fSlotMgr->setVec2Slot(SkString(slotID), v);
    }

    bool attachEditor(const std::string& layerID, size_t layerIndex) {
        if (fTextEditor) {
            fTextEditor->setEnabled(false);
            fTextEditor = nullptr;
        }

        if (layerID.empty()) {
            return true;
        }

        auto txt_handle = fPropMgr->getTextHandle(layerID, layerIndex);
        if (!txt_handle) {
            return false;
        }

        std::vector<std::unique_ptr<skottie::TextPropertyHandle>> deps;
        for (size_t i = 0; ; ++i) {
            if (i == layerIndex) {
                continue;
            }

            auto dep_handle = fPropMgr->getTextHandle(layerID, i);
            if (!dep_handle) {
                break;
            }
            deps.push_back(std::move(dep_handle));
        }

        fTextEditor = sk_make_sp<skottie_utils::TextEditor>(std::move(txt_handle),
                                                            std::move(deps));
        return true;
    }

    void enableEditor(bool enable) {
        if (fTextEditor) {
            fTextEditor->setEnabled(enable);
        }
    }

    bool dispatchEditorKey(const std::string& key) {
        // Map some useful keys to the current (odd) text editor bindings.
        // TODO: Add support for custom bindings in the editor.
        auto key2char = [](const std::string& key) -> SkUnichar {
            // Special keys.
            if (key == "ArrowLeft")  return '[';
            if (key == "ArrowRight") return ']';
            if (key == "Backspace")  return '\\';

            const char* str = key.c_str();
            const char* end = str + key.size();
            const SkUnichar uch = SkUTF::NextUTF8(&str, end);

            // Pass through single code points, ignore everything else.
            return str == end ? uch : -1;
        };

        if (fTextEditor) {
            const auto uch = key2char(key);
            if (uch != -1) {
                return fTextEditor->onCharInput(uch);
            }
        }

        return false;
    }

    bool dispatchEditorPointer(float x, float y, skui::InputState state, skui::ModifierKey mod) {
        return fTextEditor
                ? fTextEditor->onMouseInput(x, y, state, mod)
                : false;
    }

    void setEditorCursorWeight(float w) {
        if (fTextEditor) {
            fTextEditor->setCursorWeight(w);
        }
    }

    bool setTextSlot(const std::string& slotID, SimpleSlottableTextProperty t) {
        return fSlotMgr->setTextSlot(SkString(slotID), t);
    }

private:
    ManagedAnimation(sk_sp<skottie::Animation> animation,
                     std::unique_ptr<skottie_utils::CustomPropertyManager> propMgr,
                     sk_sp<skottie::SlotManager> slotMgr,
                     sk_sp<skresources::ResourceProvider> rp)
        : fAnimation(std::move(animation))
        , fPropMgr(std::move(propMgr))
        , fSlotMgr(std::move(slotMgr))
        , fResourceProvider(std::move(rp))
    {}

    const sk_sp<skottie::Animation>                             fAnimation;
    const std::unique_ptr<skottie_utils::CustomPropertyManager> fPropMgr;
    const sk_sp<skottie::SlotManager>                           fSlotMgr;
    const sk_sp<skresources::ResourceProvider>                  fResourceProvider;

    sk_sp<skottie_utils::TextEditor>                            fTextEditor;
};

} // anonymous ns

EMSCRIPTEN_BINDINGS(Skottie) {
    // Animation things (may eventually go in own library)
    class_<skottie::Animation>("Animation")
        .smart_ptr<sk_sp<skottie::Animation>>("sk_sp<Animation>")
        .function("version", optional_override([](skottie::Animation& self)->std::string {
            return std::string(self.version().c_str());
        }))
        .function("_size", optional_override([](skottie::Animation& self,
                                                WASMPointerF32 oPtr)->void {
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
                                                  WASMPointerF32 fPtr)->void {
            const SkRect* dst = reinterpret_cast<const SkRect*>(fPtr);
            self.render(canvas, dst);
        }), allow_raw_pointers());

    function("MakeAnimation", optional_override([](std::string json)->sk_sp<skottie::Animation> {
        return skottie::Animation::Make(json.c_str(), json.length());
    }));
    constant("skottie", true);

    class_<ManagedAnimation>("ManagedAnimation")
        .smart_ptr<sk_sp<ManagedAnimation>>("sk_sp<ManagedAnimation>")
        .function("version"   , &ManagedAnimation::version)
        .function("_size", optional_override([](ManagedAnimation& self,
                                                WASMPointerF32 oPtr)->void {
            SkSize* output = reinterpret_cast<SkSize*>(oPtr);
            *output = self.size();
        }))
        .function("duration"  , &ManagedAnimation::duration)
        .function("fps"       , &ManagedAnimation::fps)
        .function("_render", optional_override([](ManagedAnimation& self, SkCanvas* canvas,
                                                  WASMPointerF32 fPtr)->void {
            const SkRect* dst = reinterpret_cast<const SkRect*>(fPtr);
            self.render(canvas, dst);
        }), allow_raw_pointers())
        .function("_seek", optional_override([](ManagedAnimation& self, SkScalar t,
                                                WASMPointerF32 fPtr) {
            SkRect* damageRect = reinterpret_cast<SkRect*>(fPtr);
            damageRect[0] = self.seek(t);
        }))
        .function("_seekFrame", optional_override([](ManagedAnimation& self, double frame,
                                                     WASMPointerF32 fPtr) {
            SkRect* damageRect = reinterpret_cast<SkRect*>(fPtr);
            damageRect[0] = self.seekFrame(frame);
        }))
        .function("seekFrame" , &ManagedAnimation::seekFrame)
        .function("_setColor"  , optional_override([](ManagedAnimation& self, const std::string& key, WASMPointerF32 cPtr) {
            float* fourFloats = reinterpret_cast<float*>(cPtr);
            SkColor4f color = { fourFloats[0], fourFloats[1], fourFloats[2], fourFloats[3] };
            return self.setColor(key, color.toSkColor());
        }))
        .function("_setTransform"  , optional_override([](ManagedAnimation& self,
                                                          const std::string& key,
                                                          WASMPointerF32 transformData) {
            // transform value info is passed in as an array of 9 scalars in the following order:
            // anchor xy, position xy, scalexy, rotation, skew, skew axis
            auto transform = reinterpret_cast<SkScalar*>(transformData);
            return self.setTransform(key, transform[0], transform[1], transform[2], transform[3],
                                     transform[4], transform[5], transform[6], transform[7], transform[8]);
                                                          }))
        .function("getMarkers"       , &ManagedAnimation::getMarkers)
        .function("getColorProps"    , &ManagedAnimation::getColorProps)
        .function("getOpacityProps"  , &ManagedAnimation::getOpacityProps)
        .function("setOpacity"       , &ManagedAnimation::setOpacity)
        .function("getTextProps"     , &ManagedAnimation::getTextProps)
        .function("setText"          , &ManagedAnimation::setText)
        .function("getTransformProps", &ManagedAnimation::getTransformProps)
        .function("getSlotInfo"      , &ManagedAnimation::getSlotInfo)
        .function("_getColorSlot"    , &ManagedAnimation::getColorSlot)
        .function("_setColorSlot"    , optional_override([](ManagedAnimation& self, const std::string& key, WASMPointerF32 cPtr) {
            SkColor4f color = ptrToSkColor4f(cPtr);
            return self.setColorSlot(key, color.toSkColor());
        }))
        .function("_getVec2Slot"    , &ManagedAnimation::getVec2Slot)
        .function("_setVec2Slot"    , optional_override([](ManagedAnimation& self, const std::string& key, WASMPointerF32 vPtr) {
            float* twoFloats = reinterpret_cast<float*>(vPtr);
            SkV2 vec2 = {twoFloats[0], twoFloats[1]};
            return self.setVec2Slot(key, vec2);
        }))
        .function("getScalarSlot"    , &ManagedAnimation::getScalarSlot)
        .function("setScalarSlot"    , &ManagedAnimation::setScalarSlot)
        .function("attachEditor"         , &ManagedAnimation::attachEditor)
        .function("enableEditor"         , &ManagedAnimation::enableEditor)
        .function("dispatchEditorKey"    , &ManagedAnimation::dispatchEditorKey)
        .function("dispatchEditorPointer", &ManagedAnimation::dispatchEditorPointer)
        .function("setEditorCursorWeight", &ManagedAnimation::setEditorCursorWeight)
        .function("getTextSlot"      , &ManagedAnimation::getTextSlot)
        .function("_setTextSlot"     , &ManagedAnimation::setTextSlot)
        .function("setImageSlot"     , &ManagedAnimation::setImageSlot);

    function("_MakeManagedAnimation", optional_override([](std::string json,
                                                           size_t assetCount,
                                                           WASMPointerU32 nptr,
                                                           WASMPointerU32 dptr,
                                                           WASMPointerU32 sptr,
                                                           std::string prop_prefix,
                                                           emscripten::val soundMap,
                                                           emscripten::val logger)
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

        return ManagedAnimation::Make(json,
                                      skresources::DataURIResourceProviderProxy::Make(
                                          SkottieAssetProvider::Make(std::move(assets),
                                                                     std::move(soundMap))),
                                      prop_prefix, std::move(logger));
    }));

    enum_<skui::InputState>("InputState")
        .value("Down",  skui::InputState::kDown)
        .value("Up",    skui::InputState::kUp)
        .value("Move",  skui::InputState::kMove)
        .value("Right", skui::InputState::kRight)
        .value("Left",  skui::InputState::kLeft);

    enum_<skui::ModifierKey>("ModifierKey")
        .value("None",       skui::ModifierKey::kNone)
        .value("Shift",      skui::ModifierKey::kShift)
        .value("Control",    skui::ModifierKey::kControl)
        .value("Option",     skui::ModifierKey::kOption)
        .value("Command",    skui::ModifierKey::kCommand)
        .value("FirstPress", skui::ModifierKey::kFirstPress);

    enum_<skottie::Shaper::VAlign>("VerticalTextAlign")
        .value("Top",          skottie::Shaper::VAlign::kTop)
        .value("TopBaseline",  skottie::Shaper::VAlign::kTopBaseline)
        .value("VisualTop",    skottie::Shaper::VAlign::kVisualTop)
        .value("VisualCenter", skottie::Shaper::VAlign::kVisualCenter)
        .value("VisualBottom", skottie::Shaper::VAlign::kVisualBottom);

    enum_<skottie::Shaper::ResizePolicy>("ResizePolicy")
        .value("None",           skottie::Shaper::ResizePolicy::kNone)
        .value("ScaleToFit",     skottie::Shaper::ResizePolicy::kScaleToFit)
        .value("DownscaleToFit", skottie::Shaper::ResizePolicy::kDownscaleToFit);

    value_object<SimpleSlottableTextProperty>("SlottableTextProperty")
        .field("typeface",          &SimpleSlottableTextProperty::typeface)
        .field("text",              &SimpleSlottableTextProperty::text)
        .field("textSize",          &SimpleSlottableTextProperty::textSize)
        .field("minTextSize",       &SimpleSlottableTextProperty::minTextSize)
        .field("maxTextSize",       &SimpleSlottableTextProperty::maxTextSize)
        .field("strokeWidth",       &SimpleSlottableTextProperty::strokeWidth)
        .field("lineHeight",        &SimpleSlottableTextProperty::lineHeight)
        .field("lineShift",         &SimpleSlottableTextProperty::lineShift)
        .field("ascent",            &SimpleSlottableTextProperty::ascent)
        .field("maxLines",          &SimpleSlottableTextProperty::maxLines)
        .field("horizAlign",        &SimpleSlottableTextProperty::horizAlign)
        .field("vertAlign",         &SimpleSlottableTextProperty::vertAlign)
        .field("strokeJoin",        &SimpleSlottableTextProperty::strokeJoin)
        .field("direction",         &SimpleSlottableTextProperty::direction)
        .field("linebreak",         &SimpleSlottableTextProperty::lineBreak)
        .field("resize",            &SimpleSlottableTextProperty::resize)
        .field("_fillColorPtr",     &SimpleSlottableTextProperty::fillColorPtr)
        .field("_strokeColorPtr",   &SimpleSlottableTextProperty::strokeColorPtr)
        .field("_boundingBoxPtr",   &SimpleSlottableTextProperty::boundingBoxPtr);

    constant("managed_skottie", true);
}
