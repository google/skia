/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Skotty.h"

#include "SkottyPriv.h"
#include "SkottyProperties.h"
#include "SkottyShape.h"
#include "SkData.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkSGInvalidationController.h"
#include "SkSGGroup.h"
#include "SkStream.h"
#include "SkTArray.h"

// for testing
#include "SkCanvas.h"
#include "SkPaint.h"

namespace skotty {

class Animation::Layer : public SkNoncopyable {
public:
    virtual ~Layer() = default;

    static std::unique_ptr<Layer> Make(const Json::Value&);

    int index() const { return fIndex; }

    sk_sp<sksg::RenderNode> attach() const {
        return this->onAttach();
    }

    // temp for testing
    virtual void render(SkCanvas* canvas) const {}

protected:
    Layer(const Json::Value& layer)
        : fName       (ParseString(layer["nm"] ,    ""))
        , fIndex      (ParseInt   (layer["ind"],    -1))
        , fInPoint    (ParseScalar(layer["ip"] ,     0))
        , fOutPoint   (ParseScalar(layer["op"] ,     0))
        , fTimeStart  (ParseScalar(layer["st"] ,     0))
        , fTimeStretch(ParseScalar(layer["sr"] ,     1))
        , fAutoOrient (ParseBool  (layer["a0"] , false))
        , f3d         (ParseBool  (layer["ddd"], false))
    {}

    virtual sk_sp<sksg::RenderNode> onAttach() const = 0;

private:
    SkString fName;
    int      fIndex;
    SkScalar fInPoint,
             fOutPoint,
             fTimeStart,
             fTimeStretch;
    bool     fAutoOrient,
             f3d;

    typedef SkNoncopyable INHERITED;
};

class Animation::CompLayer : public Layer {
public:
    static std::unique_ptr<Layer> Make(const Json::Value& layer) {
        // TODO
        return std::unique_ptr<CompLayer>(new CompLayer(layer));
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    CompLayer(const Json::Value& layer) : INHERITED(layer) {
        LOG("** Comp layer stub\n");
    }

    typedef Layer INHERITED;
};

class Animation::SolidLayer : public Layer {
public:
    static std::unique_ptr<Layer> Make(const Json::Value& layer) {
        // TODO
        return std::unique_ptr<SolidLayer>(new SolidLayer(layer));
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    SolidLayer(const Json::Value& layer) : INHERITED(layer) {
        LOG("** Solid layer stub\n");
    }

    typedef Layer INHERITED;
};

class Animation::ImageLayer : public Layer {
public:
    static std::unique_ptr<Layer> Make(const Json::Value& layer) {
        // TODO
        return std::unique_ptr<ImageLayer>(new ImageLayer(layer));
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    ImageLayer(const Json::Value& layer) : INHERITED(layer) {
        LOG("** Image layer stub\n");
    }

    typedef Layer INHERITED;
};

class Animation::NullLayer : public Layer {
public:
    static std::unique_ptr<Layer> Make(const Json::Value& layer) {
        // TODO
        return std::unique_ptr<NullLayer>(new NullLayer(layer));
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    NullLayer(const Json::Value& layer) : INHERITED(layer) {
        LOG("** Null layer stub\n");
    }

    typedef Layer INHERITED;
};

class Animation::ShapeLayer : public Layer {
public:
    static std::unique_ptr<Layer> Make(const Json::Value& layer) {
        // TODO
        return std::unique_ptr<ShapeLayer>(new ShapeLayer(layer));
    }

    // temp for testing
    void render(SkCanvas* canvas) const override {
        for (const auto& s : fShapes) {
            if (s) s->render(canvas);
        }
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        auto grp = sksg::Group::Make();

        for (const auto& s : fShapes) {
            if (auto child = s->attach()) {
                grp->addChild(std::move(child));
            }
        }

        return grp;
    }

private:
    ShapeLayer(const Json::Value& layer) : INHERITED(layer) {
        for (const auto& s : layer["shapes"]) {
            if (auto shape = ShapeBase::Make(s)) {
                fShapes.push_back(std::move(shape));
            }
        }
    }

    SkTArray<std::unique_ptr<ShapeBase>> fShapes;

    typedef Layer INHERITED;
};

class Animation::TextLayer : public Layer {
public:
    static std::unique_ptr<Layer> Make(const Json::Value& layer) {
        // TODO
        return std::unique_ptr<TextLayer>(new TextLayer(layer));
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    TextLayer(const Json::Value& layer) : INHERITED(layer) {
        LOG("** Text layer stub\n");
    }

    typedef Layer INHERITED;
};

std::unique_ptr<Animation::Layer> Animation::Layer::Make(const Json::Value& layer) {
    using MakerT = std::unique_ptr<Layer> (*)(const Json::Value&);
    static constexpr MakerT gLayerMakers[] = {
        Animation::CompLayer::Make,  // 'ty': 0
        Animation::SolidLayer::Make, // 'ty': 1
        Animation::ImageLayer::Make, // 'ty': 2
        Animation::NullLayer::Make,  // 'ty': 3
        Animation::ShapeLayer::Make, // 'ty': 4
        Animation::TextLayer::Make,  // 'ty': 5
    };

    int type = ParseInt(layer["ty"], -1);
    if (type < 0 || type >= SkTo<int>(SK_ARRAY_COUNT(gLayerMakers))) {
        return nullptr;
    }

    return gLayerMakers[type](layer);
}

class Animation::Composition : public SkNoncopyable {
public:
    static std::unique_ptr<Composition> Make(const Json::Value& comp) {
        // A valid compostion requires layers.
        const auto& layers = comp["layers"];
        return layers.isNull()
            ? nullptr
            : std::unique_ptr<Composition>(new Composition(comp, layers));
    }

    const SkString& id() const { return fId; }

    // temp for testing
    void render(SkCanvas* canvas) const {
        for (const auto& l : fLayers) {
            if (l) l->render(canvas);
        }
    }

    sk_sp<sksg::RenderNode> attach() const {
        auto group = sksg::Group::Make();

        for (const auto& l : fLayers) {
            if (!l)
                continue;

            if (auto child = l->attach()) {
                group->addChild(std::move(child));
            }
        }

        return group;
    }

private:
    Composition(const Json::Value& comp, const Json::Value& layers)
        : fId(ParseString(comp["id"], "")) {

        for (const auto& l : layers) {
            if (auto layer = Layer::Make(l)) {
                int index = layer->index();
                if (index >= 0) {
                    fLayers.resize_back(index + 1);
                    fLayers[index] = std::move(layer);
                }
            }
        }
        LOG("** new composition - id: '%s', layer slots: %d\n", fId.c_str(), fLayers.count());
    }

    SkString                         fId;
    SkTArray<std::unique_ptr<Layer>> fLayers;
};

std::unique_ptr<Animation> Animation::Make(SkStream* stream) {
    if (!stream->hasLength()) {
        // TODO: handle explicit buffering?
        LOG("!! cannot parse streaming content\n");
        return nullptr;
    }

    Json::Value json;
    {
        auto data = SkData::MakeFromStream(stream, stream->getLength());
        if (!data) {
            LOG("!! could not read stream\n");
            return nullptr;
        }

        Json::Reader reader;

        auto dataStart = static_cast<const char*>(data->data());
        if (!reader.parse(dataStart, dataStart + data->size(), json, false)) {
            LOG("!! failed to parse json: %s\n", reader.getFormattedErrorMessages().c_str());
            return nullptr;
        }
    }

    const auto version = ParseString(json["v"], "");
    const auto size = SkSize::Make(ParseScalar(json["w"], -1), ParseScalar(json["h"], -1));

    if (size.isEmpty() || version.isEmpty()) {
        LOG("!! invalid animation params (version: %s, size: [%f %f])", version.c_str(),
            size.width(), size.height());
        return nullptr;
    }

    auto root = Composition::Make(json);
    if (!root) {
        LOG("!! could not parse root composition\n");
        return nullptr;
    }
    return std::unique_ptr<Animation>(new Animation(std::move(version), size, std::move(root), json));
}

Animation::Animation(SkString version, const SkSize& size, std::unique_ptr<Composition> root,
             const Json::Value& json)
    : fVersion(std::move(version))
    , fSize(size)
    , fRoot(std::move(root)) {

    SkASSERT(fRoot);

    for (const auto& asset : json["assets"]) {
        // ugh - no explicit asset typing?!

        if (auto comp = Composition::Make(asset)) {
            fCompositions.set(comp->id(), std::move(comp));
        }
    }

    fDom = fRoot->attach();
}

Animation::~Animation() = default;

void Animation::render(SkCanvas* canvas) const {
    // TODO

    // testing
    fRoot->render(canvas);

    fCompositions.foreach([&](const SkString& key, const std::unique_ptr<Composition>& comp) {
        comp->render(canvas);
    });

    if (!fDom)
        return;

    sksg::InvalidationController ic;
    fDom->revalidate(&ic, SkMatrix::I());

    // TODO: proper inval
    fDom->render(canvas);
}

void Animation::animationTick(SkMSec) {
    // TODO
}

} // namespace skotty
