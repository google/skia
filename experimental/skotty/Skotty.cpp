/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Skotty.h"

#include "SkData.h"
#include "SkJSONCPP.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkStream.h"
#include "SkTArray.h"

// for testing
#include "SkCanvas.h"
#include "SkPaint.h"

namespace skotty {

namespace {

#define LOG SkDebugf

class Comp : public SkNoncopyable {
public:

private:

};

SkScalar ParseScalar(const Json::Value& v, SkScalar defaultValue) {
    return v.isNull() || !v.isConvertibleTo(Json::realValue)
        ? defaultValue : v.asFloat();
}

SkScalar ParseScalar(const Json::Value& obj, const char prop[], SkScalar defaultValue) {
    const auto& v = obj[prop];

    return v.isNull() || !v.isConvertibleTo(Json::realValue)
        ? defaultValue : v.asFloat();
}

SkString ParseString(const Json::Value& obj, const char prop[], const char defaultValue[]) {
    const auto& v = obj[prop];

    return SkString(v.isNull() || !v.isConvertibleTo(Json::stringValue)
        ? defaultValue : v.asCString());
}

int ParseInt(const Json::Value& obj, const char prop[], int defaultValue) {
    const auto& v = obj[prop];

    return v.isNull() || !v.isConvertibleTo(Json::intValue)
        ? defaultValue : v.asInt();
}

bool ParseBool(const Json::Value& obj, const char prop[], bool defaultValue) {
    const auto& v = obj[prop];

    return v.isNull() || !v.isConvertibleTo(Json::booleanValue)
        ? defaultValue : v.asBool();
}

} // namespace

class Animation::Shape : public SkNoncopyable {
public:
    virtual ~Shape() = default;

    static std::unique_ptr<Shape> Make(const Json::Value&);

    virtual void render(SkCanvas*) const {}

protected:
    Shape(const Json::Value& shape)
        : fName     (ParseString(shape, "nm", ""))
        , fMatchName(ParseString(shape, "mn", ""))
    {}

private:
    SkString fName,
             fMatchName;

    typedef SkNoncopyable INHERITED;
};

class Animation::GroupShape : public Shape {
public:
    static std::unique_ptr<Shape> Make(const Json::Value& shape) {
        return std::unique_ptr<GroupShape>(new GroupShape(shape));
    }

    void render(SkCanvas* canvas) const override {
        for (const auto& i : fItems) {
            if (i) i->render(canvas);
        }
    }

private:
    GroupShape(const Json::Value& group) : INHERITED(group) {
        for (const auto& it : group["it"]) {
            if (auto item = Shape::Make(it)) {
                fItems.push_back(std::move(item));
            }
        }
    }

    SkTArray<std::unique_ptr<Shape>> fItems;

    typedef Shape INHERITED;
};

class Animation::ShapeShape : public Shape {
public:
    static std::unique_ptr<Shape> Make(const Json::Value& shape) {
        return std::unique_ptr<ShapeShape>(new ShapeShape(shape));
    }

    void render(SkCanvas* canvas) const override {
        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(1);
        p.setColor(SK_ColorRED);

        canvas->drawPath(fPath, p);
    }

private:
    ShapeShape(const Json::Value& group) : INHERITED(group) {
        SkSTArray<64, SkPoint, true> inPoints, outPoints, vertices;

        if (ParseBool(group["ks"], "a", false)) {
            // TODO animated values
            return;
        }

        for (const auto& p : group["ks"]["k"]["i"]) {
            inPoints.push_back(SkPoint::Make(ParseScalar(p[0], 0), ParseScalar(p[1], 0)));
        }

        for (const auto& p : group["ks"]["k"]["o"]) {
            outPoints.push_back(SkPoint::Make(ParseScalar(p[0], 0), ParseScalar(p[1], 0)));
        }

        for (const auto& p : group["ks"]["k"]["v"]) {
            vertices.push_back(SkPoint::Make(ParseScalar(p[0], 0), ParseScalar(p[1], 0)));
        }

        const auto count = std::min(inPoints.count(),
                                    std::min(outPoints.count(), vertices.count()));

        if (count < 1) {
            return;
        }

        SkPoint prev = vertices[0];
        fPath.moveTo(prev);
        for (int i = 1; i < count; ++i) {
            const auto& v = vertices[i];
            fPath.cubicTo(prev + outPoints[i - 1], v + inPoints[i], v);
            prev = v;
        }

        if (ParseBool(group["ks"]["k"], "c", false)) {
            const auto& v = vertices[0];
            fPath.cubicTo(prev + outPoints[count - 1], v + inPoints[0], v);
            fPath.close();
        }

        LOG("** Shape shape stub - in: %d, out: %d, vert: %d\n", inPoints.count(), outPoints.count(), vertices.count());
        fPath.dump();
    }

    SkPath fPath;

    typedef Shape INHERITED;
};

std::unique_ptr<Animation::Shape> Animation::Shape::Make(const Json::Value& shape) {
    static constexpr struct {
        const char*            fType;
        std::unique_ptr<Shape> (*fProc)(const Json::Value&);
    } gShapeMakers[] = {
        { "gr", GroupShape::Make },
        { "sh", ShapeShape::Make },
    };

    SkString type = ParseString(shape, "ty", "");
    // TODO: binary search?
    size_t idx = 0;
    for (; idx < SK_ARRAY_COUNT(gShapeMakers); ++idx) {
        if (!strcmp(type.c_str(), gShapeMakers[idx].fType)) {
            return gShapeMakers[idx].fProc(shape);
        }
    }

    return nullptr;
}

class Animation::Layer : public SkNoncopyable {
public:
    virtual ~Layer() = default;

    static std::unique_ptr<Layer> Make(const Json::Value&);

    int index() const { return fIndex; }

    // temp for testing
    virtual void render(SkCanvas* canvas) const {}

protected:
    Layer(const Json::Value& layer)
        :        fName(ParseString(layer,  "nm",    ""))
        ,       fIndex(   ParseInt(layer, "ind",    -1))
        ,     fInPoint(ParseScalar(layer,  "ip",     0))
        ,    fOutPoint(ParseScalar(layer,  "op",     0))
        ,   fTimeStart(ParseScalar(layer,  "st",     0))
        , fTimeStretch(ParseScalar(layer,  "sr",     1))
        ,  fAutoOrient(  ParseBool(layer,  "a0", false))
        ,          f3d(  ParseBool(layer, "ddd", false))
    {}

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

private:
    ShapeLayer(const Json::Value& layer) : INHERITED(layer) {
        for (const auto& s : layer["shapes"]) {
            if (auto shape = Shape::Make(s)) {
                fShapes.push_back(std::move(shape));
            }
        }

        LOG("** Shape layer - shapes: %d\n", fShapes.count());
    }

    SkTArray<std::unique_ptr<Shape>> fShapes;

    typedef Layer INHERITED;
};

class Animation::TextLayer : public Layer {
public:
    static std::unique_ptr<Layer> Make(const Json::Value& layer) {
        // TODO
        return std::unique_ptr<TextLayer>(new TextLayer(layer));
    }
private:
    TextLayer(const Json::Value& layer) : INHERITED(layer) {
        LOG("** Shape layer stub\n");
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

    int type = ParseInt(layer, "ty", -1);
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

private:
    Composition(const Json::Value& comp, const Json::Value& layers)
        : fId(ParseString(comp, "id", "")) {

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

    const auto version = ParseString(json, "v", "");
    const auto size = SkSize::Make(ParseScalar(json, "w", -1), ParseScalar(json, "h", -1));

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
}

Animation::~Animation() = default;

void Animation::render(SkCanvas* canvas) const {
    // TODO

    // testing
    fRoot->render(canvas);

    fCompositions.foreach([&](const SkString& key, const std::unique_ptr<Composition>& comp) {
        comp->render(canvas);
    });
}

void Animation::animationTick(SkMSec) {
    // TODO
}

} // namespace skotty
