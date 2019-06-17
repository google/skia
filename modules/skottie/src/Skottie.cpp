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
#include "include/private/SkTArray.h"
#include "include/private/SkTo.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGInvalidationController.h"
#include "modules/sksg/include/SkSGOpacityEffect.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGScene.h"
#include "modules/sksg/include/SkSGTransform.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkTraceEvent.h"

#include <chrono>
#include <cmath>

#include "stdlib.h"

namespace skottie {

namespace internal {

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

sk_sp<sksg::Transform> AnimationBuilder::attachMatrix2D(const skjson::ObjectValue& t,
                                                        AnimatorScope* ascope,
                                                        sk_sp<sksg::Transform> parent) const {
    static const VectorValue g_default_vec_0   = {  0,   0},
                             g_default_vec_100 = {100, 100};

    auto matrix = sksg::Matrix<SkMatrix>::Make(SkMatrix::I());
    auto adapter = sk_make_sp<TransformAdapter2D>(matrix);

    auto bound = this->bindProperty<VectorValue>(t["a"], ascope,
            [adapter](const VectorValue& a) {
                adapter->setAnchorPoint(ValueTraits<VectorValue>::As<SkPoint>(a));
            }, g_default_vec_0);
    bound |= this->bindProperty<VectorValue>(t["p"], ascope,
            [adapter](const VectorValue& p) {
                adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
            }, g_default_vec_0);
    bound |= this->bindProperty<VectorValue>(t["s"], ascope,
            [adapter](const VectorValue& s) {
                adapter->setScale(ValueTraits<VectorValue>::As<SkVector>(s));
            }, g_default_vec_100);

    const auto* jrotation = &t["r"];
    if (jrotation->is<skjson::NullValue>()) {
        // 3d rotations have separate rx,ry,rz components.  While we don't fully support them,
        // we can still make use of rz.
        jrotation = &t["rz"];
    }
    bound |= this->bindProperty<ScalarValue>(*jrotation, ascope,
            [adapter](const ScalarValue& r) {
                adapter->setRotation(r);
            }, 0.0f);
    bound |= this->bindProperty<ScalarValue>(t["sk"], ascope,
            [adapter](const ScalarValue& sk) {
                adapter->setSkew(sk);
            }, 0.0f);
    bound |= this->bindProperty<ScalarValue>(t["sa"], ascope,
            [adapter](const ScalarValue& sa) {
                adapter->setSkewAxis(sa);
            }, 0.0f);

    const auto dispatched = this->dispatchTransformProperty(adapter);

    return (bound || dispatched)
        ? sksg::Transform::MakeConcat(std::move(parent), std::move(matrix))
        : parent;
}

sk_sp<sksg::Transform> AnimationBuilder::attachMatrix3D(const skjson::ObjectValue& t,
                                                        AnimatorScope* ascope,
                                                        sk_sp<sksg::Transform> parent,
                                                        sk_sp<TransformAdapter3D> adapter,
                                                        bool precompose_parent) const {
    static const VectorValue g_default_vec_0   = {  0,   0,   0},
                             g_default_vec_100 = {100, 100, 100};

    if (!adapter) {
        // Default to TransformAdapter3D (we only use external adapters for cameras).
        adapter = sk_make_sp<TransformAdapter3D>();
    }

    auto bound = this->bindProperty<VectorValue>(t["a"], ascope,
            [adapter](const VectorValue& a) {
                adapter->setAnchorPoint(TransformAdapter3D::Vec3(a));
            }, g_default_vec_0);
    bound |= this->bindProperty<VectorValue>(t["p"], ascope,
            [adapter](const VectorValue& p) {
                adapter->setPosition(TransformAdapter3D::Vec3(p));
            }, g_default_vec_0);
    bound |= this->bindProperty<VectorValue>(t["s"], ascope,
            [adapter](const VectorValue& s) {
                adapter->setScale(TransformAdapter3D::Vec3(s));
            }, g_default_vec_100);

    // Orientation and rx/ry/rz are mapped to the same rotation property -- the difference is
    // in how they get interpolated (vector vs. scalar/decomposed interpolation).
    bound |= this->bindProperty<VectorValue>(t["or"], ascope,
            [adapter](const VectorValue& o) {
                adapter->setRotation(TransformAdapter3D::Vec3(o));
            }, g_default_vec_0);

    bound |= this->bindProperty<ScalarValue>(t["rx"], ascope,
            [adapter](const ScalarValue& rx) {
                const auto& r = adapter->getRotation();
                adapter->setRotation(TransformAdapter3D::Vec3({rx, r.fY, r.fZ}));
            }, 0.0f);

    bound |= this->bindProperty<ScalarValue>(t["ry"], ascope,
            [adapter](const ScalarValue& ry) {
                const auto& r = adapter->getRotation();
                adapter->setRotation(TransformAdapter3D::Vec3({r.fX, ry, r.fZ}));
            }, 0.0f);

    bound |= this->bindProperty<ScalarValue>(t["rz"], ascope,
            [adapter](const ScalarValue& rz) {
                const auto& r = adapter->getRotation();
                adapter->setRotation(TransformAdapter3D::Vec3({r.fX, r.fY, rz}));
            }, 0.0f);

    // TODO: dispatch 3D transform properties

    if (!bound) {
        return parent;
    }

    return precompose_parent
        ? sksg::Transform::MakeConcat(adapter->refTransform(), std::move(parent))
        : sksg::Transform::MakeConcat(std::move(parent), adapter->refTransform());
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachOpacity(const skjson::ObjectValue& jtransform,
                                                        AnimatorScope* ascope,
                                                        sk_sp<sksg::RenderNode> childNode) const {
    if (!childNode)
        return nullptr;

    auto opacityNode = sksg::OpacityEffect::Make(childNode);

    const auto bound = this->bindProperty<ScalarValue>(jtransform["o"], ascope,
        [opacityNode](const ScalarValue& o) {
            // BM opacity is [0..100]
            opacityNode->setOpacity(o * 0.01f);
        }, 100.0f);
    const auto dispatched = this->dispatchOpacityProperty(opacityNode);

    // We can ignore constant full opacity.
    return (bound || dispatched) ? std::move(opacityNode) : childNode;
}

namespace  {

static SkBlendMode GetBlendMode(const skjson::ObjectValue& jobject,
                                const AnimationBuilder* abuilder) {
    static constexpr SkBlendMode kBlendModeMap[] = {
        SkBlendMode::kSrcOver,    // 0:'normal'
        SkBlendMode::kMultiply,   // 1:'multiply'
        SkBlendMode::kScreen,     // 2:'screen'
        SkBlendMode::kOverlay,    // 3:'overlay
        SkBlendMode::kDarken,     // 4:'darken'
        SkBlendMode::kLighten,    // 5:'lighten'
        SkBlendMode::kColorDodge, // 6:'color-dodge'
        SkBlendMode::kColorBurn,  // 7:'color-burn'
        SkBlendMode::kHardLight,  // 8:'hard-light'
        SkBlendMode::kSoftLight,  // 9:'soft-light'
        SkBlendMode::kDifference, // 10:'difference'
        SkBlendMode::kExclusion,  // 11:'exclusion'
        SkBlendMode::kHue,        // 12:'hue'
        SkBlendMode::kSaturation, // 13:'saturation'
        SkBlendMode::kColor,      // 14:'color'
        SkBlendMode::kLuminosity, // 15:'luminosity'
        SkBlendMode::kPlus,       // 16:'add'
    };

    const auto bm_index = ParseDefault<size_t>(jobject["bm"], 0);
    if (bm_index >= SK_ARRAY_COUNT(kBlendModeMap)) {
            abuilder->log(Logger::Level::kWarning, &jobject,
                          "Unsupported blend mode %lu\n", bm_index);
            return SkBlendMode::kSrcOver;
    }

    return kBlendModeMap[bm_index];
}

} // namespace

sk_sp<sksg::RenderNode> AnimationBuilder::attachBlendMode(const skjson::ObjectValue& jobject,
                                                          sk_sp<sksg::RenderNode> child) const {
    const auto bm = GetBlendMode(jobject, this);
    if (bm != SkBlendMode::kSrcOver) {
        fHasNontrivialBlending = true;
        child = sksg::BlendModeEffect::Make(std::move(child), bm);
    }

    return child;
}

sk_sp<sksg::Path> AnimationBuilder::attachPath(const skjson::Value& jpath,
                                               AnimatorScope* ascope) const {
    auto path_node = sksg::Path::Make();
    return this->bindProperty<ShapeValue>(jpath, ascope,
        [path_node](const ShapeValue& p) {
            // FillType is tracked in the SG node, not in keyframes -- make sure we preserve it.
            auto path = ValueTraits<ShapeValue>::As<SkPath>(p);
            path.setFillType(path_node->getFillType());
            path_node->setPath(path);
        })
        ? path_node
        : nullptr;
}

sk_sp<sksg::Color> AnimationBuilder::attachColor(const skjson::ObjectValue& jcolor,
                                                 AnimatorScope* ascope,
                                                 const char prop_name[]) const {
    auto color_node = sksg::Color::Make(SK_ColorBLACK);

    this->bindProperty<VectorValue>(jcolor[prop_name], ascope,
        [color_node](const VectorValue& c) {
            color_node->setColor(ValueTraits<VectorValue>::As<SkColor>(c));
        });
    this->dispatchColorProperty(color_node);

    return color_node;
}

AnimationBuilder::AnimationBuilder(sk_sp<ResourceProvider> rp, sk_sp<SkFontMgr> fontmgr,
                                   sk_sp<PropertyObserver> pobserver, sk_sp<Logger> logger,
                                   sk_sp<MarkerObserver> mobserver,
                                   Animation::Builder::Stats* stats,
                                   const SkSize& size, float duration, float framerate)
    : fResourceProvider(std::move(rp))
    , fLazyFontMgr(std::move(fontmgr))
    , fPropertyObserver(std::move(pobserver))
    , fLogger(std::move(logger))
    , fMarkerObserver(std::move(mobserver))
    , fStats(stats)
    , fSize(size)
    , fDuration(duration)
    , fFrameRate(framerate)
    , fHasNontrivialBlending(false) {}

std::unique_ptr<sksg::Scene> AnimationBuilder::parse(const skjson::ObjectValue& jroot) {
    this->dispatchMarkers(jroot["markers"]);

    this->parseAssets(jroot["assets"]);
    this->parseFonts(jroot["fonts"], jroot["chars"]);

    AnimatorScope animators;
    auto root = this->attachComposition(jroot, &animators);

    fStats->fAnimatorCount = animators.size();

    return sksg::Scene::Make(std::move(root), std::move(animators));
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
        fPropertyObserver->onColorProperty(fPropertyObserverContext,
            [&]() {
                dispatched = true;
                return std::unique_ptr<ColorPropertyHandle>(new ColorPropertyHandle(c));
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
                return std::unique_ptr<OpacityPropertyHandle>(new OpacityPropertyHandle(o));
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
                return std::unique_ptr<TransformPropertyHandle>(new TransformPropertyHandle(t));
            });
    }

    return dispatched;
}

void AnimationBuilder::AutoPropertyTracker::updateContext(PropertyObserver* observer,
                                                          const skjson::ObjectValue& obj) {

    const skjson::StringValue* name = obj["nm"];

    fBuilder->fPropertyObserverContext = name ? name->begin() : nullptr;
}

} // namespace internal

sk_sp<SkData> ResourceProvider::load(const char[], const char[]) const {
    return nullptr;
}

sk_sp<ImageAsset> ResourceProvider::loadImageAsset(const char path[], const char name[]) const {
    return nullptr;
}

sk_sp<SkData> ResourceProvider::loadFont(const char[], const char[]) const {
    return nullptr;
}

void Logger::log(Level, const char[], const char*) {}

Animation::Builder::Builder()  = default;
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

    memset(&fStats, 0, sizeof(struct Stats));

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
               outPoint = SkTMax(ParseDefault<float>(json["op"], SK_ScalarMax), inPoint),
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
                                       &fStats, size, duration, fps);
    auto scene = builder.parse(json);

    const auto t2 = std::chrono::steady_clock::now();
    fStats.fSceneParseTimeMS = std::chrono::duration<float, std::milli>{t2-t1}.count();
    fStats.fTotalLoadTimeMS  = std::chrono::duration<float, std::milli>{t2-t0}.count();

    if (!scene && fLogger) {
        fLogger->log(Logger::Level::kError, "Could not parse animation.\n");
    }

    uint32_t flags = 0;
    if (builder.hasNontrivialBlending()) {
        flags |= Flags::kRequiresTopLevelIsolation;
    }

    return sk_sp<Animation>(new Animation(std::move(scene),
                                          std::move(version),
                                          size,
                                          inPoint,
                                          outPoint,
                                          duration,
                                          flags));
}

sk_sp<Animation> Animation::Builder::makeFromFile(const char path[]) {
    const auto data = SkData::MakeFromFileName(path);

    return data ? this->make(static_cast<const char*>(data->data()), data->size())
                : nullptr;
}

Animation::Animation(std::unique_ptr<sksg::Scene> scene, SkString version, const SkSize& size,
                     SkScalar inPoint, SkScalar outPoint, SkScalar duration, uint32_t flags)
    : fScene(std::move(scene))
    , fVersion(std::move(version))
    , fSize(size)
    , fInPoint(inPoint)
    , fOutPoint(outPoint)
    , fDuration(duration)
    , fFlags(flags) {

    // In case the client calls render before the first tick.
    this->seek(0);
}

Animation::~Animation() = default;

void Animation::setShowInval(bool show) {
    if (fScene) {
        fScene->setShowInval(show);
    }
}

void Animation::render(SkCanvas* canvas, const SkRect* dstR) const {
    this->render(canvas, dstR, 0);
}

void Animation::render(SkCanvas* canvas, const SkRect* dstR, RenderFlags renderFlags) const {
    TRACE_EVENT0("skottie", TRACE_FUNC);

    if (!fScene)
        return;

    SkAutoCanvasRestore restore(canvas, true);

    const SkRect srcR = SkRect::MakeSize(this->size());
    if (dstR) {
        canvas->concat(SkMatrix::MakeRectToRect(srcR, *dstR, SkMatrix::kCenter_ScaleToFit));
    }

    if ((fFlags & Flags::kRequiresTopLevelIsolation) &&
        !(renderFlags & RenderFlag::kSkipTopLevelIsolation)) {
        // The animation uses non-trivial blending, and needs
        // to be rendered into a separate/transparent layer.
        canvas->saveLayer(srcR, nullptr);
    }

    canvas->clipRect(srcR);

    fScene->render(canvas);
}

void Animation::seek(SkScalar t) {
    TRACE_EVENT0("skottie", TRACE_FUNC);

    if (!fScene)
        return;

    // Per AE/Lottie semantics out_point is exclusive.
    const auto kLastValidFrame = std::nextafter(fOutPoint, fInPoint);

    fScene->animate(SkTPin(fInPoint + t * (fOutPoint - fInPoint), fInPoint, kLastValidFrame));
}

void Animation::seekFrameTime(double t) {
    if (double dur = this->duration()) {
        this->seek((SkScalar)(t / dur));
    }
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
