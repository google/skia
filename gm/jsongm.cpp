/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkParsePath.h"
#include "SkPath.h"
#include "SkJSONCPP.h"
#include "Resources.h"

#if 0
static const char* name_type(Json::ValueType t) {
    const char* gTypes[] = {
        "null", "int", "uint", "real", "string", "bool", "array", "object",
    };
    return gTypes[t];
}
#endif

static void dump(const char name[], const Json::Value& v, int tabs = 0) {
    for (int i = 0; i < tabs; ++i) {
        SkDebugf("\t");
    }
    if (name) {
        SkDebugf("%s: ", name);
    }
    switch (v.type()) {
        case Json::nullValue:    SkDebugf("null\n"); break;
        case Json::intValue:     SkDebugf("int %d\n", v.asInt()); break;
        case Json::uintValue:    SkDebugf("uint %x\n", v.asInt()); break;
        case Json::realValue:    SkDebugf("real %g\n", v.asFloat()); break;
        case Json::stringValue:  SkDebugf("\"%s\"\n", v.asCString()); break;
        case Json::booleanValue: SkDebugf("bool %d\n", v.asBool()); break;
        case Json::arrayValue: {
            int count = v.size();
            SkDebugf("[%d]\n", count);
            for (int i = 0; i < count; ++i) {
                SkString str;
                str.printf("[%d]", i);
                dump(str.c_str(), v[i], tabs + 1);
            }
            break;
        }
        case Json::objectValue: {
            SkDebugf("{%d}\n", v.size());
            auto iter = v.begin();
            auto stop = v.end();
            while (iter != stop) {
                dump(iter.key().asCString(), *iter, tabs + 1);
                ++iter;
            }
            break;
        }
    }
}

template <typename T> int find(const char key[], const T array[], int count) {
    for (int i = 0; i < count; ++i) {
        if (!strcmp(key, array[i].fKey)) {
            return i;
        }
    }
    return -1;
}

struct State;

static void ignored(const char* key) {
    SkDebugf("ignored %s\n", key);
}

struct Effect {
};

struct Layer {
private:
    const int fLayerType;
public:
    Layer(int layerType) : fLayerType(layerType) {
        SkASSERT(layerType >= 0 && layerType <= 5);
    }
    virtual ~Layer() {}

    int layerType() const { return fLayerType; }

    bool    fAO = false;
    int     fBlendMode = -1;
    bool    fDDD = false;
    int     fHasMask = 0;
    int     fLayerIndex = -1;
    int     fInPoint = 0;
    int     fOutPoint = 0;
    SkTDArray<Effect*> fEffects;
    // fKS
    // fMaskProperties
    SkString    fName;
    int     fParentIndex = -1;
    SkString fRefID;
    float   fTimeStretch = 1;
    float   fTimeStart = 0;
    // fTM

    void parse(const Json::Value& v) {
        auto stop = v.end();
        for (auto iter = v.begin(); iter != stop; ++iter) {
            const char* key = iter.key().asCString();
            if (!this->handle(key, *iter)) {
                SkDebugf("unexpected key %s in Layer\n", key);
            }
        }
    }

    virtual bool handle(const char key[], const Json::Value& v) {
        const struct {
            const char* fKey;
            void (*fProc)(Layer*, const Json::Value&);
        } handlers[] = {
            { "ao", [](Layer* l, const Json::Value& v) { l->fAO = v.asBool(); }},
            { "bm", [](Layer* l, const Json::Value& v) { l->fBlendMode = v.asInt(); }},
            { "cl", [](Layer* l, const Json::Value& v) { ignored("cl - class parsed layer name"); }},
            { "ddd", [](Layer* l, const Json::Value& v) { l->fDDD = v.asBool(); }},
            { "ef", [](Layer* l, const Json::Value& v) {
                ignored("ef");
                // Effect[]
            }},
            { "hasMask", [](Layer* l, const Json::Value& v) { l->fHasMask = v.asInt(); }},
            { "ind", [](Layer* l, const Json::Value& v) { l->fLayerIndex = v.asInt(); }},
            { "ip", [](Layer* l, const Json::Value& v) { l->fInPoint = v.asInt(); }},
            { "ks", [](Layer* l, const Json::Value& v) {
                ignored("ks");
                // transform?
            }},
            { "ln", [](Layer* l, const Json::Value& v) { ignored("ln - layer HTML ID"); }},
            { "maskProperties", [](Layer* l, const Json::Value& v) {
                ignored("maskProperties");
                //
            }},
            { "nm", [](Layer* l, const Json::Value& v) { l->fName.set(v.asCString()); }},
            { "op", [](Layer* l, const Json::Value& v) { l->fOutPoint = v.asInt(); }},
            { "parent", [](Layer* l, const Json::Value& v) { l->fParentIndex = v.asInt(); }},
            { "refId", [](Layer* l, const Json::Value& v) { l->fRefID.set(v.asCString()); }},
            { "sr", [](Layer* l, const Json::Value& v) { l->fTimeStretch = v.asFloat(); }},
            { "st", [](Layer* l, const Json::Value& v) { l->fTimeStart = v.asFloat(); }},
            { "tm", [](Layer* l, const Json::Value& v) {
                ignored("tm");
                // properties/valueKeyframed
            }},
            { "ty", [](Layer* l, const Json::Value& v) { SkASSERT(v.asInt() == l->layerType()); }},
        };
        int index = find(key, handlers, SK_ARRAY_COUNT(handlers));
        if (index >= 0) {
            handlers[index].fProc(this, v);
            return true;
        }
        return false;
    }

    static Layer* Make(const Json::Value& v, int index);
    static Layer* Make(const Json::Value& v) {
        const Json::Value& k = v["ty"];
        switch (k.type()) {
            case Json::intValue:
            case Json::uintValue:
                return Make(v, k.asInt());
            default:
                break;
        }
        return nullptr;//new DummyLayer;
    }
};

enum LayerType {
    kComp_LayerType,
    kSolid_LayerType,
    kImage_LayerType,
    kNull_LayerType,
    kShape_LayerType,
    kText_LayerType,
};

struct PreCompLayer : Layer {
    PreCompLayer() : Layer(kComp_LayerType) {}
    ~PreCompLayer() override {}
};

struct SolidLayer : Layer {
    SolidLayer() : Layer(kSolid_LayerType) {}
    ~SolidLayer() override {}
};

struct ImageLayer : Layer {
    ImageLayer() : Layer(kImage_LayerType) {}
    ~ImageLayer() override {}
};

struct NullLayer : Layer {
    NullLayer() : Layer(kNull_LayerType) {}
    ~NullLayer() override {}
};

enum ShapeType {
    kShape_ShapeType,
    kRect_ShapeType,
    kEllipse_ShapeType,
    kStar_ShapeType,
    kFill_ShapeType,
    kGFill_ShapeType,
    kGStroke_ShapeType,
    kStroke_ShapeType,
    kMerge_ShapeType,
    kTrim_ShapeType,
    kGroup_ShapeType,
    kRCorners_ShapeType,
};
ShapeType string_to_ShapeType(const char*);

struct Shape {
private:
    const ShapeType fType;
public:
    int fDirection = 99999;
    SkString fName;
    SkString fMatchName;

    Shape(ShapeType type) : fType(type) {}
    virtual ~Shape() {}

    ShapeType type() const { return fType; }

    void parse(const Json::Value& v) {
        auto stop = v.end();
        for (auto iter = v.begin(); iter != stop; ++iter) {
            const char* key = iter.key().asCString();
            if (!this->handle(key, *iter)) {
                SkDebugf("unhandled %s in Shape\n", key);
            }
        }
    }

    virtual bool handle(const char key[], const Json::Value& v) {
        const struct {
            const char* fKey;
            void (*fProc)(Shape*, const Json::Value&);
        } handlers[] = {
            { "cix",  [](Shape* s, const Json::Value& v) { ignored("cix in Shape"); }},
            { "hd",  [](Shape* s, const Json::Value& v) { ignored("hd in Shape"); }},
            { "ind",  [](Shape* s, const Json::Value& v) { ignored("ind in Shape"); }},
            { "ix",  [](Shape* s, const Json::Value& v) { ignored("ix in Shape"); }},

            { "d",  [](Shape* s, const Json::Value& v) { s->fDirection = v.asInt(); }},
            { "mn", [](Shape* s, const Json::Value& v) { s->fMatchName.set(v.asCString()); }},
            { "nm", [](Shape* s, const Json::Value& v) { s->fName.set(v.asCString()); }},
            { "ks", [](Shape* s, const Json::Value& v) {
                ignored("ks in Shape");
            }},
            { "ty", [](Shape* s, const Json::Value& v) {
                ShapeType st = string_to_ShapeType(v.asCString());
                SkASSERT(st == s->type());
            }},
        };
        int index = find(key, handlers, SK_ARRAY_COUNT(handlers));
        if (index >= 0) {
            handlers[index].fProc(this, v);
            return true;
        }
        return false;
    }

    static Shape* Make(const Json::Value&);
};

struct ShapeShape : Shape {
    ShapeShape() : Shape(kShape_ShapeType) {}
    ~ShapeShape() override {}
};

struct RectShape : Shape {
    RectShape() : Shape(kRect_ShapeType) {}
    ~RectShape() override {}
};

struct EllipseShape : Shape {
    EllipseShape() : Shape(kEllipse_ShapeType) {}
    ~EllipseShape() override {}
};

struct StarShape : Shape {
    StarShape() : Shape(kStar_ShapeType) {}
    ~StarShape() override {}
};

struct FillShape : Shape {
    FillShape() : Shape(kFill_ShapeType) {}
    ~FillShape() override {}
};

struct GFillShape : Shape {
    GFillShape() : Shape(kGFill_ShapeType) {}
    ~GFillShape() override {}
};

struct GStrokeShape : Shape {
    GStrokeShape() : Shape(kGStroke_ShapeType) {}
    ~GStrokeShape() override {}
};

struct StrokeShape : Shape {
    StrokeShape() : Shape(kStroke_ShapeType) {}
    ~StrokeShape() override {}
};

struct MergeShape : Shape {
    MergeShape() : Shape(kMerge_ShapeType) {}
    ~MergeShape() override {}
};

struct TrimShape : Shape {
    TrimShape() : Shape(kTrim_ShapeType) {}
    ~TrimShape() override {}
};

struct GroupShape : Shape {
    GroupShape() : Shape(kGroup_ShapeType) {}
    ~GroupShape() override {}

    SkTDArray<Shape*> fShapes;

    bool handle(const char key[], const Json::Value& v) override {
        const struct {
            const char* fKey;
            void (*fProc)(GroupShape*, const Json::Value&);
        } handlers[] = {
            { "it", [](GroupShape* g, const Json::Value& v) {
                int count = v.size();
                g->fShapes.setCount(count);
                for (int i = 0; i < count; ++i) {
                    g->fShapes[i] = Shape::Make(v[i]);
                }
            }},
        };
        int index = find(key, handlers, SK_ARRAY_COUNT(handlers));
        if (index >= 0) {
            handlers[index].fProc(this, v);
            return true;
        }
        return this->Shape::handle(key, v);
    }
};

struct RCornersShape : Shape {
    RCornersShape() : Shape(kRCorners_ShapeType) {}
    ~RCornersShape() override {}
};

struct ShapeTypeRec {
    ShapeType   fType;
    const char* fName;
    Shape* (*fMaker)();
};
const ShapeTypeRec gShapeTypeRecs[] = {
    { kShape_ShapeType,     "sh", []() -> Shape* { return new ShapeShape; }},
    { kRect_ShapeType,      "rc", []() -> Shape* { return new RectShape; }},
    { kEllipse_ShapeType,   "el", []() -> Shape* { return new EllipseShape; }},
    { kStar_ShapeType,      "sr", []() -> Shape* { return new StarShape; }},
    { kFill_ShapeType,      "fl", []() -> Shape* { return new FillShape; }},
    { kGFill_ShapeType,     "gf", []() -> Shape* { return new GFillShape; }},
    { kGStroke_ShapeType,   "gs", []() -> Shape* { return new GStrokeShape; }},
    { kStroke_ShapeType,    "st", []() -> Shape* { return new StrokeShape; }},
    { kMerge_ShapeType,     "??", []() -> Shape* { return new MergeShape; }},
    { kTrim_ShapeType,      "tr", []() -> Shape* { return new TrimShape; }},
    { kGroup_ShapeType,     "gr", []() -> Shape* { return new GroupShape; }},
    { kRCorners_ShapeType,  "rd", []() -> Shape* { return new RCornersShape; }},
};
ShapeType string_to_ShapeType(const char* name) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gShapeTypeRecs); ++i) {
        if (!strcmp(name, gShapeTypeRecs[i].fName)) {
            return gShapeTypeRecs[i].fType;
        }
    }
    return (ShapeType)-1;
}
Shape* Shape::Make(const Json::Value& v) {
    Shape* shape = nullptr;
    const char* type = v["ty"].asCString();
    if (type) {
        for (size_t i = 0; i < SK_ARRAY_COUNT(gShapeTypeRecs); ++i) {
            if (!strcmp(type, gShapeTypeRecs[i].fName)) {
                shape = gShapeTypeRecs[i].fMaker();
                break;
            }
        }
    }
    if (shape) {
        shape->parse(v);
    }
    return shape;
}

struct ShapeLayer : Layer {
    ShapeLayer() : Layer(kShape_LayerType) {}
    ~ShapeLayer() override {}

    SkTDArray<Shape*> fShapes;

    bool handle(const char key[], const Json::Value& v) override {
        const struct {
            const char* fKey;
            void (*fProc)(ShapeLayer*, const Json::Value&);
        } handlers[] = {
            { "shapes", [](ShapeLayer* l, const Json::Value& v) {
                int count = v.size();
                l->fShapes.setCount(count);
                for (int i = 0; i < count; ++i) {
                    l->fShapes[i] = Shape::Make(v[i]);
                }
            }},
        };
        int index = find(key, handlers, SK_ARRAY_COUNT(handlers));
        if (index >= 0) {
            handlers[index].fProc(this, v);
            return true;
        }
        return this->Layer::handle(key, v);
    }
};

struct TextLayer : Layer {
    TextLayer() : Layer(kText_LayerType) {}
    ~TextLayer() override {}
};

struct DummyLayer : Layer {
    DummyLayer() : Layer(kNull_LayerType) {}
    ~DummyLayer() override {}
};

Layer* Layer::Make(const Json::Value& v, int index) {
    Layer* (*makers[])() = {
        []() -> Layer* { return new PreCompLayer; },
        []() -> Layer* { return new SolidLayer; },
        []() -> Layer* { return new ImageLayer; },
        []() -> Layer* { return new NullLayer; },
        []() -> Layer* { return new ShapeLayer; },
        []() -> Layer* { return new TextLayer; },
    };
    if ((unsigned)index >= SK_ARRAY_COUNT(makers)) {
        return new DummyLayer;
    }
    Layer* layer = makers[index]();
    layer->parse(v);
    return layer;
}

struct State {
    float fFrameRate = 0;
    float fHeight = 0;
    float fWidth = 0;
    float fInPointTime = 0;
    float fOutPointTime = 0;
    SkString fVersion;

    SkTDArray<Layer*>  fLayers;
    SkString fLayerName;


    bool fDDD = false;

    void dump() const {
        SkDebugf("frame-rate: %g, dim:[%g %g], times:[%g %g], version:'%s' name:'%s'\n",
                 fFrameRate, fWidth, fHeight, fInPointTime, fOutPointTime, fVersion.c_str(),
                 fLayerName.c_str());
    }
};

static void parse_root(const Json::Value& root, State* state) {
    const struct {
        const char* fKey;
        void (*fProc)(const Json::Value&, State*);
    } handlers[] = {
        { "assets", [](const Json::Value& v, State* state){
            if (v.size() > 0) {
                dump("assets", v);
            }
        }},
        { "chars", [](const Json::Value& v, State* state){
            if (v.size() > 0) {
                dump("chars", v);
            }
        }},
        { "fr", [](const Json::Value& v, State* s) { s->fFrameRate = v.asFloat(); }},
        { "h", [](const Json::Value& v, State* s) { s->fHeight = v.asFloat(); }},
        { "ip", [](const Json::Value& v, State* s) { s->fInPointTime = v.asFloat(); }},
        { "layers", [](const Json::Value& v, State* s) {
            int count = v.size();
            s->fLayers.setCount(count);
            for (int i = 0; i < count; ++i) {
                s->fLayers[i] = Layer::Make(v[i]);
            }
        }},
        { "op", [](const Json::Value& v, State* s) { s->fOutPointTime = v.asFloat(); }},
        { "v", [](const Json::Value& v, State* s) { s->fVersion.set(v.asCString()); }},
        { "w", [](const Json::Value& v, State* s) { s->fWidth = v.asFloat(); }},
        { "ddd", [](const Json::Value& v, State* s) { s->fDDD = v.asBool(); }},
        { "nm", [](const Json::Value& v, State* s) { s->fLayerName.set(v.asCString()); }},
    };

    auto stop = root.end();
    for (auto iter = root.begin(); iter != stop; ++iter) {
        const char* key = iter.key().asCString();
        const int index = find(key, handlers, SK_ARRAY_COUNT(handlers));
        if (index >= 0) {
            handlers[index].fProc(*iter, state);
        } else {
            SkDebugf("Unrecognized %s in root\n", key);
        }
    }
}

DEF_SIMPLE_GM(json, canvas, 500, 600) {
    auto data = SkData::MakeFromFileName("/skia/menu-home.json");
    const char* str = (const char*)data->data();
    const char* end = str + data->size();

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(str, end, root) || !reader.good()) {
        std::string err = reader.getFormattedErrorMessages();
        SkDebugf("failed to parse json: %s\n", err.c_str());
        return;
    }

    SkASSERT(root.type() == Json::objectValue);
    if (false) { dump(nullptr, root, -1); return; }

    State state;
    parse_root(root, &state);

    state.dump();
}
