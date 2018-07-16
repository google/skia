/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Skottie.h"

#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkMakeUnique.h"
#include "SkOSPath.h"
#include "SkPaint.h"
#include "SkParse.h"
#include "SkPoint.h"
#include "SkSGClipEffect.h"
#include "SkSGColor.h"
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
#include "SkottieValue.h"

#include <cmath>
#include <vector>

#include "stdlib.h"

namespace skottie {

#define LOG SkDebugf

namespace {

struct AssetInfo {
    const skjson::ObjectValue* fAsset;
    mutable bool               fIsAttaching; // Used for cycle detection
};

using AssetMap = SkTHashMap<SkString, AssetInfo>;

struct AttachContext {
    const ResourceProvider& fResources;
    const AssetMap&         fAssets;
    const float             fDuration,
                            fFrameRate;
    sksg::AnimatorList&     fAnimators;
};

bool LogFail(const skjson::Value& json, const char* msg) {
    const auto dump = json.toString();
    LOG("!! %s: %s\n", msg, dump.c_str());
    return false;
}

sk_sp<sksg::Matrix> AttachMatrix(const skjson::ObjectValue& t, AttachContext* ctx,
                                 sk_sp<sksg::Matrix> parentMatrix) {
    static const VectorValue g_default_vec_0   = {  0,   0},
                             g_default_vec_100 = {100, 100};

    auto matrix = sksg::Matrix::Make(SkMatrix::I(), parentMatrix);
    auto adapter = sk_make_sp<TransformAdapter>(matrix);

    auto bound = BindProperty<VectorValue>(t["a"], &ctx->fAnimators,
            [adapter](const VectorValue& a) {
                adapter->setAnchorPoint(ValueTraits<VectorValue>::As<SkPoint>(a));
            }, g_default_vec_0);
    bound |= BindProperty<VectorValue>(t["p"], &ctx->fAnimators,
            [adapter](const VectorValue& p) {
                adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
            }, g_default_vec_0);
    bound |= BindProperty<VectorValue>(t["s"], &ctx->fAnimators,
            [adapter](const VectorValue& s) {
                adapter->setScale(ValueTraits<VectorValue>::As<SkVector>(s));
            }, g_default_vec_100);

    const auto* jrotation = &t["r"];
    if (jrotation->is<skjson::NullValue>()) {
        // 3d rotations have separate rx,ry,rz components.  While we don't fully support them,
        // we can still make use of rz.
        jrotation = &t["rz"];
    }
    bound |= BindProperty<ScalarValue>(*jrotation, &ctx->fAnimators,
            [adapter](const ScalarValue& r) {
                adapter->setRotation(r);
            }, 0.0f);
    bound |= BindProperty<ScalarValue>(t["sk"], &ctx->fAnimators,
            [adapter](const ScalarValue& sk) {
                adapter->setSkew(sk);
            }, 0.0f);
    bound |= BindProperty<ScalarValue>(t["sa"], &ctx->fAnimators,
            [adapter](const ScalarValue& sa) {
                adapter->setSkewAxis(sa);
            }, 0.0f);

    return bound ? matrix : parentMatrix;
}

sk_sp<sksg::RenderNode> AttachOpacity(const skjson::ObjectValue& jtransform, AttachContext* ctx,
                                      sk_sp<sksg::RenderNode> childNode) {
    if (!childNode)
        return nullptr;

    auto opacityNode = sksg::OpacityEffect::Make(childNode);

    if (!BindProperty<ScalarValue>(jtransform["o"], &ctx->fAnimators,
        [opacityNode](const ScalarValue& o) {
            // BM opacity is [0..100]
            opacityNode->setOpacity(o * 0.01f);
        }, 100.0f)) {
        // We can ignore static full opacity.
        return childNode;
    }

    return std::move(opacityNode);
}

sk_sp<sksg::RenderNode> AttachComposition(const skjson::ObjectValue&, AttachContext* ctx);

sk_sp<sksg::Path> AttachPath(const skjson::Value& jpath, AttachContext* ctx) {
    auto path_node = sksg::Path::Make();
    return BindProperty<ShapeValue>(jpath, &ctx->fAnimators,
        [path_node](const ShapeValue& p) {
            // FillType is tracked in the SG node, not in keyframes -- make sure we preserve it.
            auto path = ValueTraits<ShapeValue>::As<SkPath>(p);
            path.setFillType(path_node->getFillType());
            path_node->setPath(path);
        })
        ? path_node
        : nullptr;
}

sk_sp<sksg::GeometryNode> AttachPathGeometry(const skjson::ObjectValue& jpath, AttachContext* ctx) {
    return AttachPath(jpath["ks"], ctx);
}

sk_sp<sksg::GeometryNode> AttachRRectGeometry(const skjson::ObjectValue& jrect,
                                              AttachContext* ctx) {
    auto rect_node = sksg::RRect::Make();
    auto adapter = sk_make_sp<RRectAdapter>(rect_node);

    auto p_attached = BindProperty<VectorValue>(jrect["p"], &ctx->fAnimators,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    auto s_attached = BindProperty<VectorValue>(jrect["s"], &ctx->fAnimators,
        [adapter](const VectorValue& s) {
            adapter->setSize(ValueTraits<VectorValue>::As<SkSize>(s));
        });
    auto r_attached = BindProperty<ScalarValue>(jrect["r"], &ctx->fAnimators,
        [adapter](const ScalarValue& r) {
            adapter->setRadius(SkSize::Make(r, r));
        });

    if (!p_attached && !s_attached && !r_attached) {
        return nullptr;
    }

    return std::move(rect_node);
}

sk_sp<sksg::GeometryNode> AttachEllipseGeometry(const skjson::ObjectValue& jellipse,
                                                AttachContext* ctx) {
    auto rect_node = sksg::RRect::Make();
    auto adapter = sk_make_sp<RRectAdapter>(rect_node);

    auto p_attached = BindProperty<VectorValue>(jellipse["p"], &ctx->fAnimators,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    auto s_attached = BindProperty<VectorValue>(jellipse["s"], &ctx->fAnimators,
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
                                                 AttachContext* ctx) {
    static constexpr PolyStarAdapter::Type gTypes[] = {
        PolyStarAdapter::Type::kStar, // "sy": 1
        PolyStarAdapter::Type::kPoly, // "sy": 2
    };

    const auto type = ParseDefault<int>(jstar["sy"], 0) - 1;
    if (type < 0 || type >= SkTo<int>(SK_ARRAY_COUNT(gTypes))) {
        LogFail(jstar, "Unknown polystar type");
        return nullptr;
    }

    auto path_node = sksg::Path::Make();
    auto adapter = sk_make_sp<PolyStarAdapter>(path_node, gTypes[type]);

    BindProperty<VectorValue>(jstar["p"], &ctx->fAnimators,
        [adapter](const VectorValue& p) {
            adapter->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    BindProperty<ScalarValue>(jstar["pt"], &ctx->fAnimators,
        [adapter](const ScalarValue& pt) {
            adapter->setPointCount(pt);
        });
    BindProperty<ScalarValue>(jstar["ir"], &ctx->fAnimators,
        [adapter](const ScalarValue& ir) {
            adapter->setInnerRadius(ir);
        });
    BindProperty<ScalarValue>(jstar["or"], &ctx->fAnimators,
        [adapter](const ScalarValue& otr) {
            adapter->setOuterRadius(otr);
        });
    BindProperty<ScalarValue>(jstar["is"], &ctx->fAnimators,
        [adapter](const ScalarValue& is) {
            adapter->setInnerRoundness(is);
        });
    BindProperty<ScalarValue>(jstar["os"], &ctx->fAnimators,
        [adapter](const ScalarValue& os) {
            adapter->setOuterRoundness(os);
        });
    BindProperty<ScalarValue>(jstar["r"], &ctx->fAnimators,
        [adapter](const ScalarValue& r) {
            adapter->setRotation(r);
        });

    return std::move(path_node);
}

sk_sp<sksg::Color> AttachColor(const skjson::ObjectValue& jcolor, AttachContext* ctx) {
    auto color_node = sksg::Color::Make(SK_ColorBLACK);
    BindProperty<VectorValue>(jcolor["c"], &ctx->fAnimators,
        [color_node](const VectorValue& c) {
            color_node->setColor(ValueTraits<VectorValue>::As<SkColor>(c));
        });

    return color_node;
}

sk_sp<sksg::Gradient> AttachGradient(const skjson::ObjectValue& jgrad, AttachContext* ctx) {
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

    BindProperty<VectorValue>((*stops)["k"], &ctx->fAnimators,
        [adapter](const VectorValue& stops) {
            adapter->setColorStops(stops);
        });
    BindProperty<VectorValue>(jgrad["s"], &ctx->fAnimators,
        [adapter](const VectorValue& s) {
            adapter->setStartPoint(ValueTraits<VectorValue>::As<SkPoint>(s));
        });
    BindProperty<VectorValue>(jgrad["e"], &ctx->fAnimators,
        [adapter](const VectorValue& e) {
            adapter->setEndPoint(ValueTraits<VectorValue>::As<SkPoint>(e));
        });

    return gradient_node;
}

sk_sp<sksg::PaintNode> AttachPaint(const skjson::ObjectValue& jpaint, AttachContext* ctx,
                                   sk_sp<sksg::PaintNode> paint_node) {
    if (paint_node) {
        paint_node->setAntiAlias(true);

        BindProperty<ScalarValue>(jpaint["o"], &ctx->fAnimators,
            [paint_node](const ScalarValue& o) {
                // BM opacity is [0..100]
                paint_node->setOpacity(o * 0.01f);
        });
    }

    return paint_node;
}

sk_sp<sksg::PaintNode> AttachStroke(const skjson::ObjectValue& jstroke, AttachContext* ctx,
                                    sk_sp<sksg::PaintNode> stroke_node) {
    if (!stroke_node)
        return nullptr;

    stroke_node->setStyle(SkPaint::kStroke_Style);

    BindProperty<ScalarValue>(jstroke["w"], &ctx->fAnimators,
        [stroke_node](const ScalarValue& w) {
            stroke_node->setStrokeWidth(w);
        });

    stroke_node->setStrokeMiter(ParseDefault<SkScalar>(jstroke["ml"], 4.0f));

    static constexpr SkPaint::Join gJoins[] = {
        SkPaint::kMiter_Join,
        SkPaint::kRound_Join,
        SkPaint::kBevel_Join,
    };
    stroke_node->setStrokeJoin(gJoins[SkTPin<int>(ParseDefault<int>(jstroke["lj"], 1) - 1,
                                                  0, SK_ARRAY_COUNT(gJoins) - 1)]);

    static constexpr SkPaint::Cap gCaps[] = {
        SkPaint::kButt_Cap,
        SkPaint::kRound_Cap,
        SkPaint::kSquare_Cap,
    };
    stroke_node->setStrokeCap(gCaps[SkTPin<int>(ParseDefault(jstroke["lc"], 1) - 1,
                                                0, SK_ARRAY_COUNT(gCaps) - 1)]);

    return stroke_node;
}

sk_sp<sksg::PaintNode> AttachColorFill(const skjson::ObjectValue& jfill, AttachContext* ctx) {
    return AttachPaint(jfill, ctx, AttachColor(jfill, ctx));
}

sk_sp<sksg::PaintNode> AttachGradientFill(const skjson::ObjectValue& jfill, AttachContext* ctx) {
    return AttachPaint(jfill, ctx, AttachGradient(jfill, ctx));
}

sk_sp<sksg::PaintNode> AttachColorStroke(const skjson::ObjectValue& jstroke, AttachContext* ctx) {
    return AttachStroke(jstroke, ctx, AttachPaint(jstroke, ctx, AttachColor(jstroke, ctx)));
}

sk_sp<sksg::PaintNode> AttachGradientStroke(const skjson::ObjectValue& jstroke,
                                            AttachContext* ctx) {
    return AttachStroke(jstroke, ctx, AttachPaint(jstroke, ctx, AttachGradient(jstroke, ctx)));
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
        const skjson::ObjectValue& jmerge, AttachContext*,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {
    static constexpr sksg::Merge::Mode gModes[] = {
        sksg::Merge::Mode::kMerge,      // "mm": 1
        sksg::Merge::Mode::kUnion,      // "mm": 2
        sksg::Merge::Mode::kDifference, // "mm": 3
        sksg::Merge::Mode::kIntersect,  // "mm": 4
        sksg::Merge::Mode::kXOR      ,  // "mm": 5
    };

    const auto mode = gModes[SkTPin<int>(ParseDefault<int>(jmerge["mm"], 1) - 1,
                                         0, SK_ARRAY_COUNT(gModes) - 1)];

    std::vector<sk_sp<sksg::GeometryNode>> merged;
    merged.push_back(Merge(std::move(geos), mode));

    return merged;
}

std::vector<sk_sp<sksg::GeometryNode>> AttachTrimGeometryEffect(
        const skjson::ObjectValue& jtrim, AttachContext* ctx,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {

    enum class Mode {
        kMerged,   // "m": 1
        kSeparate, // "m": 2
    } gModes[] = { Mode::kMerged, Mode::kSeparate };

    const auto mode = gModes[SkTPin<int>(ParseDefault<int>(jtrim["m"], 1) - 1,
                                         0, SK_ARRAY_COUNT(gModes) - 1)];

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
        BindProperty<ScalarValue>(jtrim["s"], &ctx->fAnimators,
            [adapter](const ScalarValue& s) {
                adapter->setStart(s);
            });
        BindProperty<ScalarValue>(jtrim["e"], &ctx->fAnimators,
            [adapter](const ScalarValue& e) {
                adapter->setEnd(e);
            });
        BindProperty<ScalarValue>(jtrim["o"], &ctx->fAnimators,
            [adapter](const ScalarValue& o) {
                adapter->setOffset(o);
            });
    }

    return trimmed;
}

std::vector<sk_sp<sksg::GeometryNode>> AttachRoundGeometryEffect(
        const skjson::ObjectValue& jtrim, AttachContext* ctx,
        std::vector<sk_sp<sksg::GeometryNode>>&& geos) {

    std::vector<sk_sp<sksg::GeometryNode>> rounded;
    rounded.reserve(geos.size());

    for (const auto& g : geos) {
        const auto roundEffect = sksg::RoundEffect::Make(std::move(g));
        rounded.push_back(roundEffect);

        BindProperty<ScalarValue>(jtrim["r"], &ctx->fAnimators,
            [roundEffect](const ScalarValue& r) {
                roundEffect->setRadius(r);
            });
    }

    return rounded;
}

using GeometryAttacherT = sk_sp<sksg::GeometryNode> (*)(const skjson::ObjectValue&, AttachContext*);
static constexpr GeometryAttacherT gGeometryAttachers[] = {
    AttachPathGeometry,
    AttachRRectGeometry,
    AttachEllipseGeometry,
    AttachPolystarGeometry,
};

using PaintAttacherT = sk_sp<sksg::PaintNode> (*)(const skjson::ObjectValue&, AttachContext*);
static constexpr PaintAttacherT gPaintAttachers[] = {
    AttachColorFill,
    AttachColorStroke,
    AttachGradientFill,
    AttachGradientStroke,
};

using GeometryEffectAttacherT =
    std::vector<sk_sp<sksg::GeometryNode>> (*)(const skjson::ObjectValue&,
                                               AttachContext*,
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

    SkString type;
    if (!Parse<SkString>(jshape["ty"], &type) || type.isEmpty())
        return nullptr;

    const auto* info = bsearch(type.c_str(),
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
    AttachShapeContext(AttachContext* ctx,
                       std::vector<sk_sp<sksg::GeometryNode>>* geos,
                       std::vector<GeometryEffectRec>* effects,
                       size_t committedAnimators)
        : fCtx(ctx)
        , fGeometryStack(geos)
        , fGeometryEffectStack(effects)
        , fCommittedAnimators(committedAnimators) {}

    AttachContext*                          fCtx;
    std::vector<sk_sp<sksg::GeometryNode>>* fGeometryStack;
    std::vector<GeometryEffectRec>*         fGeometryEffectStack;
    size_t                                  fCommittedAnimators;
};

sk_sp<sksg::RenderNode> AttachShape(const skjson::ArrayValue* jshape, AttachShapeContext* shapeCtx) {
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
            if ((shape_matrix = AttachMatrix(*shape, shapeCtx->fCtx, nullptr))) {
                shape_wrapper = sksg::Transform::Make(std::move(shape_wrapper), shape_matrix);
            }
            shape_wrapper = AttachOpacity(*shape, shapeCtx->fCtx, std::move(shape_wrapper));
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
                                                                         shapeCtx->fCtx)) {
                geos.push_back(std::move(geo));
            }
        } break;
        case ShapeType::kGeometryEffect: {
            // Apply the current effect and pop from the stack.
            SkASSERT(rec->fInfo.fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            if (!geos.empty()) {
                geos = gGeometryEffectAttachers[rec->fInfo.fAttacherIndex](rec->fJson,
                                                                           shapeCtx->fCtx,
                                                                           std::move(geos));
            }

            SkASSERT(&shapeCtx->fGeometryEffectStack->back().fJson == &rec->fJson);
            SkASSERT(shapeCtx->fGeometryEffectStack->back().fAttach ==
                     gGeometryEffectAttachers[rec->fInfo.fAttacherIndex]);
            shapeCtx->fGeometryEffectStack->pop_back();
        } break;
        case ShapeType::kGroup: {
            AttachShapeContext groupShapeCtx(shapeCtx->fCtx,
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
            auto paint = gPaintAttachers[rec->fInfo.fAttacherIndex](rec->fJson, shapeCtx->fCtx);
            if (!paint || geos.empty())
                break;

            auto drawGeos = geos;

            // Apply all pending effects from the stack.
            for (auto it = shapeCtx->fGeometryEffectStack->rbegin();
                 it != shapeCtx->fGeometryEffectStack->rend(); ++it) {
                drawGeos = it->fAttach(it->fJson, shapeCtx->fCtx, std::move(drawGeos));
            }

            // If we still have multiple geos, reduce using 'merge'.
            auto geo = drawGeos.size() > 1
                ? Merge(std::move(drawGeos), sksg::Merge::Mode::kMerge)
                : drawGeos[0];

            SkASSERT(geo);
            draws.push_back(sksg::Draw::Make(std::move(geo), std::move(paint)));
            shapeCtx->fCommittedAnimators = shapeCtx->fCtx->fAnimators.size();
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

sk_sp<sksg::RenderNode> AttachNestedAnimation(const char* path, AttachContext* ctx) {
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

        void onRender(SkCanvas* canvas) const override {
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

    const auto resStream  = ctx->fResources.openStream(path);
    if (!resStream || !resStream->hasLength()) {
        LOG("!! Could not open: %s\n", path);
        return nullptr;
    }

    auto animation = Animation::Make(resStream.get(), &ctx->fResources);
    if (!animation) {
        LOG("!! Could not load nested animation: %s\n", path);
        return nullptr;
    }


    ctx->fAnimators.push_back(
        skstd::make_unique<SkottieAnimatorAdapter>(animation,
                                                   animation->duration() / ctx->fDuration));

    return sk_make_sp<SkottieSGAdapter>(std::move(animation));
}

sk_sp<sksg::RenderNode> AttachAssetRef(const skjson::ObjectValue& jlayer, AttachContext* ctx,
    sk_sp<sksg::RenderNode>(*attach_proc)(const skjson::ObjectValue& comp, AttachContext* ctx)) {

    const auto refId = ParseDefault<SkString>(jlayer["refId"], SkString());
    if (refId.isEmpty()) {
        LOG("!! Layer missing refId\n");
        return nullptr;
    }

    if (refId.startsWith("$")) {
        return AttachNestedAnimation(refId.c_str() + 1, ctx);
    }

    const auto* asset_info = ctx->fAssets.find(refId);
    if (!asset_info) {
        LOG("!! Asset not found: '%s'\n", refId.c_str());
        return nullptr;
    }

    if (asset_info->fIsAttaching) {
        LOG("!! Asset cycle detected for: '%s'\n", refId.c_str());
        return nullptr;
    }

    asset_info->fIsAttaching = true;
    auto asset = attach_proc(*asset_info->fAsset, ctx);
    asset_info->fIsAttaching = false;

    return asset;
}

sk_sp<sksg::RenderNode> AttachCompLayer(const skjson::ObjectValue& jlayer, AttachContext* ctx) {
    const skjson::ObjectValue* time_remap = jlayer["tm"];
    const auto start_time = ParseDefault<float>(jlayer["st"], 0.0f),
             stretch_time = ParseDefault<float>(jlayer["sr"], 1.0f);
    const auto requires_time_mapping = !SkScalarNearlyEqual(start_time  , 0) ||
                                       !SkScalarNearlyEqual(stretch_time, 1) ||
                                       time_remap;

    sksg::AnimatorList local_animators;
    AttachContext local_ctx = { ctx->fResources,
                                ctx->fAssets,
                                ctx->fDuration,
                                ctx->fFrameRate,
                                requires_time_mapping ? local_animators : ctx->fAnimators };

    auto comp_layer = AttachAssetRef(jlayer, &local_ctx, AttachComposition);

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
            auto  frame_rate = ctx->fFrameRate;
            BindProperty<ScalarValue>(*time_remap, &ctx->fAnimators,
                    [raw_mapper, frame_rate](const ScalarValue& t) {
                        raw_mapper->remapTime(t * frame_rate);
                    });
        }
        ctx->fAnimators.push_back(std::move(time_mapper));
    }

    return comp_layer;
}

sk_sp<sksg::RenderNode> AttachSolidLayer(const skjson::ObjectValue& jlayer, AttachContext*) {
    const auto size = SkSize::Make(ParseDefault<float>(jlayer["sw"], 0.0f),
                                   ParseDefault<float>(jlayer["sh"], 0.0f));
    const auto hex = ParseDefault<SkString>(jlayer["sc"], SkString());
    uint32_t c;
    if (size.isEmpty() ||
        !hex.startsWith("#") ||
        !SkParse::FindHex(hex.c_str() + 1, &c)) {
        LogFail(jlayer, "Could not parse solid layer");
        return nullptr;
    }

    const SkColor color = 0xff000000 | c;

    return sksg::Draw::Make(sksg::Rect::Make(SkRect::MakeSize(size)),
                            sksg::Color::Make(color));
}

sk_sp<sksg::RenderNode> AttachImageAsset(const skjson::ObjectValue& jimage, AttachContext* ctx) {
    const auto name = ParseDefault<SkString>(jimage["p"], SkString()),
               path = ParseDefault<SkString>(jimage["u"], SkString());
    if (name.isEmpty())
        return nullptr;

    // TODO: plumb resource paths explicitly to ResourceProvider?
    const auto resName    = path.isEmpty() ? name : SkOSPath::Join(path.c_str(), name.c_str());
    const auto resStream  = ctx->fResources.openStream(resName.c_str());
    if (!resStream || !resStream->hasLength()) {
        LOG("!! Could not load image resource: %s\n", resName.c_str());
        return nullptr;
    }

    // TODO: non-intrisic image sizing
    return sksg::Image::Make(
        SkImage::MakeFromEncoded(SkData::MakeFromStream(resStream.get(), resStream->getLength())));
}

sk_sp<sksg::RenderNode> AttachImageLayer(const skjson::ObjectValue& jlayer, AttachContext* ctx) {
    return AttachAssetRef(jlayer, ctx, AttachImageAsset);
}

sk_sp<sksg::RenderNode> AttachNullLayer(const skjson::ObjectValue& layer, AttachContext*) {
    // Null layers are used solely to drive dependent transforms,
    // but we use free-floating sksg::Matrices for that purpose.
    return nullptr;
}

sk_sp<sksg::RenderNode> AttachShapeLayer(const skjson::ObjectValue& layer, AttachContext* ctx) {
    std::vector<sk_sp<sksg::GeometryNode>> geometryStack;
    std::vector<GeometryEffectRec> geometryEffectStack;
    AttachShapeContext shapeCtx(ctx, &geometryStack, &geometryEffectStack, ctx->fAnimators.size());
    auto shapeNode = AttachShape(layer["shapes"], &shapeCtx);

    // Trim uncommitted animators: AttachShape consumes effects on the fly, and greedily attaches
    // geometries => at the end, we can end up with unused geometries, which are nevertheless alive
    // due to attached animators.  To avoid this, we track committed animators and discard the
    // orphans here.
    SkASSERT(shapeCtx.fCommittedAnimators <= ctx->fAnimators.size());
    ctx->fAnimators.resize(shapeCtx.fCommittedAnimators);

    return shapeNode;
}

sk_sp<sksg::RenderNode> AttachTextLayer(const skjson::ObjectValue& layer, AttachContext*) {
    LOG("?? Text layer stub\n");
    return nullptr;
}

struct AttachLayerContext {
    AttachLayerContext(const skjson::ArrayValue& jlayers, AttachContext* ctx)
        : fLayerList(jlayers), fCtx(ctx) {}

    const skjson::ArrayValue&            fLayerList;
    AttachContext*                       fCtx;
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
            return *fLayerMatrixMap.set(layer_index, AttachMatrix(*jtransform, fCtx,
                                                                  std::move(parent_matrix)));

        }
        return nullptr;
    }
};

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
                                   AttachContext* ctx,
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

        SkString mode;
        if (!Parse<SkString>((*m)["mode"], &mode) || mode.size() != 1) {
            LogFail((*m)["mode"], "Invalid mask mode");
            continue;
        }

        if (mode[0] == 'n') {
            // "None" masks have no effect.
            continue;
        }

        const auto* mask_info = GetMaskInfo(mode[0]);
        if (!mask_info) {
            LOG("?? Unsupported mask mode: '%c'\n", mode[0]);
            continue;
        }

        auto mask_path = AttachPath((*m)["pt"], ctx);
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

        has_opacity |= BindProperty<ScalarValue>((*m)["o"], &ctx->fAnimators,
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

sk_sp<sksg::RenderNode> AttachLayer(const skjson::ObjectValue* jlayer,
                                    AttachLayerContext* layerCtx) {
    if (!jlayer) return nullptr;

    if (!(*jlayer)["ef"].is<skjson::NullValue>()) {
        LOG("?? Unsupported layer effect.\n");
    }

    using LayerAttacher = sk_sp<sksg::RenderNode> (*)(const skjson::ObjectValue&, AttachContext*);
    static constexpr LayerAttacher gLayerAttachers[] = {
        AttachCompLayer,  // 'ty': 0
        AttachSolidLayer, // 'ty': 1
        AttachImageLayer, // 'ty': 2
        AttachNullLayer,  // 'ty': 3
        AttachShapeLayer, // 'ty': 4
        AttachTextLayer,  // 'ty': 5
    };

    int type = ParseDefault<int>((*jlayer)["ty"], -1);
    if (type < 0 || type >= SkTo<int>(SK_ARRAY_COUNT(gLayerAttachers))) {
        return nullptr;
    }

    sksg::AnimatorList layer_animators;
    AttachContext local_ctx = { layerCtx->fCtx->fResources,
                                layerCtx->fCtx->fAssets,
                                layerCtx->fCtx->fDuration,
                                layerCtx->fCtx->fFrameRate,
                                layer_animators};

    // Layer content.
    auto layer = gLayerAttachers[type](*jlayer, &local_ctx);

    // Clip layers with explicit dimensions.
    float w = 0, h = 0;
    if (Parse<float>((*jlayer)["w"], &w) && Parse<float>((*jlayer)["h"], &h)) {
        layer = sksg::ClipEffect::Make(std::move(layer),
                                       sksg::Rect::Make(SkRect::MakeWH(w, h)),
                                       true);
    }

    // Optional layer mask.
    layer = AttachMask((*jlayer)["masksProperties"], &local_ctx, std::move(layer));

    // Optional layer transform.
    if (auto layerMatrix = layerCtx->AttachLayerMatrix(*jlayer)) {
        layer = sksg::Transform::Make(std::move(layer), std::move(layerMatrix));
    }

    // Optional layer opacity.
    // TODO: de-dupe this "ks" lookup with matrix above.
    if (const skjson::ObjectValue* jtransform = (*jlayer)["ks"]) {
        layer = AttachOpacity(*jtransform, &local_ctx, std::move(layer));
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

    layerCtx->fCtx->fAnimators.push_back(
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
        const auto matteType = ParseDefault<int>((*jlayer)["tt"], 1) - 1;

        if (matteType >= 0 && matteType < SkTo<int>(SK_ARRAY_COUNT(gMaskModes))) {
            return sksg::MaskEffect::Make(std::move(controller_node),
                                          std::move(layerCtx->fCurrentMatte),
                                          gMaskModes[matteType]);
        }
        layerCtx->fCurrentMatte.reset();
    }

    return std::move(controller_node);
}

sk_sp<sksg::RenderNode> AttachComposition(const skjson::ObjectValue& comp, AttachContext* ctx) {
    const skjson::ArrayValue* jlayers = comp["layers"];
    if (!jlayers) return nullptr;

    SkSTArray<16, sk_sp<sksg::RenderNode>, true> layers;
    AttachLayerContext                           layerCtx(*jlayers, ctx);

    for (const auto& l : *jlayers) {
        if (auto layer_fragment = AttachLayer(l, &layerCtx)) {
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

} // namespace

sk_sp<Animation> Animation::Make(SkStream* stream, const ResourceProvider* provider, Stats* stats) {
    Stats stats_storage;
    if (!stats)
        stats = &stats_storage;
    memset(stats, 0, sizeof(struct Stats));

    if (!stream->hasLength()) {
        // TODO: handle explicit buffering?
        LOG("!! cannot parse streaming content\n");
        return nullptr;
    }

    stats->fJsonSize = stream->getLength();
    const auto t0 = SkTime::GetMSecs();

    auto data = SkData::MakeFromStream(stream, stream->getLength());
    if (!data) {
        SkDebugf("!! Failed to read the input stream.\n");
        return nullptr;
    }

    const skjson::DOM dom(static_cast<const char*>(data->data()), data->size());
    if (!dom.root().is<skjson::ObjectValue>()) {
        // TODO: more error info.
        SkDebugf("!! Failed to parse JSON input.\n");
        return nullptr;
    }
    const auto& json = dom.root().as<skjson::ObjectValue>();

    const auto t1 = SkTime::GetMSecs();
    stats->fJsonParseTimeMS = t1 - t0;

    const auto version = ParseDefault<SkString>(json["v"], SkString());
    const auto size    = SkSize::Make(ParseDefault<float>(json["w"], 0.0f),
                                      ParseDefault<float>(json["h"], 0.0f));
    const auto fps     = ParseDefault<float>(json["fr"], -1.0f);

    if (size.isEmpty() || version.isEmpty() || fps <= 0) {
        LOG("!! invalid animation params (version: %s, size: [%f %f], frame rate: %f)",
            version.c_str(), size.width(), size.height(), fps);
        return nullptr;
    }

    class NullResourceProvider final : public ResourceProvider {
        std::unique_ptr<SkStream> openStream(const char[]) const { return nullptr; }
    };

    NullResourceProvider null_provider;
    const auto anim = sk_sp<Animation>(new Animation(provider ? *provider : null_provider,
                                                     std::move(version), size, fps, json, stats));
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

        std::unique_ptr<SkStream> openStream(const char resource[]) const override {
            const auto resPath = SkOSPath::Join(fDir.c_str(), resource);
            return SkStream::MakeFromFile(resPath.c_str());
        }

    private:
        const SkString fDir;
    };

    const auto jsonStream =  SkStream::MakeFromFile(path);
    if (!jsonStream)
        return nullptr;

    std::unique_ptr<ResourceProvider> defaultProvider;
    if (!res) {
        defaultProvider = skstd::make_unique<DirectoryResourceProvider>(SkOSPath::Dirname(path));
    }

    return Make(jsonStream.get(), res ? res : defaultProvider.get(), stats);
}

Animation::Animation(const ResourceProvider& resources,
                     SkString version, const SkSize& size, SkScalar fps,
                     const skjson::ObjectValue& json, Stats* stats)
    : fVersion(std::move(version))
    , fSize(size)
    , fFrameRate(fps)
    , fInPoint(ParseDefault<float>(json["ip"], 0.0f))
    , fOutPoint(SkTMax(ParseDefault<float>(json["op"], SK_ScalarMax), fInPoint)) {

    AssetMap assets;
    if (const skjson::ArrayValue* jassets = json["assets"]) {
        for (const skjson::ObjectValue* asset : *jassets) {
            if (asset) {
                assets.set(ParseDefault<SkString>((*asset)["id"], SkString()), { asset, false });
            }
        }
    }

    sksg::AnimatorList animators;
    AttachContext ctx = { resources, assets, this->duration(), fFrameRate, animators };
    auto root = AttachComposition(json, &ctx);

    stats->fAnimatorCount = animators.size();

    fScene = sksg::Scene::Make(std::move(root), std::move(animators));

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
