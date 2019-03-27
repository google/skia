/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottiePriv.h"

#include "SkJSON.h"
#include "SkottieAdapter.h"
#include "SkottieJson.h"
#include "SkottieValue.h"
#include "SkPath.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGeometryTransform.h"
#include "SkSGGradient.h"
#include "SkSGGroup.h"
#include "SkSGMerge.h"
#include "SkSGPath.h"
#include "SkSGRect.h"
#include "SkSGRoundEffect.h"
#include "SkSGTransform.h"
#include "SkSGTrimEffect.h"

#include <algorithm>
#include <iterator>

namespace skottie {
namespace internal {

namespace {

sk_sp<sksg::GeometryNode> AttachPathGeometry(const skjson::ObjectValue& jpath,
                                             const AnimationBuilder* abuilder,
                                             AnimatorScope* ascope) {
    return abuilder->attachPath(jpath["ks"], ascope);
}

sk_sp<sksg::GeometryNode> AttachRRectGeometry(const skjson::ObjectValue& jrect,
                                              const AnimationBuilder* abuilder,
                                              AnimatorScope* ascope) {
    auto rect_node = sksg::RRect::Make();
    rect_node->setDirection(ParseDefault(jrect["d"], -1) == 3 ? SkPath::kCCW_Direction
                                                              : SkPath::kCW_Direction);
    rect_node->setInitialPointIndex(2); // starting point: (Right, Top - radius.y)

    auto adapter = sk_make_sp<RRectAdapter>(rect_node);

    auto p_attached = abuilder->bindProperty<VectorValue>(jrect["p"], ascope,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    auto s_attached = abuilder->bindProperty<VectorValue>(jrect["s"], ascope,
        [adapter](const VectorValue& s) {
            adapter->setSize(ValueTraits<VectorValue>::As<SkSize>(s));
        });
    auto r_attached = abuilder->bindProperty<ScalarValue>(jrect["r"], ascope,
        [adapter](const ScalarValue& r) {
            adapter->setRadius(SkSize::Make(r, r));
        });

    if (!p_attached && !s_attached && !r_attached) {
        return nullptr;
    }

    return std::move(rect_node);
}

sk_sp<sksg::GeometryNode> AttachEllipseGeometry(const skjson::ObjectValue& jellipse,
                                                const AnimationBuilder* abuilder,
                                                AnimatorScope* ascope) {
    auto rect_node = sksg::RRect::Make();
    rect_node->setDirection(ParseDefault(jellipse["d"], -1) == 3 ? SkPath::kCCW_Direction
                                                                 : SkPath::kCW_Direction);
    rect_node->setInitialPointIndex(1); // starting point: (Center, Top)

    auto adapter = sk_make_sp<RRectAdapter>(rect_node);

    auto p_attached = abuilder->bindProperty<VectorValue>(jellipse["p"], ascope,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    auto s_attached = abuilder->bindProperty<VectorValue>(jellipse["s"], ascope,
        [adapter](const VectorValue& s) {
            const auto sz = ValueTraits<VectorValue>::As<SkSize>(s);
            adapter->setSize(sz);
            adapter->setRadius(SkSize::Make(sz.width() / 2, sz.height() / 2));
        });

    if (!p_attached && !s_attached) {
        return nullptr;
    }

    return std::move(rect_node);
}

sk_sp<sksg::GeometryNode> AttachPolystarGeometry(const skjson::ObjectValue& jstar,
                                                 const AnimationBuilder* abuilder,
                                                 AnimatorScope* ascope) {
    static constexpr PolyStarAdapter::Type gTypes[] = {
        PolyStarAdapter::Type::kStar, // "sy": 1
        PolyStarAdapter::Type::kPoly, // "sy": 2
    };

    const auto type = ParseDefault<size_t>(jstar["sy"], 0) - 1;
    if (type >= SK_ARRAY_COUNT(gTypes)) {
        abuilder->log(Logger::Level::kError, &jstar, "Unknown polystar type.");
        return nullptr;
    }

    auto path_node = sksg::Path::Make();
    auto adapter = sk_make_sp<PolyStarAdapter>(path_node, gTypes[type]);

    abuilder->bindProperty<VectorValue>(jstar["p"], ascope,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    abuilder->bindProperty<ScalarValue>(jstar["pt"], ascope,
        [adapter](const ScalarValue& pt) {
            adapter->setPointCount(pt);
        });
    abuilder->bindProperty<ScalarValue>(jstar["ir"], ascope,
        [adapter](const ScalarValue& ir) {
            adapter->setInnerRadius(ir);
        });
    abuilder->bindProperty<ScalarValue>(jstar["or"], ascope,
        [adapter](const ScalarValue& otr) {
            adapter->setOuterRadius(otr);
        });
    abuilder->bindProperty<ScalarValue>(jstar["is"], ascope,
        [adapter](const ScalarValue& is) {
            adapter->setInnerRoundness(is);
        });
    abuilder->bindProperty<ScalarValue>(jstar["os"], ascope,
        [adapter](const ScalarValue& os) {
            adapter->setOuterRoundness(os);
        });
    abuilder->bindProperty<ScalarValue>(jstar["r"], ascope,
        [adapter](const ScalarValue& r) {
            adapter->setRotation(r);
        });

    return std::move(path_node);
}

sk_sp<sksg::Gradient> AttachGradient(const skjson::ObjectValue& jgrad,
                                     const AnimationBuilder* abuilder, AnimatorScope* ascope) {
    const skjson::ObjectValue* stops = jgrad["g"];
    if (!stops)
        return nullptr;

    const auto stopCount = ParseDefault<int>((*stops)["p"], -1);
    if (stopCount < 0)
        return nullptr;

    sk_sp<sksg::Gradient> gradient_node;
    sk_sp<GradientAdapter> adapter;

    if (ParseDefault<int>(jgrad["t"], 1) == 1) {
        auto linear_node = sksg::LinearGradient::Make();
        adapter = sk_make_sp<LinearGradientAdapter>(linear_node, stopCount);
        gradient_node = std::move(linear_node);
    } else {
        auto radial_node = sksg::RadialGradient::Make();
        adapter = sk_make_sp<RadialGradientAdapter>(radial_node, stopCount);

        // TODO: highlight, angle
        gradient_node = std::move(radial_node);
    }

    abuilder->bindProperty<VectorValue>((*stops)["k"], ascope,
        [adapter](const VectorValue& stops) {
            adapter->setColorStops(stops);
        });
    abuilder->bindProperty<VectorValue>(jgrad["s"], ascope,
        [adapter](const VectorValue& s) {
            adapter->setStartPoint(ValueTraits<VectorValue>::As<SkPoint>(s));
        });
    abuilder->bindProperty<VectorValue>(jgrad["e"], ascope,
        [adapter](const VectorValue& e) {
            adapter->setEndPoint(ValueTraits<VectorValue>::As<SkPoint>(e));
        });

    return gradient_node;
}

sk_sp<sksg::PaintNode> AttachPaint(const skjson::ObjectValue& jpaint,
                                   const AnimationBuilder* abuilder, AnimatorScope* ascope,
                                   sk_sp<sksg::PaintNode> paint_node) {
    if (paint_node) {
        paint_node->setAntiAlias(true);

        abuilder->bindProperty<ScalarValue>(jpaint["o"], ascope,
            [paint_node](const ScalarValue& o) {
                // BM opacity is [0..100]
                paint_node->setOpacity(o * 0.01f);
        });
    }

    return paint_node;
}

sk_sp<sksg::PaintNode> AttachStroke(const skjson::ObjectValue& jstroke,
                                    const AnimationBuilder* abuilder, AnimatorScope* ascope,
                                    sk_sp<sksg::PaintNode> stroke_node) {
    if (!stroke_node)
        return nullptr;

    stroke_node->setStyle(SkPaint::kStroke_Style);

    abuilder->bindProperty<ScalarValue>(jstroke["w"], ascope,
        [stroke_node](const ScalarValue& w) {
            stroke_node->setStrokeWidth(w);
        });

    stroke_node->setStrokeMiter(ParseDefault<SkScalar>(jstroke["ml"], 4.0f));

    static constexpr SkPaint::Join gJoins[] = {
        SkPaint::kMiter_Join,
        SkPaint::kRound_Join,
        SkPaint::kBevel_Join,
    };
    stroke_node->setStrokeJoin(gJoins[SkTMin<size_t>(ParseDefault<size_t>(jstroke["lj"], 1) - 1,
                                                     SK_ARRAY_COUNT(gJoins) - 1)]);

    static constexpr SkPaint::Cap gCaps[] = {
        SkPaint::kButt_Cap,
        SkPaint::kRound_Cap,
        SkPaint::kSquare_Cap,
    };
    stroke_node->setStrokeCap(gCaps[SkTMin<size_t>(ParseDefault<size_t>(jstroke["lc"], 1) - 1,
                                                   SK_ARRAY_COUNT(gCaps) - 1)]);

    return stroke_node;
}

sk_sp<sksg::PaintNode> AttachColorFill(const skjson::ObjectValue& jfill,
                                       const AnimationBuilder* abuilder, AnimatorScope* ascope) {
    return AttachPaint(jfill, abuilder, ascope, abuilder->attachColor(jfill, ascope, "c"));
}

sk_sp<sksg::PaintNode> AttachGradientFill(const skjson::ObjectValue& jfill,
                                          const AnimationBuilder* abuilder, AnimatorScope* ascope) {
    return AttachPaint(jfill, abuilder, ascope, AttachGradient(jfill, abuilder, ascope));
}

sk_sp<sksg::PaintNode> AttachColorStroke(const skjson::ObjectValue& jstroke,
                                         const AnimationBuilder* abuilder,
                                         AnimatorScope* ascope) {
    return AttachStroke(jstroke, abuilder, ascope,
                        AttachPaint(jstroke, abuilder, ascope,
                                    abuilder->attachColor(jstroke, ascope, "c")));
}

sk_sp<sksg::PaintNode> AttachGradientStroke(const skjson::ObjectValue& jstroke,
                                            const AnimationBuilder* abuilder,
                                            AnimatorScope* ascope) {
    return AttachStroke(jstroke, abuilder, ascope,
                        AttachPaint(jstroke, abuilder, ascope,
                                    AttachGradient(jstroke, abuilder, ascope)));
}

sk_sp<sksg::Merge> Merge(std::vector<sk_sp<sksg::GeometryNode>>&& geos, sksg::Merge::Mode mode) {
    std::vector<sksg::Merge::Rec> merge_recs;
    merge_recs.reserve(geos.size());

    for (auto& geo : geos) {
        merge_recs.push_back(
            {std::move(geo), merge_recs.empty() ? sksg::Merge::Mode::kMerge : mode});
    }

    return sksg::Merge::Make(std::move(merge_recs));
}

std::vector<sk_sp<sksg::GeometryNode>> AttachMergeGeometryEffect(
        const skjson::ObjectValue& jmerge, const AnimationBuilder*, AnimatorScope*,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {
    static constexpr sksg::Merge::Mode gModes[] = {
        sksg::Merge::Mode::kMerge,      // "mm": 1
        sksg::Merge::Mode::kUnion,      // "mm": 2
        sksg::Merge::Mode::kDifference, // "mm": 3
        sksg::Merge::Mode::kIntersect,  // "mm": 4
        sksg::Merge::Mode::kXOR      ,  // "mm": 5
    };

    const auto mode = gModes[SkTMin<size_t>(ParseDefault<size_t>(jmerge["mm"], 1) - 1,
                                            SK_ARRAY_COUNT(gModes) - 1)];

    std::vector<sk_sp<sksg::GeometryNode>> merged;
    merged.push_back(Merge(std::move(geos), mode));

    return merged;
}

std::vector<sk_sp<sksg::GeometryNode>> AttachTrimGeometryEffect(
        const skjson::ObjectValue& jtrim, const AnimationBuilder* abuilder, AnimatorScope* ascope,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {

    enum class Mode {
        kMerged,   // "m": 1
        kSeparate, // "m": 2
    } gModes[] = { Mode::kMerged, Mode::kSeparate };

    const auto mode = gModes[SkTMin<size_t>(ParseDefault<size_t>(jtrim["m"], 1) - 1,
                                            SK_ARRAY_COUNT(gModes) - 1)];

    std::vector<sk_sp<sksg::GeometryNode>> inputs;
    if (mode == Mode::kMerged) {
        inputs.push_back(Merge(std::move(geos), sksg::Merge::Mode::kMerge));
    } else {
        inputs = std::move(geos);
    }

    std::vector<sk_sp<sksg::GeometryNode>> trimmed;
    trimmed.reserve(inputs.size());
    for (const auto& i : inputs) {
        auto trimEffect = sksg::TrimEffect::Make(i);
        trimmed.push_back(trimEffect);

        const auto adapter = sk_make_sp<TrimEffectAdapter>(std::move(trimEffect));
        abuilder->bindProperty<ScalarValue>(jtrim["s"], ascope,
            [adapter](const ScalarValue& s) {
                adapter->setStart(s);
            });
        abuilder->bindProperty<ScalarValue>(jtrim["e"], ascope,
            [adapter](const ScalarValue& e) {
                adapter->setEnd(e);
            });
        abuilder->bindProperty<ScalarValue>(jtrim["o"], ascope,
            [adapter](const ScalarValue& o) {
                adapter->setOffset(o);
            });
    }

    return trimmed;
}

std::vector<sk_sp<sksg::GeometryNode>> AttachRoundGeometryEffect(
        const skjson::ObjectValue& jtrim, const AnimationBuilder* abuilder, AnimatorScope* ascope,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {

    std::vector<sk_sp<sksg::GeometryNode>> rounded;
    rounded.reserve(geos.size());

    for (auto& g : geos) {
        const auto roundEffect = sksg::RoundEffect::Make(std::move(g));
        rounded.push_back(roundEffect);

        abuilder->bindProperty<ScalarValue>(jtrim["r"], ascope,
            [roundEffect](const ScalarValue& r) {
                roundEffect->setRadius(r);
            });
    }

    return rounded;
}

std::vector<sk_sp<sksg::RenderNode>> AttachRepeaterDrawEffect(
        const skjson::ObjectValue& jrepeater,
        const AnimationBuilder* abuilder,
        AnimatorScope* ascope,
        std::vector<sk_sp<sksg::RenderNode>>&& draws) {

    std::vector<sk_sp<sksg::RenderNode>> repeater_draws;

    if (const skjson::ObjectValue* jtransform = jrepeater["tr"]) {
        sk_sp<sksg::RenderNode> repeater_node;
        if (draws.size() > 1) {
            repeater_node = sksg::Group::Make(std::move(draws));
        } else {
            repeater_node = std::move(draws[0]);
        }

        const auto repeater_composite = (ParseDefault(jrepeater["m"], 1) == 1)
                ? RepeaterAdapter::Composite::kAbove
                : RepeaterAdapter::Composite::kBelow;

        auto adapter = sk_make_sp<RepeaterAdapter>(std::move(repeater_node),
                                                   repeater_composite);

        abuilder->bindProperty<ScalarValue>(jrepeater["c"], ascope,
            [adapter](const ScalarValue& c) {
                adapter->setCount(c);
            });
        abuilder->bindProperty<ScalarValue>(jrepeater["o"], ascope,
            [adapter](const ScalarValue& o) {
                adapter->setOffset(o);
            });
        abuilder->bindProperty<VectorValue>((*jtransform)["a"], ascope,
            [adapter](const VectorValue& a) {
                adapter->setAnchorPoint(ValueTraits<VectorValue>::As<SkPoint>(a));
            });
        abuilder->bindProperty<VectorValue>((*jtransform)["p"], ascope,
            [adapter](const VectorValue& p) {
                adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
            });
        abuilder->bindProperty<VectorValue>((*jtransform)["s"], ascope,
            [adapter](const VectorValue& s) {
                adapter->setScale(ValueTraits<VectorValue>::As<SkVector>(s));
            });
        abuilder->bindProperty<ScalarValue>((*jtransform)["r"], ascope,
            [adapter](const ScalarValue& r) {
                adapter->setRotation(r);
            });
        abuilder->bindProperty<ScalarValue>((*jtransform)["so"], ascope,
            [adapter](const ScalarValue& so) {
                adapter->setStartOpacity(so);
            });
        abuilder->bindProperty<ScalarValue>((*jtransform)["eo"], ascope,
            [adapter](const ScalarValue& eo) {
                adapter->setEndOpacity(eo);
            });

        repeater_draws.reserve(1);
        repeater_draws.push_back(adapter->root());
    } else {
        repeater_draws = std::move(draws);
    }

    return repeater_draws;
}

using GeometryAttacherT = sk_sp<sksg::GeometryNode> (*)(const skjson::ObjectValue&,
                                                        const AnimationBuilder*, AnimatorScope*);
static constexpr GeometryAttacherT gGeometryAttachers[] = {
    AttachPathGeometry,
    AttachRRectGeometry,
    AttachEllipseGeometry,
    AttachPolystarGeometry,
};

using PaintAttacherT = sk_sp<sksg::PaintNode> (*)(const skjson::ObjectValue&,
                                                  const AnimationBuilder*, AnimatorScope*);
static constexpr PaintAttacherT gPaintAttachers[] = {
    AttachColorFill,
    AttachColorStroke,
    AttachGradientFill,
    AttachGradientStroke,
};

using GeometryEffectAttacherT =
    std::vector<sk_sp<sksg::GeometryNode>> (*)(const skjson::ObjectValue&,
                                               const AnimationBuilder*, AnimatorScope*,
                                               std::vector<sk_sp<sksg::GeometryNode>>&&);
static constexpr GeometryEffectAttacherT gGeometryEffectAttachers[] = {
    AttachMergeGeometryEffect,
    AttachTrimGeometryEffect,
    AttachRoundGeometryEffect,
};

using DrawEffectAttacherT =
    std::vector<sk_sp<sksg::RenderNode>> (*)(const skjson::ObjectValue&,
                                             const AnimationBuilder*, AnimatorScope*,
                                             std::vector<sk_sp<sksg::RenderNode>>&&);

static constexpr DrawEffectAttacherT gDrawEffectAttachers[] = {
    AttachRepeaterDrawEffect,
};

enum class ShapeType {
    kGeometry,
    kGeometryEffect,
    kPaint,
    kGroup,
    kTransform,
    kDrawEffect,
};

struct ShapeInfo {
    const char* fTypeString;
    ShapeType   fShapeType;
    uint32_t    fAttacherIndex; // index into respective attacher tables
};

const ShapeInfo* FindShapeInfo(const skjson::ObjectValue& jshape) {
    static constexpr ShapeInfo gShapeInfo[] = {
        { "el", ShapeType::kGeometry      , 2 }, // ellipse   -> AttachEllipseGeometry
        { "fl", ShapeType::kPaint         , 0 }, // fill      -> AttachColorFill
        { "gf", ShapeType::kPaint         , 2 }, // gfill     -> AttachGradientFill
        { "gr", ShapeType::kGroup         , 0 }, // group     -> Inline handler
        { "gs", ShapeType::kPaint         , 3 }, // gstroke   -> AttachGradientStroke
        { "mm", ShapeType::kGeometryEffect, 0 }, // merge     -> AttachMergeGeometryEffect
        { "rc", ShapeType::kGeometry      , 1 }, // rrect     -> AttachRRectGeometry
        { "rd", ShapeType::kGeometryEffect, 2 }, // round     -> AttachRoundGeometryEffect
        { "rp", ShapeType::kDrawEffect    , 0 }, // repeater  -> AttachRepeaterDrawEffect
        { "sh", ShapeType::kGeometry      , 0 }, // shape     -> AttachPathGeometry
        { "sr", ShapeType::kGeometry      , 3 }, // polystar  -> AttachPolyStarGeometry
        { "st", ShapeType::kPaint         , 1 }, // stroke    -> AttachColorStroke
        { "tm", ShapeType::kGeometryEffect, 1 }, // trim      -> AttachTrimGeometryEffect
        { "tr", ShapeType::kTransform     , 0 }, // transform -> Inline handler
    };

    const skjson::StringValue* type = jshape["ty"];
    if (!type) {
        return nullptr;
    }

    const auto* info = bsearch(type->begin(),
                               gShapeInfo,
                               SK_ARRAY_COUNT(gShapeInfo),
                               sizeof(ShapeInfo),
                               [](const void* key, const void* info) {
                                  return strcmp(static_cast<const char*>(key),
                                                static_cast<const ShapeInfo*>(info)->fTypeString);
                               });

    return static_cast<const ShapeInfo*>(info);
}

struct GeometryEffectRec {
    const skjson::ObjectValue& fJson;
    GeometryEffectAttacherT    fAttach;
};

} // namespace

struct AnimationBuilder::AttachShapeContext {
    AttachShapeContext(AnimatorScope* ascope,
                       std::vector<sk_sp<sksg::GeometryNode>>* geos,
                       std::vector<GeometryEffectRec>* effects,
                       size_t committedAnimators)
        : fScope(ascope)
        , fGeometryStack(geos)
        , fGeometryEffectStack(effects)
        , fCommittedAnimators(committedAnimators) {}

    AnimatorScope*                          fScope;
    std::vector<sk_sp<sksg::GeometryNode>>* fGeometryStack;
    std::vector<GeometryEffectRec>*         fGeometryEffectStack;
    size_t                                  fCommittedAnimators;
};

sk_sp<sksg::RenderNode> AnimationBuilder::attachShape(const skjson::ArrayValue* jshape,
                                                      AttachShapeContext* ctx) const {
    if (!jshape)
        return nullptr;

    SkDEBUGCODE(const auto initialGeometryEffects = ctx->fGeometryEffectStack->size();)

    const skjson::ObjectValue* jtransform = nullptr;

    struct ShapeRec {
        const skjson::ObjectValue& fJson;
        const ShapeInfo&           fInfo;
    };

    // First pass (bottom->top):
    //
    //   * pick up the group transform and opacity
    //   * push local geometry effects onto the stack
    //   * store recs for next pass
    //
    std::vector<ShapeRec> recs;
    for (size_t i = 0; i < jshape->size(); ++i) {
        const skjson::ObjectValue* shape = (*jshape)[jshape->size() - 1 - i];
        if (!shape) continue;

        const auto* info = FindShapeInfo(*shape);
        if (!info) {
            this->log(Logger::Level::kError, &(*shape)["ty"], "Unknown shape.");
            continue;
        }

        if (ParseDefault<bool>((*shape)["hd"], false)) {
            // Ignore hidden shapes.
            continue;
        }

        recs.push_back({ *shape, *info });

        switch (info->fShapeType) {
        case ShapeType::kTransform:
            // Just track the transform property for now -- we'll deal with it later.
            jtransform = shape;
            break;
        case ShapeType::kGeometryEffect:
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            ctx->fGeometryEffectStack->push_back(
                { *shape, gGeometryEffectAttachers[info->fAttacherIndex] });
            break;
        default:
            break;
        }
    }

    // Second pass (top -> bottom, after 2x reverse):
    //
    //   * track local geometry
    //   * emit local paints
    //
    std::vector<sk_sp<sksg::GeometryNode>> geos;
    std::vector<sk_sp<sksg::RenderNode  >> draws;

    const auto add_draw = [this, &draws](sk_sp<sksg::RenderNode> draw, const ShapeRec& rec) {
        // All draws can have an optional blend mode.
        draws.push_back(this->attachBlendMode(rec.fJson, std::move(draw)));
    };

    for (auto rec = recs.rbegin(); rec != recs.rend(); ++rec) {
        const AutoPropertyTracker apt(this, rec->fJson);

        switch (rec->fInfo.fShapeType) {
        case ShapeType::kGeometry: {
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gGeometryAttachers));
            if (auto geo = gGeometryAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                         this,
                                                                         ctx->fScope)) {
                geos.push_back(std::move(geo));
            }
        } break;
        case ShapeType::kGeometryEffect: {
            // Apply the current effect and pop from the stack.
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            if (!geos.empty()) {
                geos = gGeometryEffectAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                           this,
                                                                           ctx->fScope,
                                                                           std::move(geos));
            }

            SkASSERT(&ctx->fGeometryEffectStack->back().fJson == &rec->fJson);
            SkASSERT(ctx->fGeometryEffectStack->back().fAttach ==
                     gGeometryEffectAttachers[rec->fInfo.fAttacherIndex]);
            ctx->fGeometryEffectStack->pop_back();
        } break;
        case ShapeType::kGroup: {
            AttachShapeContext groupShapeCtx(ctx->fScope,
                                             &geos,
                                             ctx->fGeometryEffectStack,
                                             ctx->fCommittedAnimators);
            if (auto subgroup = this->attachShape(rec->fJson["it"], &groupShapeCtx)) {
                add_draw(std::move(subgroup), *rec);
                SkASSERT(groupShapeCtx.fCommittedAnimators >= ctx->fCommittedAnimators);
                ctx->fCommittedAnimators = groupShapeCtx.fCommittedAnimators;
            }
        } break;
        case ShapeType::kPaint: {
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gPaintAttachers));
            auto paint = gPaintAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                    this,
                                                                    ctx->fScope);
            if (!paint || geos.empty())
                break;

            auto drawGeos = geos;

            // Apply all pending effects from the stack.
            for (auto it = ctx->fGeometryEffectStack->rbegin();
                 it != ctx->fGeometryEffectStack->rend(); ++it) {
                drawGeos = it->fAttach(it->fJson, this, ctx->fScope, std::move(drawGeos));
            }

            // If we still have multiple geos, reduce using 'merge'.
            auto geo = drawGeos.size() > 1
                ? Merge(std::move(drawGeos), sksg::Merge::Mode::kMerge)
                : drawGeos[0];

            SkASSERT(geo);
            add_draw(sksg::Draw::Make(std::move(geo), std::move(paint)), *rec);
            ctx->fCommittedAnimators = ctx->fScope->size();
        } break;
        case ShapeType::kDrawEffect: {
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gDrawEffectAttachers));
            if (!draws.empty()) {
                draws = gDrawEffectAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                        this,
                                                                        ctx->fScope,
                                                                        std::move(draws));
                ctx->fCommittedAnimators = ctx->fScope->size();
            }
        } break;
        default:
            break;
        }
    }

    // By now we should have popped all local geometry effects.
    SkASSERT(ctx->fGeometryEffectStack->size() == initialGeometryEffects);

    sk_sp<sksg::RenderNode> shape_wrapper;
    if (draws.size() == 1) {
        // For a single draw, we don't need a group.
        shape_wrapper = std::move(draws.front());
    } else if (!draws.empty()) {
        // Emit local draws reversed (bottom->top, per spec).
        std::reverse(draws.begin(), draws.end());
        draws.shrink_to_fit();

        // We need a group to dispatch multiple draws.
        shape_wrapper = sksg::Group::Make(std::move(draws));
    }

    sk_sp<sksg::Transform> shape_transform;
    if (jtransform) {
        const AutoPropertyTracker apt(this, *jtransform);

        // This is tricky due to the interaction with ctx->fCommittedAnimators: we want any
        // animators related to tranform/opacity to be committed => they must be inserted in front
        // of the dangling/uncommitted ones.
        AnimatorScope local_scope;

        if ((shape_transform = this->attachMatrix2D(*jtransform, &local_scope, nullptr))) {
            shape_wrapper = sksg::TransformEffect::Make(std::move(shape_wrapper), shape_transform);
        }
        shape_wrapper = this->attachOpacity(*jtransform, &local_scope, std::move(shape_wrapper));

        ctx->fScope->insert(ctx->fScope->begin() + ctx->fCommittedAnimators,
                            std::make_move_iterator(local_scope.begin()),
                            std::make_move_iterator(local_scope.end()));
        ctx->fCommittedAnimators += local_scope.size();
    }

    // Push transformed local geometries to parent list, for subsequent paints.
    for (auto& geo : geos) {
        ctx->fGeometryStack->push_back(shape_transform
            ? sksg::GeometryTransform::Make(std::move(geo), shape_transform)
            : std::move(geo));
    }

    return shape_wrapper;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachShapeLayer(const skjson::ObjectValue& layer,
                                                           const LayerInfo&,
                                                           AnimatorScope* ascope) const {
    std::vector<sk_sp<sksg::GeometryNode>> geometryStack;
    std::vector<GeometryEffectRec> geometryEffectStack;
    AttachShapeContext shapeCtx(ascope, &geometryStack, &geometryEffectStack, ascope->size());
    auto shapeNode = this->attachShape(layer["shapes"], &shapeCtx);

    // Trim uncommitted animators: AttachShape consumes effects on the fly, and greedily attaches
    // geometries => at the end, we can end up with unused geometries, which are nevertheless alive
    // due to attached animators.  To avoid this, we track committed animators and discard the
    // orphans here.
    SkASSERT(shapeCtx.fCommittedAnimators <= ascope->size());
    ascope->resize(shapeCtx.fCommittedAnimators);

    return shapeNode;
}

} // namespace internal
} // namespace skottie
