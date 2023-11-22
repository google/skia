/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/Skottie.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "modules/skottie/include/ExternalLayer.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/src/Composition.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/Transform.h"
#include "modules/skottie/src/text/TextAdapter.h"
#include "modules/sksg/include/SkSGInvalidationController.h"
#include "modules/sksg/include/SkSGOpacityEffect.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGTransform.h"
#include "src/core/SkTraceEvent.h"

#include <chrono>
#include <cmath>
#include <memory>
#include <ratio>

#include <stdlib.h>

namespace skottie {

namespace internal {

void SceneGraphRevalidator::setRoot(sk_sp<sksg::RenderNode> root) {
    fRoot = std::move(root);
}

void SceneGraphRevalidator::revalidate() {
    if (fRoot) {
        fRoot->revalidate(nullptr, SkMatrix::I());
    }
}

void AnimationBuilder::log(Logger::Level lvl, const skjson::Value* json,
                           const char fmt[], ...) const {
    if (!fLogger) {
        return;
    }

    char buff[1024];
    va_list va;
    va_start(va, fmt);
    const auto len = vsnprintf(buff, sizeof(buff), fmt, va);
    va_end(va);

    if (len < 0) {
        SkDebugf("!! Could not format log message !!\n");
        return;
    }

    if (len >= SkToInt(sizeof(buff))) {
        static constexpr char kEllipsesStr[] = "...";
        strcpy(buff + sizeof(buff) - sizeof(kEllipsesStr), kEllipsesStr);
    }

    SkString jsonstr = json ? json->toString() : SkString();

    fLogger->log(lvl, buff, jsonstr.c_str());
}

class OpacityAdapter final : public DiscardableAdapterBase<OpacityAdapter, sksg::OpacityEffect> {
public:
    OpacityAdapter(const skjson::ObjectValue& jobject,
                   sk_sp<sksg::RenderNode> child,
                   const AnimationBuilder& abuilder)
        : INHERITED(sksg::OpacityEffect::Make(std::move(child))) {
        this->bind(abuilder, jobject["o"], fOpacity);
    }

private:
    void onSync() override {
        this->node()->setOpacity(fOpacity * 0.01f);
    }

    ScalarValue fOpacity = 100;

    using INHERITED = DiscardableAdapterBase<OpacityAdapter, sksg::OpacityEffect>;
};


sk_sp<sksg::RenderNode> AnimationBuilder::attachOpacity(const skjson::ObjectValue& jobject,
                                                        sk_sp<sksg::RenderNode> child_node) const {
    if (!child_node)
        return nullptr;

    auto adapter = OpacityAdapter::Make(jobject, child_node, *this);
    if (adapter->isStatic()) {
        adapter->seek(0);
    }
    auto dispatched = this->dispatchOpacityProperty(adapter->node());
    if (adapter->isStatic()) {
        if (!dispatched && adapter->node()->getOpacity() >= 1) {
            // No obeservable effects - we can discard.
            return child_node;
        }
    } else {
        fCurrentAnimatorScope->push_back(adapter);
    }

    return adapter->node();
}

AnimationBuilder::AnimationBuilder(sk_sp<ResourceProvider> rp, sk_sp<SkFontMgr> fontmgr,
                                   sk_sp<PropertyObserver> pobserver, sk_sp<Logger> logger,
                                   sk_sp<MarkerObserver> mobserver, sk_sp<PrecompInterceptor> pi,
                                   sk_sp<ExpressionManager> expressionmgr,
                                   Animation::Builder::Stats* stats,
                                   const SkSize& comp_size, float duration, float framerate,
                                   uint32_t flags)
    : fResourceProvider(std::move(rp))
    , fFontMgr(std::move(fontmgr))
    , fPropertyObserver(std::move(pobserver))
    , fLogger(std::move(logger))
    , fMarkerObserver(std::move(mobserver))
    , fPrecompInterceptor(std::move(pi))
    , fExpressionManager(std::move(expressionmgr))
    , fRevalidator(sk_make_sp<SceneGraphRevalidator>())
    , fSlotManager(sk_make_sp<SlotManager>(fRevalidator))
    , fStats(stats)
    , fCompSize(comp_size)
    , fDuration(duration)
    , fFrameRate(framerate)
    , fFlags(flags)
    , fHasNontrivialBlending(false) {}

AnimationBuilder::AnimationInfo AnimationBuilder::parse(const skjson::ObjectValue& jroot) {
    this->dispatchMarkers(jroot["markers"]);

    AutoScope ascope(this);
    AutoPropertyTracker apt(this, jroot, PropertyObserver::NodeType::COMPOSITION);

    this->parseAssets(jroot["assets"]);
    this->parseFonts(jroot["fonts"], jroot["chars"]);
    fSlotsRoot = jroot["slots"];

    auto root = CompositionBuilder(*this, fCompSize, jroot).build(*this);

    auto animators = ascope.release();
    fStats->fAnimatorCount = animators.size();

    // Point the revalidator to our final root, and perform initial revalidation.
    fRevalidator->setRoot(root);
    fRevalidator->revalidate();

    return { std::move(root), std::move(animators), std::move(fSlotManager)};
}

void AnimationBuilder::parseAssets(const skjson::ArrayValue* jassets) {
    if (!jassets) {
        return;
    }

    for (const skjson::ObjectValue* asset : *jassets) {
        if (asset) {
            fAssets.set(ParseDefault<SkString>((*asset)["id"], SkString()), { asset, false });
        }
    }
}

void AnimationBuilder::dispatchMarkers(const skjson::ArrayValue* jmarkers) const {
    if (!fMarkerObserver || !jmarkers) {
        return;
    }

    // For frame-number -> t conversions.
    const auto frameRatio = 1 / (fFrameRate * fDuration);

    for (const skjson::ObjectValue* m : *jmarkers) {
        if (!m) continue;

        const skjson::StringValue* name = (*m)["cm"];
        const auto time = ParseDefault((*m)["tm"], -1.0f),
               duration = ParseDefault((*m)["dr"], -1.0f);

        if (name && time >= 0 && duration >= 0) {
            fMarkerObserver->onMarker(
                        name->begin(),
                        // "tm" is in frames
                        time * frameRatio,
                        // ... as is "dr"
                        (time + duration) * frameRatio
            );
        } else {
            this->log(Logger::Level::kWarning, m, "Ignoring unexpected marker.");
        }
    }
}

bool AnimationBuilder::dispatchColorProperty(const sk_sp<sksg::Color>& c) const {
    bool dispatched = false;
    if (fPropertyObserver) {
        const char * node_name = fPropertyObserverContext;
        fPropertyObserver->onColorProperty(node_name,
            [&]() {
                dispatched = true;
                return std::make_unique<ColorPropertyHandle>(c, fRevalidator);
            });
    }

    return dispatched;
}

bool AnimationBuilder::dispatchOpacityProperty(const sk_sp<sksg::OpacityEffect>& o) const {
    bool dispatched = false;

    if (fPropertyObserver) {
        fPropertyObserver->onOpacityProperty(fPropertyObserverContext,
            [&]() {
                dispatched = true;
                return std::make_unique<OpacityPropertyHandle>(o, fRevalidator);
            });
    }

    return dispatched;
}

bool AnimationBuilder::dispatchTextProperty(const sk_sp<TextAdapter>& t,
                                            const skjson::ObjectValue* jtext) const {
    bool dispatched = false;

    if (jtext) {
        if (const skjson::StringValue* slotID = (*jtext)["sid"]) {
            fSlotManager->trackTextValue(SkString(slotID->begin()), t);
            dispatched = true;
        }
    }

    if (fPropertyObserver) {
        fPropertyObserver->onTextProperty(fPropertyObserverContext,
            [&]() {
                dispatched = true;
                return std::make_unique<TextPropertyHandle>(t, fRevalidator);
            });
    }

    return dispatched;
}

bool AnimationBuilder::dispatchTransformProperty(const sk_sp<TransformAdapter2D>& t) const {
    bool dispatched = false;

    if (fPropertyObserver) {
        fPropertyObserver->onTransformProperty(fPropertyObserverContext,
            [&]() {
                dispatched = true;
                return std::make_unique<TransformPropertyHandle>(t, fRevalidator);
            });
    }

    return dispatched;
}

sk_sp<ExpressionManager> AnimationBuilder::expression_manager() const {
    return fExpressionManager;
}

void AnimationBuilder::AutoPropertyTracker::updateContext(PropertyObserver* observer,
                                                          const skjson::ObjectValue& obj) {
    const skjson::StringValue* name = obj["nm"];
    fBuilder->fPropertyObserverContext = name ? name->begin() : fPrevContext;
}

} // namespace internal

void Logger::log(Level, const char[], const char*) {}

Animation::Builder::Builder(uint32_t flags) : fFlags(flags) {}
Animation::Builder::~Builder() = default;

Animation::Builder& Animation::Builder::setResourceProvider(sk_sp<ResourceProvider> rp) {
    fResourceProvider = std::move(rp);
    return *this;
}

Animation::Builder& Animation::Builder::setFontManager(sk_sp<SkFontMgr> fmgr) {
    fFontMgr = std::move(fmgr);
    return *this;
}

Animation::Builder& Animation::Builder::setPropertyObserver(sk_sp<PropertyObserver> pobserver) {
    fPropertyObserver = std::move(pobserver);
    return *this;
}

Animation::Builder& Animation::Builder::setLogger(sk_sp<Logger> logger) {
    fLogger = std::move(logger);
    return *this;
}

Animation::Builder& Animation::Builder::setMarkerObserver(sk_sp<MarkerObserver> mobserver) {
    fMarkerObserver = std::move(mobserver);
    return *this;
}

Animation::Builder& Animation::Builder::setPrecompInterceptor(sk_sp<PrecompInterceptor> pi) {
    fPrecompInterceptor = std::move(pi);
    return *this;
}

Animation::Builder& Animation::Builder::setExpressionManager(sk_sp<ExpressionManager> em) {
    fExpressionManager = std::move(em);
    return *this;
}

sk_sp<Animation> Animation::Builder::make(SkStream* stream) {
    if (!stream->hasLength()) {
        // TODO: handle explicit buffering?
        if (fLogger) {
            fLogger->log(Logger::Level::kError, "Cannot parse streaming content.\n");
        }
        return nullptr;
    }

    auto data = SkData::MakeFromStream(stream, stream->getLength());
    if (!data) {
        if (fLogger) {
            fLogger->log(Logger::Level::kError, "Failed to read the input stream.\n");
        }
        return nullptr;
    }

    return this->make(static_cast<const char*>(data->data()), data->size());
}

sk_sp<Animation> Animation::Builder::make(const char* data, size_t data_len) {
    TRACE_EVENT0("skottie", TRACE_FUNC);

    // Sanitize factory args.
    class NullResourceProvider final : public ResourceProvider {
        sk_sp<SkData> load(const char[], const char[]) const override { return nullptr; }
    };
    auto resolvedProvider = fResourceProvider
            ? fResourceProvider : sk_make_sp<NullResourceProvider>();

    fStats = Stats{};

    fStats.fJsonSize = data_len;
    const auto t0 = std::chrono::steady_clock::now();

    const skjson::DOM dom(data, data_len);
    if (!dom.root().is<skjson::ObjectValue>()) {
        // TODO: more error info.
        if (fLogger) {
            fLogger->log(Logger::Level::kError, "Failed to parse JSON input.\n");
        }
        return nullptr;
    }
    const auto& json = dom.root().as<skjson::ObjectValue>();

    const auto t1 = std::chrono::steady_clock::now();
    fStats.fJsonParseTimeMS = std::chrono::duration<float, std::milli>{t1-t0}.count();

    const auto version  = ParseDefault<SkString>(json["v"], SkString());
    const auto size     = SkSize::Make(ParseDefault<float>(json["w"], 0.0f),
                                       ParseDefault<float>(json["h"], 0.0f));
    const auto fps      = ParseDefault<float>(json["fr"], -1.0f),
               inPoint  = ParseDefault<float>(json["ip"], 0.0f),
               outPoint = std::max(ParseDefault<float>(json["op"], SK_ScalarMax), inPoint),
               duration = sk_ieee_float_divide(outPoint - inPoint, fps);

    if (size.isEmpty() || version.isEmpty() || fps <= 0 ||
        !SkScalarIsFinite(inPoint) || !SkScalarIsFinite(outPoint) || !SkScalarIsFinite(duration)) {
        if (fLogger) {
            const auto msg = SkStringPrintf(
                         "Invalid animation params (version: %s, size: [%f %f], frame rate: %f, "
                         "in-point: %f, out-point: %f)\n",
                         version.c_str(), size.width(), size.height(), fps, inPoint, outPoint);
            fLogger->log(Logger::Level::kError, msg.c_str());
        }
        return nullptr;
    }

    SkASSERT(resolvedProvider);
    internal::AnimationBuilder builder(std::move(resolvedProvider), fFontMgr,
                                       std::move(fPropertyObserver),
                                       std::move(fLogger),
                                       std::move(fMarkerObserver),
                                       std::move(fPrecompInterceptor),
                                       std::move(fExpressionManager),
                                       &fStats, size, duration, fps, fFlags);
    auto ainfo = builder.parse(json);

    fSlotManager = ainfo.fSlotManager;

    const auto t2 = std::chrono::steady_clock::now();
    fStats.fSceneParseTimeMS = std::chrono::duration<float, std::milli>{t2-t1}.count();
    fStats.fTotalLoadTimeMS  = std::chrono::duration<float, std::milli>{t2-t0}.count();

    if (!ainfo.fSceneRoot && fLogger) {
        fLogger->log(Logger::Level::kError, "Could not parse animation.\n");
    }

    uint32_t flags = 0;
    if (builder.hasNontrivialBlending()) {
        flags |= Animation::Flags::kRequiresTopLevelIsolation;
    }

    return sk_sp<Animation>(new Animation(std::move(ainfo.fSceneRoot),
                                          std::move(ainfo.fAnimators),
                                          std::move(version),
                                          size,
                                          inPoint,
                                          outPoint,
                                          duration,
                                          fps,
                                          flags));
}

sk_sp<Animation> Animation::Builder::makeFromFile(const char path[]) {
    const auto data = SkData::MakeFromFileName(path);

    return data ? this->make(static_cast<const char*>(data->data()), data->size())
                : nullptr;
}

Animation::Animation(sk_sp<sksg::RenderNode> scene_root,
                     std::vector<sk_sp<internal::Animator>>&& animators,
                     SkString version, const SkSize& size,
                     double inPoint, double outPoint, double duration, double fps, uint32_t flags)
    : fSceneRoot(std::move(scene_root))
    , fAnimators(std::move(animators))
    , fVersion(std::move(version))
    , fSize(size)
    , fInPoint(inPoint)
    , fOutPoint(outPoint)
    , fDuration(duration)
    , fFPS(fps)
    , fFlags(flags) {}

Animation::~Animation() = default;

void Animation::render(SkCanvas* canvas, const SkRect* dstR) const {
    this->render(canvas, dstR, 0);
}

void Animation::render(SkCanvas* canvas, const SkRect* dstR, RenderFlags renderFlags) const {
    TRACE_EVENT0("skottie", TRACE_FUNC);

    if (!fSceneRoot)
        return;

    SkAutoCanvasRestore restore(canvas, true);

    const SkRect srcR = SkRect::MakeSize(this->size());
    if (dstR) {
        canvas->concat(SkMatrix::RectToRect(srcR, *dstR, SkMatrix::kCenter_ScaleToFit));
    }

    if (!(renderFlags & RenderFlag::kDisableTopLevelClipping)) {
        canvas->clipRect(srcR);
    }

    if ((fFlags & Flags::kRequiresTopLevelIsolation) &&
        !(renderFlags & RenderFlag::kSkipTopLevelIsolation)) {
        // The animation uses non-trivial blending, and needs
        // to be rendered into a separate/transparent layer.
        canvas->saveLayer(srcR, nullptr);
    }

    fSceneRoot->render(canvas);
}

void Animation::seekFrame(double t, sksg::InvalidationController* ic) {
    TRACE_EVENT0("skottie", TRACE_FUNC);

    if (!fSceneRoot)
        return;

    // Per AE/Lottie semantics out_point is exclusive.
    const auto kLastValidFrame = std::nextafterf(fOutPoint, fInPoint),
                     comp_time = SkTPin<float>(fInPoint + t, fInPoint, kLastValidFrame);

    for (const auto& anim : fAnimators) {
        anim->seek(comp_time);
    }

    fSceneRoot->revalidate(ic, SkMatrix::I());
}

void Animation::seekFrameTime(double t, sksg::InvalidationController* ic) {
    this->seekFrame(t * fFPS, ic);
}

sk_sp<Animation> Animation::Make(const char* data, size_t length) {
    return Builder().make(data, length);
}

sk_sp<Animation> Animation::Make(SkStream* stream) {
    return Builder().make(stream);
}

sk_sp<Animation> Animation::MakeFromFile(const char path[]) {
    return Builder().makeFromFile(path);
}

} // namespace skottie
