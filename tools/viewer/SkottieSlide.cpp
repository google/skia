/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/viewer/SkottieSlide.h"

#if defined(SK_ENABLE_SKOTTIE)

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkTime.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTPin.h"
#include "modules/audioplayer/SkAudioPlayer.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "modules/skresources/include/SkResources.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tools/Resources.h"
#include "tools/timer/TimeUtils.h"
#include "tools/viewer/SkottieTextEditor.h"

#include <cmath>
#include <vector>

#include "imgui.h"

namespace {

class Track final : public skresources::ExternalTrackAsset {
public:
    explicit Track(std::unique_ptr<SkAudioPlayer> player) : fPlayer(std::move(player)) {}

private:
    void seek(float t) override {
        if (fPlayer->isStopped() && t >=0) {
            fPlayer->play();
        }

        if (fPlayer->isPlaying()) {
            if (t < 0) {
                fPlayer->stop();
            } else {
                static constexpr float kTolerance = 0.075f;
                const auto player_pos = fPlayer->time();

                if (std::abs(player_pos - t) > kTolerance) {
                    fPlayer->setTime(t);
                }
            }
        }
    }

    const std::unique_ptr<SkAudioPlayer> fPlayer;
};

class AudioProviderProxy final : public skresources::ResourceProviderProxyBase {
public:
    explicit AudioProviderProxy(sk_sp<skresources::ResourceProvider> rp)
        : INHERITED(std::move(rp)) {}

private:
    sk_sp<skresources::ExternalTrackAsset> loadAudioAsset(const char path[],
                                                          const char name[],
                                                          const char[] /*id*/) override {
        if (auto data = this->load(path, name)) {
            if (auto player = SkAudioPlayer::Make(std::move(data))) {
                return sk_make_sp<Track>(std::move(player));
            }
        }

        return nullptr;
    }

    using INHERITED = skresources::ResourceProviderProxyBase;
};

class Decorator : public SkNoncopyable {
public:
    virtual ~Decorator() = default;

    // We pass in the Matrix and have the Decorator handle using it independently
    // This is so decorators can keep position on screen after moving.
    virtual void render(SkCanvas*, double, const SkMatrix) = 0;
};

class SimpleMarker final : public Decorator {
public:
    ~SimpleMarker() override = default;

    static std::unique_ptr<Decorator> Make() { return std::make_unique<SimpleMarker>(); }

    void render(SkCanvas* canvas, double t, const SkMatrix transform) override {
        canvas->concat(transform);
        SkPaint p;
        p.setAntiAlias(true);

        p.setColor(SK_ColorGREEN);
        canvas->drawCircle(0, 0, 5, p);

        p.setColor(SK_ColorRED);
        p.setStrokeWidth(1.5f);
        canvas->drawLine(-10, 0, 10, 0, p);
        canvas->drawLine(0, -10, 0, 10, p);
    }
};

class TestingResourceProvider : public skresources::ResourceProvider {
public:
    TestingResourceProvider() {}

    sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const override {
        auto it = fResources.find(resource_name);
        if (it != fResources.end()) {
            return it->second;
        } else {
            return GetResourceAsData(SkOSPath::Join(resource_path, resource_name).c_str());
        }
    }

    sk_sp<skresources::ImageAsset> loadImageAsset(const char resource_path[],
                                                  const char resource_name[],
                                                  const char /*resource_id*/[]) const override {
        auto data = this->load(resource_path, resource_name);
        return skresources::MultiFrameImageAsset::Make(data);
    }

    void addPath(const char resource_name[], const SkPath& path) {
        fResources[resource_name] = path.serialize();
    }

private:
    std::unordered_map<std::string, sk_sp<SkData>> fResources;
};

static const struct DecoratorRec {
    const char* fName;
    std::unique_ptr<Decorator>(*fFactory)();
} kDecorators[] = {
    { "Simple marker",       SimpleMarker::Make },
};

class TextTracker final : public skottie::PropertyObserver {
public:
    explicit TextTracker(sk_sp<PropertyObserver> delegate) : fDelegate(std::move(delegate)) {}

    std::vector<std::unique_ptr<skottie::TextPropertyHandle>>& props() {
        return fTextProps;
    }

private:
    void onTextProperty(const char node_name[],
                        const LazyHandle<skottie::TextPropertyHandle>& lh) override {
        fTextProps.push_back(lh());

        if (fDelegate) {
            fDelegate->onTextProperty(node_name, lh);
        }
    }

    const sk_sp<PropertyObserver>                             fDelegate;
    std::vector<std::unique_ptr<skottie::TextPropertyHandle>> fTextProps;
};

} // namespace

class SkottieSlide::TransformTracker : public skottie::PropertyObserver {
public:
    void renderUI() {
        if (ImGui::Begin("Transform Tracker", nullptr)) {
            if (ImGui::BeginCombo("Transform", fTransformSelect
                                       ? std::get<0>(*fTransformSelect).c_str()
                                       : nullptr)) {
                if (ImGui::Selectable("(none)", true)) {
                    fTransformSelect = nullptr;
                }
                for (const auto& entry : fTransforms) {
                    const auto* transform_name = std::get<0>(entry).c_str();
                    if (ImGui::Selectable(transform_name, false)) {
                        if (!fTransformSelect ||
                            transform_name != std::get<0>(*fTransformSelect).c_str()) {
                            fTransformSelect = &entry;
                            // Reset the decorator on transform change.
                            fDecorator = fDecoratorSelect->fFactory();
                        }
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::BeginCombo("Decoration", fDecoratorSelect->fName)) {
                for (const auto& dec : kDecorators) {
                    if (ImGui::Selectable(dec.fName, false)) {
                        if (dec.fName != fDecoratorSelect->fName) {
                            fDecoratorSelect = &dec;
                            fDecorator = fDecoratorSelect->fFactory();
                        }
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::End();
    }

    void renderTracker(SkCanvas* canvas, double time, const SkSize& win_size, const SkSize& anim_size) const {
        if (!fTransformSelect) {
            return;
        }

        const auto tprop = std::get<1>(*fTransformSelect)->get();

        const auto m = SkMatrix::Translate(tprop.fPosition.fX, tprop.fPosition.fY)
                     * SkMatrix::RotateDeg(tprop.fRotation)
                     * SkMatrix::Scale    (tprop.fScale.fX*0.01f, tprop.fScale.fY*0.01f)
                     * SkMatrix::Translate(tprop.fAnchorPoint.fX, tprop.fAnchorPoint.fY);

        const auto viewer_matrix = SkMatrix::RectToRect(SkRect::MakeSize(anim_size),
                                                        SkRect::MakeSize(win_size),
                                                        SkMatrix::kCenter_ScaleToFit);

        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(viewer_matrix);

        SkASSERT(fDecorator);
        fDecorator->render(canvas, time, m);
    }

private:
    void onTransformProperty(const char name[],
                             const LazyHandle<skottie::TransformPropertyHandle>& lh) override {

        fTransforms.push_back(std::make_tuple(SkString(name), lh()));
    }

    using TransformT = std::tuple<SkString, std::unique_ptr<skottie::TransformPropertyHandle>>;

    std::vector<TransformT>    fTransforms;
    std::unique_ptr<Decorator> fDecorator;
    const TransformT*          fTransformSelect = nullptr;
    const DecoratorRec*        fDecoratorSelect = &kDecorators[0];
};

// Holds a pointer to a slot manager and the list of slots for the UI widget to track
class SkottieSlide::SlotManagerWrapper {
public:
    SlotManagerWrapper(sk_sp<skresources::ResourceProvider> rp, SkottieSlide* ss)
        : fSlotManager(sk_make_sp<skottie_utils::SlotManager>())
        , fResourceProvider(std::move(rp))
        , fSkottieSlide(ss)
    {}


    void renderUI() {
        if (ImGui::Begin("Slot Manager", nullptr)) {
            ImGui::Text("Color Slots");
            for (size_t i = 0; i < fColorSlots.size(); i++) {
                auto& cSlot = fColorSlots.at(i);
                ImGui::PushID(i);
                ImGui::InputText("ColorSlotID", cSlot.first.data(), cSlot.first.size());
                ImGui::ColorEdit4("Color", cSlot.second.data());
                ImGui::PopID();
            }
            if(ImGui::Button("+ Color")) {
                std::array<char, kBufferLen> s = {'\0'};
                fColorSlots.push_back(std::make_pair(s, std::array{1.0f, 1.0f, 1.0f, 1.0f}));
            }

            ImGui::Text("Text Slots");
            for (size_t i = 0; i < fTextStringSlots.size(); i++) {
                auto& tSlot = fTextStringSlots.at(i);
                ImGui::PushID(i);
                ImGui::InputText("TextSlotID", tSlot.first.data(), tSlot.first.size());
                ImGui::InputText("Text", tSlot.second.data(), tSlot.second.size());
                ImGui::PopID();
            }
            if (ImGui::Button("+ Text")) {
                std::array<char, kBufferLen> s = {'\0'};
                std::array<char, kBufferLen> t = {'\0'};
                fTextStringSlots.push_back(std::make_pair(s, t));
            }

            ImGui::Text("Image Slots");
            for (size_t i = 0; i < fImageSlots.size(); i++) {
                auto& iSlot = fImageSlots.at(i);
                ImGui::PushID(i);
                ImGui::InputText("ImageSlotID", iSlot.first.data(), iSlot.first.size());
                if (ImGui::BeginCombo("Resource", iSlot.second.data())) {
                    for (const auto& res : fResList) {
                        if (ImGui::Selectable(res.c_str(), false)) {
                            iSlot.second = res.c_str();
                        }
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopID();
            }
            if (ImGui::Button("+ Image")) {
                std::array<char, kBufferLen> s = {'\0'};
                fImageSlots.push_back(std::make_pair(s, std::string()));
            }

            if (ImGui::Button("Apply Slots")) {
                this->pushSlots();
            }

        }
        ImGui::End();
    }

    void pushSlots() {
        for(const auto& s : fColorSlots) {
            fSlotManager->setColorSlot(s.first.data(), SkColor4f{s.second[0], s.second[1],
                                                       s.second[2], s.second[3]}.toSkColor());
        }
        for(const auto& s : fTextStringSlots) {
            fSlotManager->setTextStringSlot(s.first.data(), SkString(s.second.data()));
        }
        for(const auto& s : fImageSlots) {
            auto img = fResourceProvider->loadImageAsset("images/", s.second.c_str(), nullptr);
            if (img) {
                fSlotManager->setImageSlot(s.first.data(), img);
            }
        }
        fSkottieSlide->init();
    }

    void prepareImageAssetList(const char* dirname) {
        fResList.clear();
        SkOSFile::Iter iter(dirname, ".png");
        for (SkString file; iter.next(&file); ) {
            fResList.push_back(file);
        }
    }

    sk_sp<skresources::ResourceProvider> getResourceProvider() {
        return fSlotManager->getResourceProvider();
    }

    sk_sp<skottie::PropertyObserver> getPropertyObserver() {
        return fSlotManager->getPropertyObserver();
    }
private:
    static constexpr int kBufferLen = 256;

    const sk_sp<skottie_utils::SlotManager> fSlotManager;
    const sk_sp<skresources::ResourceProvider> fResourceProvider;
    std::vector<SkString> fResList;

    SkottieSlide* fSkottieSlide;

    using GuiTextBuffer = std::array<char, kBufferLen>;

    std::vector<std::pair<GuiTextBuffer, std::array<float, 4>>>   fColorSlots;
    std::vector<std::pair<GuiTextBuffer, GuiTextBuffer>>          fTextStringSlots;
    std::vector<std::pair<GuiTextBuffer, std::string>>            fImageSlots;

};

static void draw_stats_box(SkCanvas* canvas, const skottie::Animation::Builder::Stats& stats) {
    static constexpr SkRect kR = { 10, 10, 280, 120 };
    static constexpr SkScalar kTextSize = 20;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xffeeeeee);

    SkFont font(nullptr, kTextSize);

    canvas->drawRect(kR, paint);

    paint.setColor(SK_ColorBLACK);

    const auto json_size = SkStringPrintf("Json size: %zu bytes",
                                          stats.fJsonSize);
    canvas->drawString(json_size, kR.x() + 10, kR.y() + kTextSize * 1, font, paint);
    const auto animator_count = SkStringPrintf("Animator count: %zu",
                                               stats.fAnimatorCount);
    canvas->drawString(animator_count, kR.x() + 10, kR.y() + kTextSize * 2, font, paint);
    const auto json_parse_time = SkStringPrintf("Json parse time: %.3f ms",
                                                stats.fJsonParseTimeMS);
    canvas->drawString(json_parse_time, kR.x() + 10, kR.y() + kTextSize * 3, font, paint);
    const auto scene_parse_time = SkStringPrintf("Scene build time: %.3f ms",
                                                 stats.fSceneParseTimeMS);
    canvas->drawString(scene_parse_time, kR.x() + 10, kR.y() + kTextSize * 4, font, paint);
    const auto total_load_time = SkStringPrintf("Total load time: %.3f ms",
                                                stats.fTotalLoadTimeMS);
    canvas->drawString(total_load_time, kR.x() + 10, kR.y() + kTextSize * 5, font, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(kR, paint);
}

SkottieSlide::SkottieSlide(const SkString& name, const SkString& path)
    : fPath(path) {
    fName = name;
}

void SkottieSlide::init() {
    class Logger final : public skottie::Logger {
    public:
        struct LogEntry {
            SkString fMessage,
                     fJSON;
        };

        void log(skottie::Logger::Level lvl, const char message[], const char json[]) override {
            auto& log = lvl == skottie::Logger::Level::kError ? fErrors : fWarnings;
            log.push_back({ SkString(message), json ? SkString(json) : SkString() });
        }

        void report() const {
            SkDebugf("Animation loaded with %zu error%s, %zu warning%s.\n",
                     fErrors.size(), fErrors.size() == 1 ? "" : "s",
                     fWarnings.size(), fWarnings.size() == 1 ? "" : "s");

            const auto& show = [](const LogEntry& log, const char prefix[]) {
                SkDebugf("%s%s", prefix, log.fMessage.c_str());
                if (!log.fJSON.isEmpty())
                    SkDebugf(" : %s", log.fJSON.c_str());
                SkDebugf("\n");
            };

            for (const auto& err : fErrors)   show(err, "  !! ");
            for (const auto& wrn : fWarnings) show(wrn, "  ?? ");
        }

    private:
        std::vector<LogEntry> fErrors,
                              fWarnings;
    };

    auto logger = sk_make_sp<Logger>();

    uint32_t flags = 0;
    if (fPreferGlyphPaths) {
        flags |= skottie::Animation::Builder::kPreferEmbeddedFonts;
    }
    skottie::Animation::Builder builder(flags);

    auto resource_provider =
        sk_make_sp<AudioProviderProxy>(
            skresources::DataURIResourceProviderProxy::Make(
                skresources::FileResourceProvider::Make(SkOSPath::Dirname(fPath.c_str()),
                                                        /*predecode=*/true),
                /*predecode=*/true));

    static constexpr char kInterceptPrefix[] = "__";
    auto precomp_interceptor =
            sk_make_sp<skottie_utils::ExternalAnimationPrecompInterceptor>(resource_provider,
                                                                           kInterceptPrefix);

    fTransformTracker = sk_make_sp<TransformTracker>();
    auto text_tracker = sk_make_sp<TextTracker>(fTransformTracker);

    if (!fSlotManagerWrapper) {
        fSlotManagerWrapper = std::make_unique<SlotManagerWrapper>(resource_provider, this);
    }
    fSlotManagerWrapper->prepareImageAssetList(GetResourcePath("skottie/images").c_str());

    builder.setLogger(logger)
           .setPrecompInterceptor(std::move(precomp_interceptor));

    if (fShowSlotManager) {
        builder.setResourceProvider(fSlotManagerWrapper->getResourceProvider())
               .setPropertyObserver(fSlotManagerWrapper->getPropertyObserver());
    } else {
        builder.setResourceProvider(std::move(resource_provider))
               .setPropertyObserver(text_tracker);
    }
    fAnimation = builder.makeFromFile(fPath.c_str());
    fAnimationStats = builder.getStats();
    fTimeBase       = 0; // force a time reset

    if (fAnimation) {
        fAnimation->seek(0);
        fFrameTimes.resize(SkScalarCeilToInt(fAnimation->duration() * fAnimation->fps()));
        SkDebugf("Loaded Bodymovin animation v: %s, size: [%f %f]\n",
                 fAnimation->version().c_str(),
                 fAnimation->size().width(),
                 fAnimation->size().height());
        logger->report();

        if (auto text_props = std::move(text_tracker->props()); !text_props.empty()) {
            // Attach the editor to the first text layer, and track the rest as dependents.
            auto editor_target = std::move(text_props[0]);
            text_props.erase(text_props.cbegin());
            fTextEditor = sk_make_sp<SkottieTextEditor>(std::move(editor_target),
                                                        std::move(text_props));
        }
    } else {
        SkDebugf("failed to load Bodymovin animation: %s\n", fPath.c_str());
    }
}

void SkottieSlide::load(SkScalar w, SkScalar h) {
    fWinSize = SkSize::Make(w, h);
    this->init();
}

void SkottieSlide::unload() {
    fAnimation.reset();
}

void SkottieSlide::resize(SkScalar w, SkScalar h) {
    fWinSize = { w, h };
}

void SkottieSlide::draw(SkCanvas* canvas) {
    if (fAnimation) {
        SkAutoCanvasRestore acr(canvas, true);
        const auto dstR = SkRect::MakeSize(fWinSize);

        {
            const auto t0 = SkTime::GetNSecs();
            fAnimation->render(canvas, &dstR);

            // TODO: this does not capture GPU flush time!
            const auto  frame_index  = static_cast<size_t>(fCurrentFrame);
            fFrameTimes[frame_index] = static_cast<float>((SkTime::GetNSecs() - t0) * 1e-6);
        }

        double fr = 60;
        if (fFrameRate != 0) {
            fr = fFrameRate;
        }
        fTransformTracker->renderTracker(canvas, fCurrentFrame/fr, fWinSize, fAnimation->size());

        if (fShowAnimationStats) {
            draw_stats_box(canvas, fAnimationStats);
        }
        if (fShowAnimationInval) {
            const auto t = SkMatrix::RectToRect(SkRect::MakeSize(fAnimation->size()), dstR,
                                                SkMatrix::kCenter_ScaleToFit);
            SkPaint fill, stroke;
            fill.setAntiAlias(true);
            fill.setColor(0x40ff0000);
            stroke.setAntiAlias(true);
            stroke.setColor(0xffff0000);
            stroke.setStyle(SkPaint::kStroke_Style);

            for (const auto& r : fInvalController) {
                SkRect bounds;
                t.mapRect(&bounds, r);
                canvas->drawRect(bounds, fill);
                canvas->drawRect(bounds, stroke);
            }
        }
        if (fShowUI) {
            this->renderUI();
        }
        if (fShowSlotManager) {
            // not able to track layers with a PropertyObserver while using SM's PropertyObserver
            fShowTrackerUI = false;
            fSlotManagerWrapper->renderUI();
        }
        if (fShowTrackerUI) {
            fTransformTracker->renderUI();
        }
    }
}

bool SkottieSlide::animate(double nanos) {
    if (!fTimeBase) {
        // Reset the animation time.
        fTimeBase = nanos;
    }

    if (fAnimation) {
        fInvalController.reset();

        const auto frame_count = fAnimation->duration() * fAnimation->fps();

        if (!fDraggingProgress) {
            // Clock-driven progress: update current frame.
            const double t_sec = (nanos - fTimeBase) * 1e-9;
            fCurrentFrame = std::fmod(t_sec * fAnimation->fps(), frame_count);
        } else {
            // Slider-driven progress: update the time origin.
            fTimeBase = nanos - fCurrentFrame / fAnimation->fps() * 1e9;
        }

        // Sanitize and rate-lock the current frame.
        fCurrentFrame = SkTPin<float>(fCurrentFrame, 0.0f, frame_count - 1);
        if (fFrameRate > 0) {
            const auto fps_scale = fFrameRate / fAnimation->fps();
            fCurrentFrame = std::trunc(fCurrentFrame * fps_scale) / fps_scale;
        }

        fAnimation->seekFrame(fCurrentFrame, fShowAnimationInval ? &fInvalController
                                                                 : nullptr);
    }
    return true;
}

bool SkottieSlide::onChar(SkUnichar c) {
    if (fTextEditor && fTextEditor->onCharInput(c)) {
        return true;
    }

    switch (c) {
    case 'I':
        fShowAnimationStats = !fShowAnimationStats;
        return true;
    case 'G':
        fPreferGlyphPaths = !fPreferGlyphPaths;
        this->load(fWinSize.width(), fWinSize.height());
        return true;
    case 'T':
        fShowTrackerUI = !fShowTrackerUI;
        return true;
    case 'M':
        fShowSlotManager = !fShowSlotManager;
        return true;
    case 'E':
        if (fTextEditor) {
            fTextEditor->toggleEnabled();
        }
        return true;
    }

    return INHERITED::onChar(c);
}

bool SkottieSlide::onMouse(SkScalar x, SkScalar y, skui::InputState state, skui::ModifierKey mod) {
    if (fTextEditor && fTextEditor->onMouseInput(x, y, state, mod)) {
        return true;
    }

    switch (state) {
    case skui::InputState::kUp:
        fShowAnimationInval = !fShowAnimationInval;
        fShowAnimationStats = !fShowAnimationStats;
        break;
    default:
        break;
    }

    fShowUI = this->UIArea().contains(x, y);

    return false;
}

SkRect SkottieSlide::UIArea() const {
    static constexpr float kUIHeight = 120.0f;

    return SkRect::MakeXYWH(0, fWinSize.height() - kUIHeight, fWinSize.width(), kUIHeight);
}

void SkottieSlide::renderUI() {
    static constexpr auto kUI_opacity     = 0.35f,
                          kUI_hist_height = 50.0f,
                          kUI_fps_width   = 100.0f;

    auto add_frame_rate_option = [this](const char* label, double rate) {
        const auto is_selected = (fFrameRate == rate);
        if (ImGui::Selectable(label, is_selected)) {
            fFrameRate      = rate;
            fFrameRateLabel = label;
        }
        if (is_selected) {
            ImGui::SetItemDefaultFocus();
        }
    };

    ImGui::SetNextWindowBgAlpha(kUI_opacity);
    if (ImGui::Begin("Skottie Controls", nullptr, ImGuiWindowFlags_NoDecoration |
                                                  ImGuiWindowFlags_NoResize |
                                                  ImGuiWindowFlags_NoMove |
                                                  ImGuiWindowFlags_NoSavedSettings |
                                                  ImGuiWindowFlags_NoFocusOnAppearing |
                                                  ImGuiWindowFlags_NoNav)) {
        const auto ui_area = this->UIArea();
        ImGui::SetWindowPos(ImVec2(ui_area.x(), ui_area.y()));
        ImGui::SetWindowSize(ImVec2(ui_area.width(), ui_area.height()));

        ImGui::PushItemWidth(-1);
        ImGui::PlotHistogram("", fFrameTimes.data(), fFrameTimes.size(),
                                 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0, kUI_hist_height));
        ImGui::SliderFloat("", &fCurrentFrame, 0, fAnimation->duration() * fAnimation->fps() - 1);
        fDraggingProgress = ImGui::IsItemActive();
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(kUI_fps_width);
        if (ImGui::BeginCombo("FPS", fFrameRateLabel)) {
            add_frame_rate_option("", 0.0);
            add_frame_rate_option("Native", fAnimation->fps());
            add_frame_rate_option( "1",  1.0);
            add_frame_rate_option("15", 15.0);
            add_frame_rate_option("24", 24.0);
            add_frame_rate_option("30", 30.0);
            add_frame_rate_option("60", 60.0);
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
    }
    ImGui::End();
}

#endif // SK_ENABLE_SKOTTIE
