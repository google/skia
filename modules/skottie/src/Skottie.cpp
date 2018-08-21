/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Skottie.h"

#include "SkCanvas.h"
#include "SkData.h"
#include "SkFontMgr.h"
#include "SkImage.h"
#include "SkMakeUnique.h"
#include "SkOSPath.h"
#include "SkPaint.h"
#include "SkParse.h"
#include "SkPoint.h"
#include "SkSGClipEffect.h"
#include "SkSGColor.h"
#include "SkSGColorFilter.h"
#include "SkSGDraw.h"
#include "SkSGGeometryTransform.h"
#include "SkSGGradient.h"
#include "SkSGGroup.h"
#include "SkSGImage.h"
#include "SkSGInvalidationController.h"
#include "SkSGMaskEffect.h"
#include "SkSGMerge.h"
#include "SkSGOpacityEffect.h"
#include "SkSGPath.h"
#include "SkSGRect.h"
#include "SkSGRoundEffect.h"
#include "SkSGScene.h"
#include "SkSGTransform.h"
#include "SkSGTrimEffect.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTHash.h"
#include "SkTLazy.h"
#include "SkTime.h"
#include "SkTo.h"
#include "SkottieAdapter.h"
#include "SkottieAnimator.h"
#include "SkottieJson.h"
#include "SkottiePriv.h"
#include "SkottieValue.h"

#include <cmath>
#include <vector>

#include "stdlib.h"

namespace skottie {

namespace internal {

void LogJSON(const skjson::Value& json, const char msg[]) {
    const auto dump = json.toString();
    LOG("%s: %s\n", msg, dump.c_str());
}

namespace {

// DEPRECATED: replace w/ LogJSON.
bool LogFail(const skjson::Value& json, const char* msg) {
    const auto dump = json.toString();
    LOG("!! %s: %s\n", msg, dump.c_str());
    return false;
}

sk_sp<sksg::Matrix> AttachMatrix(const skjson::ObjectValue& t, AnimatorScope* ascope,
                                 sk_sp<sksg::Matrix> parentMatrix) {
    static const VectorValue g_default_vec_0   = {  0,   0},
                             g_default_vec_100 = {100, 100};

    auto matrix = sksg::Matrix::Make(SkMatrix::I(), parentMatrix);
    auto adapter = sk_make_sp<TransformAdapter>(matrix);

    auto bound = BindProperty<VectorValue>(t["a"], ascope,
            [adapter](const VectorValue& a) {
                adapter->setAnchorPoint(ValueTraits<VectorValue>::As<SkPoint>(a));
            }, g_default_vec_0);
    bound |= BindProperty<VectorValue>(t["p"], ascope,
            [adapter](const VectorValue& p) {
                adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
            }, g_default_vec_0);
    bound |= BindProperty<VectorValue>(t["s"], ascope,
            [adapter](const VectorValue& s) {
                adapter->setScale(ValueTraits<VectorValue>::As<SkVector>(s));
            }, g_default_vec_100);

    const auto* jrotation = &t["r"];
    if (jrotation->is<skjson::NullValue>()) {
        // 3d rotations have separate rx,ry,rz components.  While we don't fully support them,
        // we can still make use of rz.
        jrotation = &t["rz"];
    }
    bound |= BindProperty<ScalarValue>(*jrotation, ascope,
            [adapter](const ScalarValue& r) {
                adapter->setRotation(r);
            }, 0.0f);
    bound |= BindProperty<ScalarValue>(t["sk"], ascope,
            [adapter](const ScalarValue& sk) {
                adapter->setSkew(sk);
            }, 0.0f);
    bound |= BindProperty<ScalarValue>(t["sa"], ascope,
            [adapter](const ScalarValue& sa) {
                adapter->setSkewAxis(sa);
            }, 0.0f);

    return bound ? matrix : parentMatrix;
}

sk_sp<sksg::RenderNode> AttachOpacity(const skjson::ObjectValue& jtransform, AnimatorScope* ascope,
                                      sk_sp<sksg::RenderNode> childNode) {
    if (!childNode)
        return nullptr;

    auto opacityNode = sksg::OpacityEffect::Make(childNode);

    if (!BindProperty<ScalarValue>(jtransform["o"], ascope,
        [opacityNode](const ScalarValue& o) {
            // BM opacity is [0..100]
            opacityNode->setOpacity(o * 0.01f);
        }, 100.0f)) {
        // We can ignore static full opacity.
        return childNode;
    }

    return std::move(opacityNode);
}

sk_sp<sksg::Path> AttachPath(const skjson::Value& jpath, AnimatorScope* ascope) {
    auto path_node = sksg::Path::Make();
    return BindProperty<ShapeValue>(jpath, ascope,
        [path_node](const ShapeValue& p) {
            // FillType is tracked in the SG node, not in keyframes -- make sure we preserve it.
            auto path = ValueTraits<ShapeValue>::As<SkPath>(p);
            path.setFillType(path_node->getFillType());
            path_node->setPath(path);
        })
        ? path_node
        : nullptr;
}

sk_sp<sksg::GeometryNode> AttachPathGeometry(const skjson::ObjectValue& jpath,
                                             AnimatorScope* ascope) {
    return AttachPath(jpath["ks"], ascope);
}

sk_sp<sksg::GeometryNode> AttachRRectGeometry(const skjson::ObjectValue& jrect,
                                              AnimatorScope* ascope) {
    auto rect_node = sksg::RRect::Make();
    auto adapter = sk_make_sp<RRectAdapter>(rect_node);

    auto p_attached = BindProperty<VectorValue>(jrect["p"], ascope,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    auto s_attached = BindProperty<VectorValue>(jrect["s"], ascope,
        [adapter](const VectorValue& s) {
            adapter->setSize(ValueTraits<VectorValue>::As<SkSize>(s));
        });
    auto r_attached = BindProperty<ScalarValue>(jrect["r"], ascope,
        [adapter](const ScalarValue& r) {
            adapter->setRadius(SkSize::Make(r, r));
        });

    if (!p_attached && !s_attached && !r_attached) {
        return nullptr;
    }

    return std::move(rect_node);
}

sk_sp<sksg::GeometryNode> AttachEllipseGeometry(const skjson::ObjectValue& jellipse,
                                                AnimatorScope* ascope) {
    auto rect_node = sksg::RRect::Make();
    auto adapter = sk_make_sp<RRectAdapter>(rect_node);

    auto p_attached = BindProperty<VectorValue>(jellipse["p"], ascope,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    auto s_attached = BindProperty<VectorValue>(jellipse["s"], ascope,
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
                                                 AnimatorScope* ascope) {
    static constexpr PolyStarAdapter::Type gTypes[] = {
        PolyStarAdapter::Type::kStar, // "sy": 1
        PolyStarAdapter::Type::kPoly, // "sy": 2
    };

    const auto type = ParseDefault<size_t>(jstar["sy"], 0) - 1;
    if (type >= SK_ARRAY_COUNT(gTypes)) {
        LogFail(jstar, "Unknown polystar type");
        return nullptr;
    }

    auto path_node = sksg::Path::Make();
    auto adapter = sk_make_sp<PolyStarAdapter>(path_node, gTypes[type]);

    BindProperty<VectorValue>(jstar["p"], ascope,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    BindProperty<ScalarValue>(jstar["pt"], ascope,
        [adapter](const ScalarValue& pt) {
            adapter->setPointCount(pt);
        });
    BindProperty<ScalarValue>(jstar["ir"], ascope,
        [adapter](const ScalarValue& ir) {
            adapter->setInnerRadius(ir);
        });
    BindProperty<ScalarValue>(jstar["or"], ascope,
        [adapter](const ScalarValue& otr) {
            adapter->setOuterRadius(otr);
        });
    BindProperty<ScalarValue>(jstar["is"], ascope,
        [adapter](const ScalarValue& is) {
            adapter->setInnerRoundness(is);
        });
    BindProperty<ScalarValue>(jstar["os"], ascope,
        [adapter](const ScalarValue& os) {
            adapter->setOuterRoundness(os);
        });
    BindProperty<ScalarValue>(jstar["r"], ascope,
        [adapter](const ScalarValue& r) {
            adapter->setRotation(r);
        });

    return std::move(path_node);
}

sk_sp<sksg::Color> AttachColor(const skjson::ObjectValue& jcolor, AnimatorScope* ascope,
                               const char prop_name[]) {
    auto color_node = sksg::Color::Make(SK_ColorBLACK);
    BindProperty<VectorValue>(jcolor[prop_name], ascope,
        [color_node](const VectorValue& c) {
            color_node->setColor(ValueTraits<VectorValue>::As<SkColor>(c));
        });

    return color_node;
}

sk_sp<sksg::Gradient> AttachGradient(const skjson::ObjectValue& jgrad, AnimatorScope* ascope) {
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

    BindProperty<VectorValue>((*stops)["k"], ascope,
        [adapter](const VectorValue& stops) {
            adapter->setColorStops(stops);
        });
    BindProperty<VectorValue>(jgrad["s"], ascope,
        [adapter](const VectorValue& s) {
            adapter->setStartPoint(ValueTraits<VectorValue>::As<SkPoint>(s));
        });
    BindProperty<VectorValue>(jgrad["e"], ascope,
        [adapter](const VectorValue& e) {
            adapter->setEndPoint(ValueTraits<VectorValue>::As<SkPoint>(e));
        });

    return gradient_node;
}

sk_sp<sksg::PaintNode> AttachPaint(const skjson::ObjectValue& jpaint, AnimatorScope* ascope,
                                   sk_sp<sksg::PaintNode> paint_node) {
    if (paint_node) {
        paint_node->setAntiAlias(true);

        BindProperty<ScalarValue>(jpaint["o"], ascope,
            [paint_node](const ScalarValue& o) {
                // BM opacity is [0..100]
                paint_node->setOpacity(o * 0.01f);
        });
    }

    return paint_node;
}

sk_sp<sksg::PaintNode> AttachStroke(const skjson::ObjectValue& jstroke, AnimatorScope* ascope,
                                    sk_sp<sksg::PaintNode> stroke_node) {
    if (!stroke_node)
        return nullptr;

    stroke_node->setStyle(SkPaint::kStroke_Style);

    BindProperty<ScalarValue>(jstroke["w"], ascope,
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

sk_sp<sksg::PaintNode> AttachColorFill(const skjson::ObjectValue& jfill, AnimatorScope* ascope) {
    return AttachPaint(jfill, ascope, AttachColor(jfill, ascope, "c"));
}

sk_sp<sksg::PaintNode> AttachGradientFill(const skjson::ObjectValue& jfill, AnimatorScope* ascope) {
    return AttachPaint(jfill, ascope, AttachGradient(jfill, ascope));
}

sk_sp<sksg::PaintNode> AttachColorStroke(const skjson::ObjectValue& jstroke,
                                         AnimatorScope* ascope) {
    return AttachStroke(jstroke, ascope, AttachPaint(jstroke, ascope,
                                                     AttachColor(jstroke, ascope, "c")));
}

sk_sp<sksg::PaintNode> AttachGradientStroke(const skjson::ObjectValue& jstroke,
                                            AnimatorScope* ascope) {
    return AttachStroke(jstroke, ascope, AttachPaint(jstroke, ascope,
                                                     AttachGradient(jstroke, ascope)));
}

sk_sp<sksg::Merge> Merge(std::vector<sk_sp<sksg::GeometryNode>>&& geos, sksg::Merge::Mode mode) {
    std::vector<sksg::Merge::Rec> merge_recs;
    merge_recs.reserve(geos.size());

    for (const auto& geo : geos) {
        merge_recs.push_back(
            {std::move(geo), merge_recs.empty() ? sksg::Merge::Mode::kMerge : mode});
    }

    return sksg::Merge::Make(std::move(merge_recs));
}

std::vector<sk_sp<sksg::GeometryNode>> AttachMergeGeometryEffect(
        const skjson::ObjectValue& jmerge, AnimatorScope*,
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
        const skjson::ObjectValue& jtrim, AnimatorScope* ascope,
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
        const auto trimEffect = sksg::TrimEffect::Make(i);
        trimmed.push_back(trimEffect);

        const auto adapter = sk_make_sp<TrimEffectAdapter>(std::move(trimEffect));
        BindProperty<ScalarValue>(jtrim["s"], ascope,
            [adapter](const ScalarValue& s) {
                adapter->setStart(s);
            });
        BindProperty<ScalarValue>(jtrim["e"], ascope,
            [adapter](const ScalarValue& e) {
                adapter->setEnd(e);
            });
        BindProperty<ScalarValue>(jtrim["o"], ascope,
            [adapter](const ScalarValue& o) {
                adapter->setOffset(o);
            });
    }

    return trimmed;
}

std::vector<sk_sp<sksg::GeometryNode>> AttachRoundGeometryEffect(
        const skjson::ObjectValue& jtrim, AnimatorScope* ascope,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {

    std::vector<sk_sp<sksg::GeometryNode>> rounded;
    rounded.reserve(geos.size());

    for (const auto& g : geos) {
        const auto roundEffect = sksg::RoundEffect::Make(std::move(g));
        rounded.push_back(roundEffect);

        BindProperty<ScalarValue>(jtrim["r"], ascope,
            [roundEffect](const ScalarValue& r) {
                roundEffect->setRadius(r);
            });
    }

    return rounded;
}

using GeometryAttacherT = sk_sp<sksg::GeometryNode> (*)(const skjson::ObjectValue&, AnimatorScope*);
static constexpr GeometryAttacherT gGeometryAttachers[] = {
    AttachPathGeometry,
    AttachRRectGeometry,
    AttachEllipseGeometry,
    AttachPolystarGeometry,
};

using PaintAttacherT = sk_sp<sksg::PaintNode> (*)(const skjson::ObjectValue&, AnimatorScope*);
static constexpr PaintAttacherT gPaintAttachers[] = {
    AttachColorFill,
    AttachColorStroke,
    AttachGradientFill,
    AttachGradientStroke,
};

using GeometryEffectAttacherT =
    std::vector<sk_sp<sksg::GeometryNode>> (*)(const skjson::ObjectValue&,
                                               AnimatorScope*,
                                               std::vector<sk_sp<sksg::GeometryNode>>&&);
static constexpr GeometryEffectAttacherT gGeometryEffectAttachers[] = {
    AttachMergeGeometryEffect,
    AttachTrimGeometryEffect,
    AttachRoundGeometryEffect,
};

enum class ShapeType {
    kGeometry,
    kGeometryEffect,
    kPaint,
    kGroup,
    kTransform,
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

struct AttachShapeContext {
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

sk_sp<sksg::RenderNode> AttachShape(const skjson::ArrayValue* jshape,
                                    AttachShapeContext* shapeCtx) {
    if (!jshape)
        return nullptr;

    SkDEBUGCODE(const auto initialGeometryEffects = shapeCtx->fGeometryEffectStack->size();)

    sk_sp<sksg::Group> shape_group = sksg::Group::Make();
    sk_sp<sksg::RenderNode> shape_wrapper = shape_group;
    sk_sp<sksg::Matrix> shape_matrix;

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
            LogFail((*shape)["ty"], "Unknown shape");
            continue;
        }

        recs.push_back({ *shape, *info });

        switch (info->fShapeType) {
        case ShapeType::kTransform:
            if ((shape_matrix = AttachMatrix(*shape, shapeCtx->fScope, nullptr))) {
                shape_wrapper = sksg::Transform::Make(std::move(shape_wrapper), shape_matrix);
            }
            shape_wrapper = AttachOpacity(*shape, shapeCtx->fScope, std::move(shape_wrapper));
            break;
        case ShapeType::kGeometryEffect:
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            shapeCtx->fGeometryEffectStack->push_back(
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
    for (auto rec = recs.rbegin(); rec != recs.rend(); ++rec) {
        switch (rec->fInfo.fShapeType) {
        case ShapeType::kGeometry: {
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gGeometryAttachers));
            if (auto geo = gGeometryAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                         shapeCtx->fScope)) {
                geos.push_back(std::move(geo));
            }
        } break;
        case ShapeType::kGeometryEffect: {
            // Apply the current effect and pop from the stack.
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            if (!geos.empty()) {
                geos = gGeometryEffectAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                           shapeCtx->fScope,
                                                                           std::move(geos));
            }

            SkASSERT(&shapeCtx->fGeometryEffectStack->back().fJson == &rec->fJson);
            SkASSERT(shapeCtx->fGeometryEffectStack->back().fAttach ==
                     gGeometryEffectAttachers[rec->fInfo.fAttacherIndex]);
            shapeCtx->fGeometryEffectStack->pop_back();
        } break;
        case ShapeType::kGroup: {
            AttachShapeContext groupShapeCtx(shapeCtx->fScope,
                                             &geos,
                                             shapeCtx->fGeometryEffectStack,
                                             shapeCtx->fCommittedAnimators);
            if (auto subgroup = AttachShape(rec->fJson["it"], &groupShapeCtx)) {
                draws.push_back(std::move(subgroup));
                SkASSERT(groupShapeCtx.fCommittedAnimators >= shapeCtx->fCommittedAnimators);
                shapeCtx->fCommittedAnimators = groupShapeCtx.fCommittedAnimators;
            }
        } break;
        case ShapeType::kPaint: {
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gPaintAttachers));
            auto paint = gPaintAttachers[rec->fInfo.fAttacherIndex](rec->fJson, shapeCtx->fScope);
            if (!paint || geos.empty())
                break;

            auto drawGeos = geos;

            // Apply all pending effects from the stack.
            for (auto it = shapeCtx->fGeometryEffectStack->rbegin();
                 it != shapeCtx->fGeometryEffectStack->rend(); ++it) {
                drawGeos = it->fAttach(it->fJson, shapeCtx->fScope, std::move(drawGeos));
            }

            // If we still have multiple geos, reduce using 'merge'.
            auto geo = drawGeos.size() > 1
                ? Merge(std::move(drawGeos), sksg::Merge::Mode::kMerge)
                : drawGeos[0];

            SkASSERT(geo);
            draws.push_back(sksg::Draw::Make(std::move(geo), std::move(paint)));
            shapeCtx->fCommittedAnimators = shapeCtx->fScope->size();
        } break;
        default:
            break;
        }
    }

    // By now we should have popped all local geometry effects.
    SkASSERT(shapeCtx->fGeometryEffectStack->size() == initialGeometryEffects);

    // Push transformed local geometries to parent list, for subsequent paints.
    for (const auto& geo : geos) {
        shapeCtx->fGeometryStack->push_back(shape_matrix
            ? sksg::GeometryTransform::Make(std::move(geo), shape_matrix)
            : std::move(geo));
    }

    // Emit local draws reversed (bottom->top, per spec).
    for (auto it = draws.rbegin(); it != draws.rend(); ++it) {
        shape_group->addChild(std::move(*it));
    }

    return draws.empty() ? nullptr : shape_wrapper;
}

} // namespace

sk_sp<sksg::RenderNode> AnimationBuilder::attachNestedAnimation(const char* name,
                                                                AnimatorScope* ascope) {
    class SkottieSGAdapter final : public sksg::RenderNode {
    public:
        explicit SkottieSGAdapter(sk_sp<Animation> animation)
            : fAnimation(std::move(animation)) {
            SkASSERT(fAnimation);
        }

    protected:
        SkRect onRevalidate(sksg::InvalidationController*, const SkMatrix&) override {
            return SkRect::MakeSize(fAnimation->size());
        }

        void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
            const auto local_scope =
                ScopedRenderContext(canvas, ctx).setIsolation(this->bounds(), true);
            fAnimation->render(canvas);
        }

    private:
        const sk_sp<Animation> fAnimation;
    };

    class SkottieAnimatorAdapter final : public sksg::Animator {
    public:
        SkottieAnimatorAdapter(sk_sp<Animation> animation, float time_scale)
            : fAnimation(std::move(animation))
            , fTimeScale(time_scale) {
            SkASSERT(fAnimation);
        }

    protected:
        void onTick(float t) {
            // TODO: we prolly need more sophisticated timeline mapping for nested animations.
            fAnimation->seek(t * fTimeScale);
        }

    private:
        const sk_sp<Animation> fAnimation;
        const float            fTimeScale;
    };

    const auto data = fResourceProvider.load("", name);
    if (!data) {
        LOG("!! Could not load: %s\n", name);
        return nullptr;
    }

    auto animation = Animation::Make(static_cast<const char*>(data->data()), data->size(),
                                     &fResourceProvider);
    if (!animation) {
        LOG("!! Could not parse nested animation: %s\n", name);
        return nullptr;
    }


    ascope->push_back(
        skstd::make_unique<SkottieAnimatorAdapter>(animation, animation->duration() / fDuration));

    return sk_make_sp<SkottieSGAdapter>(std::move(animation));
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachAssetRef(
    const skjson::ObjectValue& jlayer, AnimatorScope* ascope,
    sk_sp<sksg::RenderNode>(AnimationBuilder::* attach_proc)(const skjson::ObjectValue& comp,
                                                             AnimatorScope* ascope)) {

    const auto refId = ParseDefault<SkString>(jlayer["refId"], SkString());
    if (refId.isEmpty()) {
        LOG("!! Layer missing refId\n");
        return nullptr;
    }

    if (refId.startsWith("$")) {
        return this->attachNestedAnimation(refId.c_str() + 1, ascope);
    }

    const auto* asset_info = fAssets.find(refId);
    if (!asset_info) {
        LOG("!! Asset not found: '%s'\n", refId.c_str());
        return nullptr;
    }

    if (asset_info->fIsAttaching) {
        LOG("!! Asset cycle detected for: '%s'\n", refId.c_str());
        return nullptr;
    }

    asset_info->fIsAttaching = true;
    auto asset = (this->*attach_proc)(*asset_info->fAsset, ascope);
    asset_info->fIsAttaching = false;

    return asset;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachPrecompLayer(const skjson::ObjectValue& jlayer,
                                                             AnimatorScope* ascope) {
    const skjson::ObjectValue* time_remap = jlayer["tm"];
    const auto start_time = ParseDefault<float>(jlayer["st"], 0.0f),
             stretch_time = ParseDefault<float>(jlayer["sr"], 1.0f);
    const auto requires_time_mapping = !SkScalarNearlyEqual(start_time  , 0) ||
                                       !SkScalarNearlyEqual(stretch_time, 1) ||
                                       time_remap;

    AnimatorScope local_animators;
    auto comp_layer = this->attachAssetRef(jlayer,
                                           requires_time_mapping ? &local_animators : ascope,
                                           &AnimationBuilder::attachComposition);

    // Applies a bias/scale/remap t-adjustment to child animators.
    class CompTimeMapper final : public sksg::GroupAnimator {
    public:
        CompTimeMapper(sksg::AnimatorList&& layer_animators, float time_bias, float time_scale)
            : INHERITED(std::move(layer_animators))
            , fTimeBias(time_bias)
            , fTimeScale(time_scale) {}

        void onTick(float t) override {
            // When time remapping is active, |t| is driven externally.
            if (fRemappedTime.isValid()) {
                t = *fRemappedTime.get();
            }

            this->INHERITED::onTick((t + fTimeBias) * fTimeScale);
        }

        void remapTime(float t) { fRemappedTime.set(t); }

    private:
        const float    fTimeBias,
                       fTimeScale;
        SkTLazy<float> fRemappedTime;

        using INHERITED = sksg::GroupAnimator;
    };

    if (requires_time_mapping) {
        const auto t_bias  = -start_time,
                   t_scale = sk_ieee_float_divide(1, stretch_time);
        auto time_mapper = skstd::make_unique<CompTimeMapper>(std::move(local_animators),
                                                              t_bias, t_scale);
        if (time_remap) {
            // The lambda below captures a raw pointer to the mapper object.  That should be safe,
            // because both the lambda and the mapper are scoped/owned by ctx->fAnimators.
            auto* raw_mapper = time_mapper.get();
            auto  frame_rate = fFrameRate;
            BindProperty<ScalarValue>(*time_remap, ascope,
                    [raw_mapper, frame_rate](const ScalarValue& t) {
                        raw_mapper->remapTime(t * frame_rate);
                    });
        }
        ascope->push_back(std::move(time_mapper));
    }

    return comp_layer;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachSolidLayer(const skjson::ObjectValue& jlayer,
                                                           AnimatorScope*) {
    const auto size = SkSize::Make(ParseDefault<float>(jlayer["sw"], 0.0f),
                                   ParseDefault<float>(jlayer["sh"], 0.0f));
    const skjson::StringValue* hex_str = jlayer["sc"];
    uint32_t c;
    if (size.isEmpty() ||
        *hex_str->begin() != '#' ||
        !SkParse::FindHex(hex_str->begin() + 1, &c)) {
        LogFail(jlayer, "Could not parse solid layer");
        return nullptr;
    }

    const SkColor color = 0xff000000 | c;

    return sksg::Draw::Make(sksg::Rect::Make(SkRect::MakeSize(size)),
                            sksg::Color::Make(color));
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachImageAsset(const skjson::ObjectValue& jimage,
                                                           AnimatorScope*) {
    const skjson::StringValue* name = jimage["p"];
    const skjson::StringValue* path = jimage["u"];
    if (!name) {
        return nullptr;
    }

    const auto name_cstr = name->begin(),
               path_cstr = path ? path->begin() : "";
    const auto res_id = SkStringPrintf("%s|%s", path_cstr, name_cstr);
    if (auto* attached_image = fAssetCache.find(res_id)) {
        return *attached_image;
    }

    const auto data = fResourceProvider.load(path_cstr, name_cstr);
    if (!data) {
        LOG("!! Could not load image resource: %s/%s\n", path_cstr, name_cstr);
        return nullptr;
    }

    // TODO: non-intrisic image sizing
    return *fAssetCache.set(res_id, sksg::Image::Make(SkImage::MakeFromEncoded(data)));
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachImageLayer(const skjson::ObjectValue& jlayer,
                                                           AnimatorScope* ascope) {
    return this->attachAssetRef(jlayer, ascope, &AnimationBuilder::attachImageAsset);
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachNullLayer(const skjson::ObjectValue& layer,
                                                          AnimatorScope*) {
    // Null layers are used solely to drive dependent transforms,
    // but we use free-floating sksg::Matrices for that purpose.
    return nullptr;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachShapeLayer(const skjson::ObjectValue& layer,
                                                           AnimatorScope* ascope) {
    std::vector<sk_sp<sksg::GeometryNode>> geometryStack;
    std::vector<GeometryEffectRec> geometryEffectStack;
    AttachShapeContext shapeCtx(ascope, &geometryStack, &geometryEffectStack, ascope->size());
    auto shapeNode = AttachShape(layer["shapes"], &shapeCtx);

    // Trim uncommitted animators: AttachShape consumes effects on the fly, and greedily attaches
    // geometries => at the end, we can end up with unused geometries, which are nevertheless alive
    // due to attached animators.  To avoid this, we track committed animators and discard the
    // orphans here.
    SkASSERT(shapeCtx.fCommittedAnimators <= ascope->size());
    ascope->resize(shapeCtx.fCommittedAnimators);

    return shapeNode;
}

struct AnimationBuilder::AttachLayerContext {
    AttachLayerContext(const skjson::ArrayValue& jlayers, AnimatorScope* scope)
        : fLayerList(jlayers), fScope(scope) {}

    const skjson::ArrayValue&            fLayerList;
    AnimatorScope*                       fScope;
    SkTHashMap<int, sk_sp<sksg::Matrix>> fLayerMatrixMap;
    sk_sp<sksg::RenderNode>              fCurrentMatte;

    sk_sp<sksg::Matrix> AttachLayerMatrix(const skjson::ObjectValue& jlayer) {
        const auto layer_index = ParseDefault<int>(jlayer["ind"], -1);
        if (layer_index < 0)
            return nullptr;

        if (auto* m = fLayerMatrixMap.find(layer_index))
            return *m;

        return this->AttachLayerMatrixImpl(jlayer, layer_index);
    }

private:
    sk_sp<sksg::Matrix> AttachParentLayerMatrix(const skjson::ObjectValue& jlayer,
                                                int layer_index) {
        const auto parent_index = ParseDefault<int>(jlayer["parent"], -1);
        if (parent_index < 0 || parent_index == layer_index)
            return nullptr;

        if (auto* m = fLayerMatrixMap.find(parent_index))
            return *m;

        for (const skjson::ObjectValue* l : fLayerList) {
            if (!l) continue;

            if (ParseDefault<int>((*l)["ind"], -1) == parent_index) {
                return this->AttachLayerMatrixImpl(*l, parent_index);
            }
        }

        return nullptr;
    }

    sk_sp<sksg::Matrix> AttachLayerMatrixImpl(const skjson::ObjectValue& jlayer, int layer_index) {
        SkASSERT(!fLayerMatrixMap.find(layer_index));

        // Add a stub entry to break recursion cycles.
        fLayerMatrixMap.set(layer_index, nullptr);

        auto parent_matrix = this->AttachParentLayerMatrix(jlayer, layer_index);

        if (const skjson::ObjectValue* jtransform = jlayer["ks"]) {
            return *fLayerMatrixMap.set(layer_index, AttachMatrix(*jtransform, fScope,
                                                                  std::move(parent_matrix)));

        }
        return nullptr;
    }
};

namespace {

struct MaskInfo {
    SkBlendMode       fBlendMode;      // used when masking with layers/blending
    sksg::Merge::Mode fMergeMode;      // used when clipping
    bool              fInvertGeometry;
};

const MaskInfo* GetMaskInfo(char mode) {
    static constexpr MaskInfo k_add_info =
        { SkBlendMode::kSrcOver   , sksg::Merge::Mode::kUnion     , false };
    static constexpr MaskInfo k_int_info =
        { SkBlendMode::kSrcIn     , sksg::Merge::Mode::kIntersect , false };
    // AE 'subtract' is the same as 'intersect' + inverted geometry
    // (draws the opacity-adjusted paint *outside* the shape).
    static constexpr MaskInfo k_sub_info =
        { SkBlendMode::kSrcIn     , sksg::Merge::Mode::kIntersect , true  };
    static constexpr MaskInfo k_dif_info =
        { SkBlendMode::kDifference, sksg::Merge::Mode::kDifference, false };

    switch (mode) {
    case 'a': return &k_add_info;
    case 'f': return &k_dif_info;
    case 'i': return &k_int_info;
    case 's': return &k_sub_info;
    default: break;
    }

    return nullptr;
}

sk_sp<sksg::RenderNode> AttachMask(const skjson::ArrayValue* jmask,
                                   AnimatorScope* ascope,
                                   sk_sp<sksg::RenderNode> childNode) {
    if (!jmask) return childNode;

    struct MaskRecord {
        sk_sp<sksg::Path>  mask_path;  // for clipping and masking
        sk_sp<sksg::Color> mask_paint; // for masking
        sksg::Merge::Mode  merge_mode; // for clipping
    };

    SkSTArray<4, MaskRecord, true> mask_stack;

    bool has_opacity = false;

    for (const skjson::ObjectValue* m : *jmask) {
        if (!m) continue;

        const skjson::StringValue* jmode = (*m)["mode"];
        if (!jmode || jmode->size() != 1) {
            LogFail((*m)["mode"], "Invalid mask mode");
            continue;
        }

        const auto mode = *jmode->begin();
        if (mode == 'n') {
            // "None" masks have no effect.
            continue;
        }

        const auto* mask_info = GetMaskInfo(mode);
        if (!mask_info) {
            LOG("?? Unsupported mask mode: '%c'\n", mode);
            continue;
        }

        auto mask_path = AttachPath((*m)["pt"], ascope);
        if (!mask_path) {
            LogFail(*m, "Could not parse mask path");
            continue;
        }

        // "inv" is cumulative with mask info fInvertGeometry
        const auto inverted =
            (mask_info->fInvertGeometry != ParseDefault<bool>((*m)["inv"], false));
        mask_path->setFillType(inverted ? SkPath::kInverseWinding_FillType
                                        : SkPath::kWinding_FillType);

        auto mask_paint = sksg::Color::Make(SK_ColorBLACK);
        mask_paint->setAntiAlias(true);
        // First mask in the stack initializes the mask buffer.
        mask_paint->setBlendMode(mask_stack.empty() ? SkBlendMode::kSrc
                                                    : mask_info->fBlendMode);

        has_opacity |= BindProperty<ScalarValue>((*m)["o"], ascope,
            [mask_paint](const ScalarValue& o) {
                mask_paint->setOpacity(o * 0.01f);
        }, 100.0f);

        mask_stack.push_back({mask_path, mask_paint, mask_info->fMergeMode});
    }

    if (mask_stack.empty())
        return childNode;

    // If the masks are fully opaque, we can clip.
    if (!has_opacity) {
        sk_sp<sksg::GeometryNode> clip_node;

        if (mask_stack.count() == 1) {
            // Single path -> just clip.
            clip_node = std::move(mask_stack.front().mask_path);
        } else {
            // Multiple clip paths -> merge.
            std::vector<sksg::Merge::Rec> merge_recs;
            merge_recs.reserve(SkToSizeT(mask_stack.count()));

            for (const auto& mask : mask_stack) {
                const auto mode = merge_recs.empty() ? sksg::Merge::Mode::kMerge : mask.merge_mode;
                merge_recs.push_back({std::move(mask.mask_path), mode});
            }
            clip_node = sksg::Merge::Make(std::move(merge_recs));
        }

        return sksg::ClipEffect::Make(std::move(childNode), std::move(clip_node), true);
    }

    auto mask_group = sksg::Group::Make();
    for (const auto& rec : mask_stack) {
        mask_group->addChild(sksg::Draw::Make(std::move(rec.mask_path),
                                              std::move(rec.mask_paint)));

    }

    return sksg::MaskEffect::Make(std::move(childNode), std::move(mask_group));
}


sk_sp<sksg::RenderNode> AttachFillLayerEffect(const skjson::ArrayValue* jeffect_props,
                                              AnimatorScope* ascope,
                                              sk_sp<sksg::RenderNode> layer) {
    if (!jeffect_props) return layer;

    sk_sp<sksg::Color> color_node;

    for (const skjson::ObjectValue* jprop : *jeffect_props) {
        if (!jprop) continue;

        switch (const auto ty = ParseDefault<int>((*jprop)["ty"], -1)) {
        case 2: // color
            color_node = AttachColor(*jprop, ascope, "v");
            break;
        default:
            LOG("?? Ignoring unsupported fill effect poperty type: %d\n", ty);
            break;
        }
    }

    return color_node
        ? sksg::ColorModeFilter::Make(std::move(layer), std::move(color_node), SkBlendMode::kSrcIn)
        : nullptr;
}

} // namespace

sk_sp<sksg::RenderNode> AnimationBuilder::attachLayerEffects(const skjson::ArrayValue& jeffects,
                                                             AnimatorScope* ascope,
                                                             sk_sp<sksg::RenderNode> layer) {
    for (const skjson::ObjectValue* jeffect : jeffects) {
        if (!jeffect) continue;

        switch (const auto ty = ParseDefault<int>((*jeffect)["ty"], -1)) {
        case 21: // Fill
            layer = AttachFillLayerEffect((*jeffect)["ef"], ascope, std::move(layer));
            break;
        default:
            LOG("?? Unsupported layer effect type: %d\n", ty);
            break;
        }
    }

    return layer;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachLayer(const skjson::ObjectValue* jlayer,
                                                     AttachLayerContext* layerCtx) {
    if (!jlayer) return nullptr;

    using LayerAttacher = sk_sp<sksg::RenderNode> (AnimationBuilder::*)(const skjson::ObjectValue&,
                                                                        AnimatorScope*);
    static constexpr LayerAttacher gLayerAttachers[] = {
        &AnimationBuilder::attachPrecompLayer,  // 'ty': 0
        &AnimationBuilder::attachSolidLayer,    // 'ty': 1
        &AnimationBuilder::attachImageLayer,    // 'ty': 2
        &AnimationBuilder::attachNullLayer,     // 'ty': 3
        &AnimationBuilder::attachShapeLayer,    // 'ty': 4
        &AnimationBuilder::attachTextLayer,     // 'ty': 5
    };

    int type = ParseDefault<int>((*jlayer)["ty"], -1);
    if (type < 0 || type >= SkTo<int>(SK_ARRAY_COUNT(gLayerAttachers))) {
        return nullptr;
    }

    AnimatorScope layer_animators;

    // Layer content.
    auto layer = (this->*(gLayerAttachers[type]))(*jlayer, &layer_animators);

    // Clip layers with explicit dimensions.
    float w = 0, h = 0;
    if (Parse<float>((*jlayer)["w"], &w) && Parse<float>((*jlayer)["h"], &h)) {
        layer = sksg::ClipEffect::Make(std::move(layer),
                                       sksg::Rect::Make(SkRect::MakeWH(w, h)),
                                       true);
    }

    // Optional layer mask.
    layer = AttachMask((*jlayer)["masksProperties"], &layer_animators, std::move(layer));

    // Optional layer transform.
    if (auto layerMatrix = layerCtx->AttachLayerMatrix(*jlayer)) {
        layer = sksg::Transform::Make(std::move(layer), std::move(layerMatrix));
    }

    // Optional layer opacity.
    // TODO: de-dupe this "ks" lookup with matrix above.
    if (const skjson::ObjectValue* jtransform = (*jlayer)["ks"]) {
        layer = AttachOpacity(*jtransform, &layer_animators, std::move(layer));
    }

    // Optional layer effects.
    if (const skjson::ArrayValue* jeffects = (*jlayer)["ef"]) {
        layer = this->attachLayerEffects(*jeffects, &layer_animators, std::move(layer));
    }

    class LayerController final : public sksg::GroupAnimator {
    public:
        LayerController(sksg::AnimatorList&& layer_animators,
                        sk_sp<sksg::OpacityEffect> controlNode,
                        float in, float out)
            : INHERITED(std::move(layer_animators))
            , fControlNode(std::move(controlNode))
            , fIn(in)
            , fOut(out) {}

        void onTick(float t) override {
            const auto active = (t >= fIn && t <= fOut);

            // Keep the layer fully transparent except for its [in..out] lifespan.
            // (note: opacity == 0 disables rendering, while opacity == 1 is a noop)
            fControlNode->setOpacity(active ? 1 : 0);

            // Dispatch ticks only while active.
            if (active) this->INHERITED::onTick(t);
        }

    private:
        const sk_sp<sksg::OpacityEffect> fControlNode;
        const float                      fIn,
                                         fOut;

        using INHERITED = sksg::GroupAnimator;
    };

    auto controller_node = sksg::OpacityEffect::Make(std::move(layer));
    const auto        in = ParseDefault<float>((*jlayer)["ip"], 0.0f),
                     out = ParseDefault<float>((*jlayer)["op"], in);

    if (in >= out || !controller_node)
        return nullptr;

    layerCtx->fScope->push_back(
        skstd::make_unique<LayerController>(std::move(layer_animators), controller_node, in, out));

    if (ParseDefault<bool>((*jlayer)["td"], false)) {
        // This layer is a matte.  We apply it as a mask to the next layer.
        layerCtx->fCurrentMatte = std::move(controller_node);
        return nullptr;
    }

    if (layerCtx->fCurrentMatte) {
        // There is a pending matte. Apply and reset.
        static constexpr sksg::MaskEffect::Mode gMaskModes[] = {
            sksg::MaskEffect::Mode::kNormal, // tt: 1
            sksg::MaskEffect::Mode::kInvert, // tt: 2
        };
        const auto matteType = ParseDefault<size_t>((*jlayer)["tt"], 1) - 1;

        if (matteType < SK_ARRAY_COUNT(gMaskModes)) {
            return sksg::MaskEffect::Make(std::move(controller_node),
                                          std::move(layerCtx->fCurrentMatte),
                                          gMaskModes[matteType]);
        }
        layerCtx->fCurrentMatte.reset();
    }

    return std::move(controller_node);
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachComposition(const skjson::ObjectValue& comp,
                                                            AnimatorScope* scope) {
    const skjson::ArrayValue* jlayers = comp["layers"];
    if (!jlayers) return nullptr;

    SkSTArray<16, sk_sp<sksg::RenderNode>, true> layers;
    AttachLayerContext                           layerCtx(*jlayers, scope);

    for (const auto& l : *jlayers) {
        if (auto layer_fragment = this->attachLayer(l, &layerCtx)) {
            layers.push_back(std::move(layer_fragment));
        }
    }

    if (layers.empty()) {
        return nullptr;
    }

    // Layers are painted in bottom->top order.
    auto comp_group = sksg::Group::Make();
    for (int i = layers.count() - 1; i >= 0; --i) {
        comp_group->addChild(std::move(layers[i]));
    }

    return std::move(comp_group);
}

AnimationBuilder::AnimationBuilder(const ResourceProvider& rp, sk_sp<SkFontMgr> fontmgr,
                                  Animation::Stats* stats, float duration, float framerate)
    : fResourceProvider(rp)
    , fFontMgr(std::move(fontmgr))
    , fStats(stats)
    , fDuration(duration)
    , fFrameRate(framerate) {}

std::unique_ptr<sksg::Scene> AnimationBuilder::parse(const skjson::ObjectValue& jroot) {
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

} // namespace internal

sk_sp<Animation> Animation::Make(SkStream* stream, const ResourceProvider* provider, Stats* stats) {
    if (!stream->hasLength()) {
        // TODO: handle explicit buffering?
        LOG("!! cannot parse streaming content\n");
        return nullptr;
    }

    auto data = SkData::MakeFromStream(stream, stream->getLength());
    if (!data) {
        SkDebugf("!! Failed to read the input stream.\n");
        return nullptr;
    }

    return Make(static_cast<const char*>(data->data()), data->size(), provider, stats);
}

sk_sp<Animation> Animation::Make(const char* data, size_t data_len,
                                 const ResourceProvider* provider, Stats* stats) {
    Stats stats_storage;
    if (!stats)
        stats = &stats_storage;
    memset(stats, 0, sizeof(struct Stats));

    stats->fJsonSize = data_len;
    const auto t0 = SkTime::GetMSecs();

    const skjson::DOM dom(data, data_len);
    if (!dom.root().is<skjson::ObjectValue>()) {
        // TODO: more error info.
        SkDebugf("!! Failed to parse JSON input.\n");
        return nullptr;
    }
    const auto& json = dom.root().as<skjson::ObjectValue>();

    const auto t1 = SkTime::GetMSecs();
    stats->fJsonParseTimeMS = t1 - t0;

    const auto version  = ParseDefault<SkString>(json["v"], SkString());
    const auto size     = SkSize::Make(ParseDefault<float>(json["w"], 0.0f),
                                       ParseDefault<float>(json["h"], 0.0f));
    const auto fps      = ParseDefault<float>(json["fr"], -1.0f),
               inPoint  = ParseDefault<float>(json["ip"], 0.0f),
               outPoint = SkTMax(ParseDefault<float>(json["op"], SK_ScalarMax), inPoint);

    if (size.isEmpty() || version.isEmpty() || fps <= 0 ||
        !SkScalarIsFinite(inPoint) || !SkScalarIsFinite(outPoint)) {
        LOG("!! invalid animation params (version: %s, size: [%f %f], frame rate: %f, "
            "in-point: %f, out-point: %f)\n",
            version.c_str(), size.width(), size.height(), fps, inPoint, outPoint);
        return nullptr;
    }

    class NullResourceProvider final : public ResourceProvider {
        sk_sp<SkData> load(const char[], const char[]) const override { return nullptr; }
    };

    NullResourceProvider null_provider;
    const auto anim = sk_sp<Animation>(new Animation(provider ? *provider : null_provider,
                                                     std::move(version), size, fps,
                                                     inPoint, outPoint, json, stats));
    const auto t2 = SkTime::GetMSecs();
    stats->fSceneParseTimeMS = t2 - t1;
    stats->fTotalLoadTimeMS  = t2 - t0;

    return anim;
}

sk_sp<Animation> Animation::MakeFromFile(const char path[], const ResourceProvider* res,
                                         Stats* stats) {
    class DirectoryResourceProvider final : public ResourceProvider {
    public:
        explicit DirectoryResourceProvider(SkString dir) : fDir(std::move(dir)) {}

        sk_sp<SkData> load(const char resource_path[], const char resource_name[]) const override {
            const auto full_dir  = SkOSPath::Join(fDir.c_str(), resource_path),
                       full_path = SkOSPath::Join(full_dir.c_str(), resource_name);
            return SkData::MakeFromFileName(full_path.c_str());
        }

    private:
        const SkString fDir;
    };

    const auto data =  SkData::MakeFromFileName(path);
    if (!data)
        return nullptr;

    std::unique_ptr<ResourceProvider> defaultProvider;
    if (!res) {
        defaultProvider = skstd::make_unique<DirectoryResourceProvider>(SkOSPath::Dirname(path));
    }

    return Make(static_cast<const char*>(data->data()), data->size(),
                res ? res : defaultProvider.get(), stats);
}

Animation::Animation(const ResourceProvider& resources,
                     SkString version, const SkSize& size,
                     SkScalar fps, SkScalar in, SkScalar out,
                     const skjson::ObjectValue& json, Stats* stats)
    : fVersion(std::move(version))
    , fSize(size)
    , fFrameRate(fps)
    , fInPoint(in)
    , fOutPoint(out) {

    internal::AnimationBuilder builder(resources, SkFontMgr::RefDefault(), stats,
                                       this->duration(), fps);

    fScene = builder.parse(json);

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
    if (!fScene)
        return;

    SkAutoCanvasRestore restore(canvas, true);
    const SkRect srcR = SkRect::MakeSize(this->size());
    if (dstR) {
        canvas->concat(SkMatrix::MakeRectToRect(srcR, *dstR, SkMatrix::kCenter_ScaleToFit));
    }
    canvas->clipRect(srcR);
    fScene->render(canvas);
}

void Animation::seek(SkScalar t) {
    if (!fScene)
        return;

    fScene->animate(fInPoint + SkTPin(t, 0.0f, 1.0f) * (fOutPoint - fInPoint));
}

} // namespace skottie
