/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottyShape.h"

#include "SkottyPriv.h"
#include "SkottyProperties.h"
#include "SkPaint.h"
#include "SkSGGroup.h"

// testing
#include "SkCanvas.h"
#include "SkPath.h"

namespace skotty {

namespace {

class Group final : public ShapeBase {
public:
    static std::unique_ptr<ShapeBase> Make(const Json::Value& shape) {
        return std::unique_ptr<Group>(new Group(shape));
    }

    void render(SkCanvas* canvas) const override {
        for (const auto& i : fItems) {
            if (i) i->render(canvas);
        }
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    Group(const Json::Value& group) : INHERITED(group) {
        for (const auto& it : group["it"]) {
            if (auto item = ShapeBase::Make(it)) {
                fItems.push_back(std::move(item));
            }
        }
    }

    SkTArray<std::unique_ptr<ShapeBase>> fItems;

    typedef ShapeBase INHERITED;
};

class Shape final : public ShapeBase {
public:
    static std::unique_ptr<ShapeBase> Make(const Json::Value& shape) {
        auto shapeProperty = ShapeProperty::Make(shape["ks"]);

        return shapeProperty
            ? std::unique_ptr<Shape>(new Shape(shape, std::move(shapeProperty)))
            : nullptr;
    }

    void render(SkCanvas* canvas) const override {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(1);
        p.setColor(SK_ColorRED);

        canvas->drawPath((*fShape)(0).asPath(), p);
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    Shape(const Json::Value& shape, std::unique_ptr<ShapeProperty> shapeProperty)
        : INHERITED(shape)
        , fShape(std::move(shapeProperty)) {}

    std::unique_ptr<ShapeProperty> fShape;

    typedef ShapeBase INHERITED;
};

class Fill final : public ShapeBase {
public:
    static std::unique_ptr<ShapeBase> Make(const Json::Value& shape) {
        auto color   = VectorProperty::Make(shape["c"]);
        auto opacity = ScalarProperty::Make(shape["o"]);

        if (!color || !opacity)
            return nullptr;
        return std::unique_ptr<Fill>(new Fill(shape,
                                              std::move(color),
                                              std::move(opacity)));
    }

    void render(SkCanvas* canvas) const override {
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    Fill(const Json::Value& shape,
         std::unique_ptr<VectorProperty> color,
         std::unique_ptr<ScalarProperty> opacity)
        : INHERITED(shape)
        , fColor(std::move(color))
        , fOpacity(std::move(opacity)) {
        LOG("*** FILLSHAPE\n");
    }

    std::unique_ptr<VectorProperty> fColor;
    std::unique_ptr<ScalarProperty> fOpacity;

    typedef ShapeBase INHERITED;
};

class Stroke final : public ShapeBase {
public:
    static std::unique_ptr<ShapeBase> Make(const Json::Value& shape) {
        auto color   = VectorProperty::Make(shape["c"]);
        auto opacity = ScalarProperty::Make(shape["o"]);
        auto width   = ScalarProperty::Make(shape["w"]);

        if (!color || !opacity || !width)
            return nullptr;

        return std::unique_ptr<Stroke>(new Stroke(shape,
                                                  std::move(color),
                                                  std::move(opacity),
                                                  std::move(width)));
    }

    void render(SkCanvas* canvas) const override {
    }

protected:
    sk_sp<sksg::RenderNode> onAttach() const override {
        // TODO
        return nullptr;
    }

private:
    Stroke(const Json::Value& shape,
           std::unique_ptr<VectorProperty> color,
           std::unique_ptr<ScalarProperty> opacity,
           std::unique_ptr<ScalarProperty> width)
        : INHERITED(shape)
        , fColor(std::move(color))
        , fOpacity(std::move(opacity))
        , fWidth(std::move(width))
        , fMiterLimit(ParseScalar(shape["ml"], 4)) {

        static constexpr SkPaint::Join gJoins[] = {
            SkPaint::kMiter_Join,
            SkPaint::kRound_Join,
            SkPaint::kBevel_Join,
        };

        static constexpr SkPaint::Cap gCaps[] = {
            SkPaint::kButt_Cap,
            SkPaint::kRound_Cap,
            SkPaint::kSquare_Cap,
        };

        fJoin = gJoins[SkTPin<int>(ParseInt(shape["lj"], 0) - 1, 0, SK_ARRAY_COUNT(gJoins))];
        fCap  = gCaps [SkTPin<int>(ParseInt(shape["lc"], 0) - 1, 0, SK_ARRAY_COUNT(gCaps ))];

        LOG("*** STROKESHAPE\n");
    }

    std::unique_ptr<VectorProperty> fColor;
    std::unique_ptr<ScalarProperty> fOpacity;
    std::unique_ptr<ScalarProperty> fWidth;

    SkPaint::Join fJoin;
    SkPaint::Cap  fCap;
    SkScalar      fMiterLimit;

    typedef ShapeBase INHERITED;
};

} // namespace

ShapeBase::ShapeBase(const Json::Value& shape)
    : fName     (ParseString(shape["nm"], ""))
    , fMatchName(ParseString(shape["mn"], "")) {}

std::unique_ptr<ShapeBase> ShapeBase::Make(const Json::Value& shape) {
    static constexpr struct {
        const char*                fType;
        std::unique_ptr<ShapeBase> (*fProc)(const Json::Value&);
    } gShapeMakers[] = {
        { "fl", Fill::Make   },
        { "gr", Group::Make  },
        { "sh", Shape::Make  },
        { "st", Stroke::Make },
    };

    SkString type = ParseString(shape["ty"], "");
    // TODO: binary search?
    size_t idx = 0;
    for (; idx < SK_ARRAY_COUNT(gShapeMakers); ++idx) {
        if (!strcmp(type.c_str(), gShapeMakers[idx].fType)) {
            return gShapeMakers[idx].fProc(shape);
        }
    }

    return nullptr;
}

} // namespace skotty
