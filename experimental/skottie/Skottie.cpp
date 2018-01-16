/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Skottie.h"

#include "SkCanvas.h"
#include "SkottieAnimator.h"
#include "SkottiePriv.h"
#include "SkottieProperties.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkMakeUnique.h"
#include "SkOSPath.h"
#include "SkPaint.h"
#include "SkParse.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGradient.h"
#include "SkSGGroup.h"
#include "SkSGImage.h"
#include "SkSGInvalidationController.h"
#include "SkSGMaskEffect.h"
#include "SkSGMerge.h"
#include "SkSGOpacityEffect.h"
#include "SkSGPath.h"
#include "SkSGRect.h"
#include "SkSGTransform.h"
#include "SkSGTrimEffect.h"
#include "SkStream.h"
#include "SkTArray.h"
#include "SkTHash.h"

#include <cmath>
#include <unordered_map>
#include <vector>

#include "stdlib.h"

namespace skottie {

namespace {

using AssetMap = SkTHashMap<SkString, const Json::Value*>;

struct AttachContext {
    const ResourceProvider&                  fResources;
    const AssetMap&                          fAssets;
    SkTArray<std::unique_ptr<AnimatorBase>>& fAnimators;
};

bool LogFail(const Json::Value& json, const char* msg) {
    const auto dump = json.toStyledString();
    LOG("!! %s: %s", msg, dump.c_str());
    return false;
}

// This is the workhorse for binding properties: depending on whether the property is animated,
// it will either apply immediately or instantiate and attach a keyframe animator.
template <typename ValT, typename NodeT>
bool BindProperty(const Json::Value& jprop, AttachContext* ctx, const sk_sp<NodeT>& node,
                  typename Animator<ValT, NodeT>::ApplyFuncT&& apply) {
    if (!jprop.isObject())
        return false;

    const auto& jpropA = jprop["a"];
    const auto& jpropK = jprop["k"];

    // Older Json versions don't have an "a" animation marker.
    // For those, we attempt to parse both ways.
    if (jpropA.isNull() || !ParseBool(jpropA, "false")) {
        ValT val;
        if (ValueTraits<ValT>::Parse(jpropK, &val)) {
            // Static property.
            apply(node.get(), val);
            return true;
        }

        if (!jpropA.isNull()) {
            return LogFail(jprop, "Could not parse (explicit) static property");
        }
    }

    // Keyframe property.
    using AnimatorT = Animator<ValT, NodeT>;
    auto animator = AnimatorT::Make(ParseFrames<ValT>(jpropK), node, std::move(apply));

    if (!animator) {
        return LogFail(jprop, "Could not parse keyframed property");
    }

    ctx->fAnimators.push_back(std::move(animator));

    return true;
}

sk_sp<sksg::Matrix> AttachMatrix(const Json::Value& t, AttachContext* ctx,
                                        sk_sp<sksg::Matrix> parentMatrix) {
    if (!t.isObject())
        return nullptr;

    auto matrix = sksg::Matrix::Make(SkMatrix::I(), std::move(parentMatrix));
    auto composite = sk_make_sp<CompositeTransform>(matrix);
    auto anchor_attached = BindProperty<VectorValue>(t["a"], ctx, composite,
            [](CompositeTransform* node, const VectorValue& a) {
                node->setAnchorPoint(ValueTraits<VectorValue>::As<SkPoint>(a));
            });
    auto position_attached = BindProperty<VectorValue>(t["p"], ctx, composite,
            [](CompositeTransform* node, const VectorValue& p) {
                node->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
            });
    auto scale_attached = BindProperty<VectorValue>(t["s"], ctx, composite,
            [](CompositeTransform* node, const VectorValue& s) {
                node->setScale(ValueTraits<VectorValue>::As<SkVector>(s));
            });
    auto rotation_attached = BindProperty<ScalarValue>(t["r"], ctx, composite,
            [](CompositeTransform* node, const ScalarValue& r) {
                node->setRotation(r);
            });
    auto skew_attached = BindProperty<ScalarValue>(t["sk"], ctx, composite,
            [](CompositeTransform* node, const ScalarValue& sk) {
                node->setSkew(sk);
            });
    auto skewaxis_attached = BindProperty<ScalarValue>(t["sa"], ctx, composite,
            [](CompositeTransform* node, const ScalarValue& sa) {
                node->setSkewAxis(sa);
            });

    if (!anchor_attached &&
        !position_attached &&
        !scale_attached &&
        !rotation_attached &&
        !skew_attached &&
        !skewaxis_attached) {
        LogFail(t, "Could not parse transform");
        return nullptr;
    }

    return matrix;
}

sk_sp<sksg::RenderNode> AttachOpacity(const Json::Value& jtransform, AttachContext* ctx,
                                      sk_sp<sksg::RenderNode> childNode) {
    if (!jtransform.isObject() || !childNode)
        return childNode;

    // This is more peeky than other attachers, because we want to avoid redundant opacity
    // nodes for the extremely common case of static opaciy == 100.
    const auto& opacity = jtransform["o"];
    if (opacity.isObject() &&
        !ParseBool(opacity["a"], true) &&
        ParseScalar(opacity["k"], -1) == 100) {
        // Ignoring static full opacity.
        return childNode;
    }

    auto opacityNode = sksg::OpacityEffect::Make(childNode);
    BindProperty<ScalarValue>(opacity, ctx, opacityNode,
        [](sksg::OpacityEffect* node, const ScalarValue& o) {
            // BM opacity is [0..100]
            node->setOpacity(o * 0.01f);
        });

    return opacityNode;
}

sk_sp<sksg::RenderNode> AttachShape(const Json::Value&, AttachContext* ctx);
sk_sp<sksg::RenderNode> AttachComposition(const Json::Value&, AttachContext* ctx);

sk_sp<sksg::RenderNode> AttachShapeGroup(const Json::Value& jgroup, AttachContext* ctx) {
    SkASSERT(jgroup.isObject());

    return AttachShape(jgroup["it"], ctx);
}

sk_sp<sksg::GeometryNode> AttachPathGeometry(const Json::Value& jpath, AttachContext* ctx) {
    SkASSERT(jpath.isObject());

    auto path_node = sksg::Path::Make();
    auto path_attached = BindProperty<ShapeValue>(jpath["ks"], ctx, path_node,
        [](sksg::Path* node, const ShapeValue& p) { node->setPath(p); });

    if (path_attached)
        LOG("** Attached path geometry - verbs: %d\n", path_node->getPath().countVerbs());

    return path_attached ? path_node : nullptr;
}

sk_sp<sksg::GeometryNode> AttachRRectGeometry(const Json::Value& jrect, AttachContext* ctx) {
    SkASSERT(jrect.isObject());

    auto rect_node = sksg::RRect::Make();
    auto composite = sk_make_sp<CompositeRRect>(rect_node);

    auto p_attached = BindProperty<VectorValue>(jrect["p"], ctx, composite,
        [](CompositeRRect* node, const VectorValue& p) {
                node->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    auto s_attached = BindProperty<VectorValue>(jrect["s"], ctx, composite,
        [](CompositeRRect* node, const VectorValue& s) {
            node->setSize(ValueTraits<VectorValue>::As<SkSize>(s));
        });
    auto r_attached = BindProperty<ScalarValue>(jrect["r"], ctx, composite,
        [](CompositeRRect* node, const ScalarValue& r) {
            node->setRadius(SkSize::Make(r, r));
        });

    if (!p_attached && !s_attached && !r_attached) {
        return nullptr;
    }

    LOG("** Attached (r)rect geometry\n");

    return rect_node;
}

sk_sp<sksg::GeometryNode> AttachEllipseGeometry(const Json::Value& jellipse, AttachContext* ctx) {
    SkASSERT(jellipse.isObject());

    auto rect_node = sksg::RRect::Make();
    auto composite = sk_make_sp<CompositeRRect>(rect_node);

    auto p_attached = BindProperty<VectorValue>(jellipse["p"], ctx, composite,
        [](CompositeRRect* node, const VectorValue& p) {
            node->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    auto s_attached = BindProperty<VectorValue>(jellipse["s"], ctx, composite,
        [](CompositeRRect* node, const VectorValue& s) {
            const auto sz = ValueTraits<VectorValue>::As<SkSize>(s);
            node->setSize(sz);
            node->setRadius(SkSize::Make(sz.width() / 2, sz.height() / 2));
        });

    if (!p_attached && !s_attached) {
        return nullptr;
    }

    LOG("** Attached ellipse geometry\n");

    return rect_node;
}

sk_sp<sksg::GeometryNode> AttachPolystarGeometry(const Json::Value& jstar, AttachContext* ctx) {
    SkASSERT(jstar.isObject());

    static constexpr CompositePolyStar::Type gTypes[] = {
        CompositePolyStar::Type::kStar, // "sy": 1
        CompositePolyStar::Type::kPoly, // "sy": 2
    };

    const auto type = ParseInt(jstar["sy"], 0) - 1;
    if (type < 0 || type >= SkTo<int>(SK_ARRAY_COUNT(gTypes))) {
        LogFail(jstar, "Unknown polystar type");
        return nullptr;
    }

    auto path_node = sksg::Path::Make();
    auto composite = sk_make_sp<CompositePolyStar>(path_node, gTypes[type]);

    BindProperty<VectorValue>(jstar["p"], ctx, composite,
        [](CompositePolyStar* node, const VectorValue& p) {
            node->setPosition(ValueTraits<VectorValue>::As<SkPoint>(p));
        });
    BindProperty<ScalarValue>(jstar["pt"], ctx, composite,
        [](CompositePolyStar* node, const ScalarValue& pt) {
            node->setPointCount(pt);
        });
    BindProperty<ScalarValue>(jstar["ir"], ctx, composite,
        [](CompositePolyStar* node, const ScalarValue& ir) {
            node->setInnerRadius(ir);
        });
    BindProperty<ScalarValue>(jstar["or"], ctx, composite,
        [](CompositePolyStar* node, const ScalarValue& otr) {
            node->setOuterRadius(otr);
        });
    BindProperty<ScalarValue>(jstar["is"], ctx, composite,
        [](CompositePolyStar* node, const ScalarValue& is) {
            node->setInnerRoundness(is);
        });
    BindProperty<ScalarValue>(jstar["os"], ctx, composite,
        [](CompositePolyStar* node, const ScalarValue& os) {
            node->setOuterRoundness(os);
        });
    BindProperty<ScalarValue>(jstar["r"], ctx, composite,
        [](CompositePolyStar* node, const ScalarValue& r) {
            node->setRotation(r);
        });

    return path_node;
}

sk_sp<sksg::Color> AttachColor(const Json::Value& obj, AttachContext* ctx) {
    SkASSERT(obj.isObject());

    auto color_node = sksg::Color::Make(SK_ColorBLACK);
    auto color_attached = BindProperty<VectorValue>(obj["c"], ctx, color_node,
        [](sksg::Color* node, const VectorValue& c) {
            node->setColor(ValueTraits<VectorValue>::As<SkColor>(c));
        });

    return color_attached ? color_node : nullptr;
}

sk_sp<sksg::Gradient> AttachGradient(const Json::Value& obj, AttachContext* ctx) {
    SkASSERT(obj.isObject());

    const auto& stops = obj["g"];
    if (!stops.isObject())
        return nullptr;

    const auto stopCount = ParseInt(stops["p"], -1);
    if (stopCount < 0)
        return nullptr;

    sk_sp<sksg::Gradient> gradient_node;
    sk_sp<CompositeGradient> composite;

    if (ParseInt(obj["t"], 1) == 1) {
        auto linear_node = sksg::LinearGradient::Make();
        composite = sk_make_sp<CompositeLinearGradient>(linear_node, stopCount);
        gradient_node = std::move(linear_node);
    } else {
        auto radial_node = sksg::RadialGradient::Make();
        composite = sk_make_sp<CompositeRadialGradient>(radial_node, stopCount);

        // TODO: highlight, angle
        gradient_node = std::move(radial_node);
    }

    BindProperty<VectorValue>(stops["k"], ctx, composite,
        [](CompositeGradient* node, const VectorValue& stops) {
            node->setColorStops(stops);
        });
    BindProperty<VectorValue>(obj["s"], ctx, composite,
        [](CompositeGradient* node, const VectorValue& s) {
            node->setStartPoint(ValueTraits<VectorValue>::As<SkPoint>(s));
        });
    BindProperty<VectorValue>(obj["e"], ctx, composite,
        [](CompositeGradient* node, const VectorValue& e) {
            node->setEndPoint(ValueTraits<VectorValue>::As<SkPoint>(e));
        });

    return gradient_node;
}

sk_sp<sksg::PaintNode> AttachPaint(const Json::Value& jpaint, AttachContext* ctx,
                                   sk_sp<sksg::PaintNode> paint_node) {
    if (paint_node) {
        paint_node->setAntiAlias(true);

        BindProperty<ScalarValue>(jpaint["o"], ctx, paint_node,
            [](sksg::PaintNode* node, const ScalarValue& o) {
                // BM opacity is [0..100]
                node->setOpacity(o * 0.01f);
        });
    }

    return paint_node;
}

sk_sp<sksg::PaintNode> AttachStroke(const Json::Value& jstroke, AttachContext* ctx,
                                    sk_sp<sksg::PaintNode> stroke_node) {
    SkASSERT(jstroke.isObject());

    if (!stroke_node)
        return nullptr;

    stroke_node->setStyle(SkPaint::kStroke_Style);

    auto width_attached = BindProperty<ScalarValue>(jstroke["w"], ctx, stroke_node,
        [](sksg::PaintNode* node, const ScalarValue& w) {
            node->setStrokeWidth(w);
        });
    if (!width_attached)
        return nullptr;

    stroke_node->setStrokeMiter(ParseScalar(jstroke["ml"], 4));

    static constexpr SkPaint::Join gJoins[] = {
        SkPaint::kMiter_Join,
        SkPaint::kRound_Join,
        SkPaint::kBevel_Join,
    };
    stroke_node->setStrokeJoin(gJoins[SkTPin<int>(ParseInt(jstroke["lj"], 1) - 1,
                                                  0, SK_ARRAY_COUNT(gJoins) - 1)]);

    static constexpr SkPaint::Cap gCaps[] = {
        SkPaint::kButt_Cap,
        SkPaint::kRound_Cap,
        SkPaint::kSquare_Cap,
    };
    stroke_node->setStrokeCap(gCaps[SkTPin<int>(ParseInt(jstroke["lc"], 1) - 1,
                                                0, SK_ARRAY_COUNT(gCaps) - 1)]);

    return stroke_node;
}

sk_sp<sksg::PaintNode> AttachColorFill(const Json::Value& jfill, AttachContext* ctx) {
    SkASSERT(jfill.isObject());

    return AttachPaint(jfill, ctx, AttachColor(jfill, ctx));
}

sk_sp<sksg::PaintNode> AttachGradientFill(const Json::Value& jfill, AttachContext* ctx) {
    SkASSERT(jfill.isObject());

    return AttachPaint(jfill, ctx, AttachGradient(jfill, ctx));
}

sk_sp<sksg::PaintNode> AttachColorStroke(const Json::Value& jstroke, AttachContext* ctx) {
    SkASSERT(jstroke.isObject());

    return AttachStroke(jstroke, ctx, AttachPaint(jstroke, ctx, AttachColor(jstroke, ctx)));
}

sk_sp<sksg::PaintNode> AttachGradientStroke(const Json::Value& jstroke, AttachContext* ctx) {
    SkASSERT(jstroke.isObject());

    return AttachStroke(jstroke, ctx, AttachPaint(jstroke, ctx, AttachGradient(jstroke, ctx)));
}

std::vector<sk_sp<sksg::GeometryNode>> AttachMergeGeometryEffect(
    const Json::Value& jmerge, AttachContext* ctx, std::vector<sk_sp<sksg::GeometryNode>>&& geos) {
    std::vector<sk_sp<sksg::GeometryNode>> merged;

    static constexpr sksg::Merge::Mode gModes[] = {
        sksg::Merge::Mode::kMerge,      // "mm": 1
        sksg::Merge::Mode::kUnion,      // "mm": 2
        sksg::Merge::Mode::kDifference, // "mm": 3
        sksg::Merge::Mode::kIntersect,  // "mm": 4
        sksg::Merge::Mode::kXOR      ,  // "mm": 5
    };

    const auto mode = gModes[SkTPin<int>(ParseInt(jmerge["mm"], 1) - 1,
                                         0, SK_ARRAY_COUNT(gModes) - 1)];
    merged.push_back(sksg::Merge::Make(std::move(geos), mode));

    LOG("** Attached merge path effect, mode: %d\n", mode);

    return merged;
}

std::vector<sk_sp<sksg::GeometryNode>> AttachTrimGeometryEffect(
    const Json::Value& jtrim, AttachContext* ctx, std::vector<sk_sp<sksg::GeometryNode>>&& geos) {

    enum class Mode {
        kMerged,   // "m": 1
        kSeparate, // "m": 2
    } gModes[] = { Mode::kMerged, Mode::kSeparate };

    const auto mode = gModes[SkTPin<int>(ParseInt(jtrim["m"], 1) - 1,
                                         0, SK_ARRAY_COUNT(gModes) - 1)];

    std::vector<sk_sp<sksg::GeometryNode>> inputs;
    if (mode == Mode::kMerged) {
        inputs.push_back(sksg::Merge::Make(std::move(geos), sksg::Merge::Mode::kMerge));
    } else {
        inputs = std::move(geos);
    }

    std::vector<sk_sp<sksg::GeometryNode>> trimmed;
    trimmed.reserve(inputs.size());
    for (const auto& i : inputs) {
        const auto trim = sksg::TrimEffect::Make(i);
        trimmed.push_back(trim);
        BindProperty<ScalarValue>(jtrim["s"], ctx, trim,
            [](sksg::TrimEffect* node, const ScalarValue& s) {
                node->setStart(s * 0.01f);
            });
        BindProperty<ScalarValue>(jtrim["e"], ctx, trim,
            [](sksg::TrimEffect* node, const ScalarValue& e) {
                node->setEnd(e * 0.01f);
            });
        // TODO: "offset" doesn't currently work the same as BM - figure out what's going on.
        BindProperty<ScalarValue>(jtrim["o"], ctx, trim,
            [](sksg::TrimEffect* node, const ScalarValue& o) {
                node->setOffset(o * 0.01f);
            });
    }

    return trimmed;
}

using GeometryAttacherT = sk_sp<sksg::GeometryNode> (*)(const Json::Value&, AttachContext*);
static constexpr GeometryAttacherT gGeometryAttachers[] = {
    AttachPathGeometry,
    AttachRRectGeometry,
    AttachEllipseGeometry,
    AttachPolystarGeometry,
};

using PaintAttacherT = sk_sp<sksg::PaintNode> (*)(const Json::Value&, AttachContext*);
static constexpr PaintAttacherT gPaintAttachers[] = {
    AttachColorFill,
    AttachColorStroke,
    AttachGradientFill,
    AttachGradientStroke,
};

using GroupAttacherT = sk_sp<sksg::RenderNode> (*)(const Json::Value&, AttachContext*);
static constexpr GroupAttacherT gGroupAttachers[] = {
    AttachShapeGroup,
};

using GeometryEffectAttacherT =
    std::vector<sk_sp<sksg::GeometryNode>> (*)(const Json::Value&,
                                               AttachContext*,
                                               std::vector<sk_sp<sksg::GeometryNode>>&&);
static constexpr GeometryEffectAttacherT gGeometryEffectAttachers[] = {
    AttachMergeGeometryEffect,
    AttachTrimGeometryEffect,
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

const ShapeInfo* FindShapeInfo(const Json::Value& shape) {
    static constexpr ShapeInfo gShapeInfo[] = {
        { "el", ShapeType::kGeometry      , 2 }, // ellipse   -> AttachEllipseGeometry
        { "fl", ShapeType::kPaint         , 0 }, // fill      -> AttachColorFill
        { "gf", ShapeType::kPaint         , 2 }, // gfill     -> AttachGradientFill
        { "gr", ShapeType::kGroup         , 0 }, // group     -> AttachShapeGroup
        { "gs", ShapeType::kPaint         , 3 }, // gstroke   -> AttachGradientStroke
        { "mm", ShapeType::kGeometryEffect, 0 }, // merge     -> AttachMergeGeometryEffect
        { "rc", ShapeType::kGeometry      , 1 }, // rrect     -> AttachRRectGeometry
        { "sh", ShapeType::kGeometry      , 0 }, // shape     -> AttachPathGeometry
        { "sr", ShapeType::kGeometry      , 3 }, // polystar  -> AttachPolyStarGeometry
        { "st", ShapeType::kPaint         , 1 }, // stroke    -> AttachColorStroke
        { "tm", ShapeType::kGeometryEffect, 1 }, // trim      -> AttachTrimGeometryEffect
        { "tr", ShapeType::kTransform     , 0 }, // transform -> In-place handler
    };

    if (!shape.isObject())
        return nullptr;

    const auto& type = shape["ty"];
    if (!type.isString())
        return nullptr;

    const auto* info = bsearch(type.asCString(),
                               gShapeInfo,
                               SK_ARRAY_COUNT(gShapeInfo),
                               sizeof(ShapeInfo),
                               [](const void* key, const void* info) {
                                  return strcmp(static_cast<const char*>(key),
                                                static_cast<const ShapeInfo*>(info)->fTypeString);
                               });

    return static_cast<const ShapeInfo*>(info);
}

sk_sp<sksg::RenderNode> AttachShape(const Json::Value& shapeArray, AttachContext* ctx) {
    if (!shapeArray.isArray())
        return nullptr;

    // (https://helpx.adobe.com/after-effects/using/overview-shape-layers-paths-vector.html#groups_and_render_order_for_shapes_and_shape_attributes)
    //
    // Render order for shapes within a shape layer
    //
    // The rules for rendering a shape layer are similar to the rules for rendering a composition
    // that contains nested compositions:
    //
    //   * Within a group, the shape at the bottom of the Timeline panel stacking order is rendered
    //     first.
    //
    //   * All path operations within a group are performed before paint operations. This means,
    //     for example, that the stroke follows the distortions in the path made by the Wiggle Paths
    //     path operation. Path operations within a group are performed from top to bottom.
    //
    //   * Paint operations within a group are performed from the bottom to the top in the Timeline
    //     panel stacking order. This means, for example, that a stroke is rendered on top of
    //     (in front of) a stroke that appears after it in the Timeline panel.
    //
    sk_sp<sksg::Group>        shape_group = sksg::Group::Make();
    sk_sp<sksg::RenderNode> xformed_group = shape_group;

    std::vector<sk_sp<sksg::GeometryNode>> geos;
    std::vector<sk_sp<sksg::RenderNode>> draws;

    for (const auto& s : shapeArray) {
        const auto* info = FindShapeInfo(s);
        if (!info) {
            LogFail(s.isObject() ? s["ty"] : s, "Unknown shape");
            continue;
        }

        switch (info->fShapeType) {
        case ShapeType::kGeometry: {
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGeometryAttachers));
            if (auto geo = gGeometryAttachers[info->fAttacherIndex](s, ctx)) {
                geos.push_back(std::move(geo));
            }
        } break;
        case ShapeType::kGeometryEffect: {
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGeometryEffectAttachers));
            geos = gGeometryEffectAttachers[info->fAttacherIndex](s, ctx, std::move(geos));
        } break;
        case ShapeType::kPaint: {
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gPaintAttachers));
            if (auto paint = gPaintAttachers[info->fAttacherIndex](s, ctx)) {
                for (const auto& geo : geos) {
                    draws.push_back(sksg::Draw::Make(geo, paint));
                }
            }
        } break;
        case ShapeType::kGroup: {
            SkASSERT(info->fAttacherIndex < SK_ARRAY_COUNT(gGroupAttachers));
            if (auto group = gGroupAttachers[info->fAttacherIndex](s, ctx)) {
                draws.push_back(std::move(group));
            }
        } break;
        case ShapeType::kTransform: {
            // TODO: BM appears to transform the geometry, not the draw op itself.
            if (auto matrix = AttachMatrix(s, ctx, nullptr)) {
                xformed_group = sksg::Transform::Make(std::move(xformed_group),
                                                      std::move(matrix));
            }
            xformed_group = AttachOpacity(s, ctx, std::move(xformed_group));
        } break;
        }
    }

    if (draws.empty()) {
        return nullptr;
    }

    for (auto draw = draws.rbegin(); draw != draws.rend(); ++draw) {
        shape_group->addChild(std::move(*draw));
    }

    LOG("** Attached shape: %zd draws.\n", draws.size());
    return xformed_group;
}

sk_sp<sksg::RenderNode> AttachCompLayer(const Json::Value& layer, AttachContext* ctx) {
    SkASSERT(layer.isObject());

    auto refId = ParseString(layer["refId"], "");
    if (refId.isEmpty()) {
        LOG("!! Comp layer missing refId\n");
        return nullptr;
    }

    const auto* comp = ctx->fAssets.find(refId);
    if (!comp) {
        LOG("!! Pre-comp not found: '%s'\n", refId.c_str());
        return nullptr;
    }

    // TODO: cycle detection
    return AttachComposition(**comp, ctx);
}

sk_sp<sksg::RenderNode> AttachSolidLayer(const Json::Value& jlayer, AttachContext*) {
    SkASSERT(jlayer.isObject());

    const auto size = SkSize::Make(ParseScalar(jlayer["sw"], -1),
                                   ParseScalar(jlayer["sh"], -1));
    const auto hex = ParseString(jlayer["sc"], "");
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

sk_sp<sksg::RenderNode> AttachImageAsset(const Json::Value& jimage, AttachContext* ctx) {
    SkASSERT(jimage.isObject());

    const auto name = ParseString(jimage["p"], ""),
               path = ParseString(jimage["u"], "");
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

sk_sp<sksg::RenderNode> AttachImageLayer(const Json::Value& layer, AttachContext* ctx) {
    SkASSERT(layer.isObject());

    auto refId = ParseString(layer["refId"], "");
    if (refId.isEmpty()) {
        LOG("!! Image layer missing refId\n");
        return nullptr;
    }

    const auto* jimage = ctx->fAssets.find(refId);
    if (!jimage) {
        LOG("!! Image asset not found: '%s'\n", refId.c_str());
        return nullptr;
    }

    return AttachImageAsset(**jimage, ctx);
}

sk_sp<sksg::RenderNode> AttachNullLayer(const Json::Value& layer, AttachContext*) {
    SkASSERT(layer.isObject());

    // Null layers are used solely to drive dependent transforms,
    // but we use free-floating sksg::Matrices for that purpose.
    return nullptr;
}

sk_sp<sksg::RenderNode> AttachShapeLayer(const Json::Value& layer, AttachContext* ctx) {
    SkASSERT(layer.isObject());

    LOG("** Attaching shape layer ind: %d\n", ParseInt(layer["ind"], 0));

    return AttachShape(layer["shapes"], ctx);
}

sk_sp<sksg::RenderNode> AttachTextLayer(const Json::Value& layer, AttachContext*) {
    SkASSERT(layer.isObject());

    LOG("?? Text layer stub\n");
    return nullptr;
}

struct AttachLayerContext {
    AttachLayerContext(const Json::Value& jlayers, AttachContext* ctx)
        : fLayerList(jlayers), fCtx(ctx) {}

    const Json::Value&                                          fLayerList;
    AttachContext*                                              fCtx;
    std::unordered_map<const Json::Value*, sk_sp<sksg::Matrix>> fLayerMatrixCache;
    std::unordered_map<int, const Json::Value*>                 fLayerIndexCache;
    sk_sp<sksg::RenderNode>                                     fCurrentMatte;

    const Json::Value* findLayer(int index) {
        SkASSERT(fLayerList.isArray());

        if (index < 0) {
            return nullptr;
        }

        const auto cached = fLayerIndexCache.find(index);
        if (cached != fLayerIndexCache.end()) {
            return cached->second;
        }

        for (const auto& l : fLayerList) {
            if (!l.isObject()) {
                continue;
            }

            if (ParseInt(l["ind"], -1) == index) {
                fLayerIndexCache.insert(std::make_pair(index, &l));
                return &l;
            }
        }

        return nullptr;
    }

    sk_sp<sksg::Matrix> AttachLayerMatrix(const Json::Value& jlayer) {
        SkASSERT(jlayer.isObject());

        const auto cached = fLayerMatrixCache.find(&jlayer);
        if (cached != fLayerMatrixCache.end()) {
            return cached->second;
        }

        const auto* parentLayer = this->findLayer(ParseInt(jlayer["parent"], -1));

        // TODO: cycle detection?
        auto parentMatrix = (parentLayer && parentLayer != &jlayer)
            ? this->AttachLayerMatrix(*parentLayer) : nullptr;

        auto layerMatrix = AttachMatrix(jlayer["ks"], fCtx, std::move(parentMatrix));
        fLayerMatrixCache.insert(std::make_pair(&jlayer, layerMatrix));

        return layerMatrix;
    }
};

sk_sp<sksg::RenderNode> AttachLayer(const Json::Value& jlayer,
                                    AttachLayerContext* layerCtx) {
    if (!jlayer.isObject())
        return nullptr;

    using LayerAttacher = sk_sp<sksg::RenderNode> (*)(const Json::Value&, AttachContext*);
    static constexpr LayerAttacher gLayerAttachers[] = {
        AttachCompLayer,  // 'ty': 0
        AttachSolidLayer, // 'ty': 1
        AttachImageLayer, // 'ty': 2
        AttachNullLayer,  // 'ty': 3
        AttachShapeLayer, // 'ty': 4
        AttachTextLayer,  // 'ty': 5
    };

    int type = ParseInt(jlayer["ty"], -1);
    if (type < 0 || type >= SkTo<int>(SK_ARRAY_COUNT(gLayerAttachers))) {
        return nullptr;
    }

    // Layer content.
    auto layer = gLayerAttachers[type](jlayer, layerCtx->fCtx);
    if (auto layerMatrix = layerCtx->AttachLayerMatrix(jlayer)) {
        // Optional layer transform.
        layer = sksg::Transform::Make(std::move(layer), std::move(layerMatrix));
    }
    // Optional layer opacity.
    layer = AttachOpacity(jlayer["ks"], layerCtx->fCtx, std::move(layer));

    // TODO: we should also disable related/inactive animators.
    class Activator final : public AnimatorBase {
    public:
        Activator(sk_sp<sksg::OpacityEffect> controlNode, float in, float out)
            : fControlNode(std::move(controlNode))
            , fIn(in)
            , fOut(out) {}

        void tick(float t) override {
            // Keep the layer fully transparent except for its [in..out] lifespan.
            // (note: opacity == 0 disables rendering, while opacity == 1 is a noop)
            fControlNode->setOpacity(t >= fIn && t <= fOut ? 1 : 0);
        }

    private:
        const sk_sp<sksg::OpacityEffect> fControlNode;
        const float                      fIn,
                                         fOut;
    };

    auto layerControl = sksg::OpacityEffect::Make(std::move(layer));
    const auto  in = ParseScalar(jlayer["ip"], 0),
               out = ParseScalar(jlayer["op"], in);

    if (in >= out || ! layerControl)
        return nullptr;

    layerCtx->fCtx->fAnimators.push_back(skstd::make_unique<Activator>(layerControl, in, out));

    if (ParseBool(jlayer["td"], false)) {
        // This layer is a matte.  We apply it as a mask to the next layer.
        layerCtx->fCurrentMatte = std::move(layerControl);
        return nullptr;
    }

    if (layerCtx->fCurrentMatte) {
        // There is a pending matte. Apply and reset.
        return sksg::MaskEffect::Make(std::move(layerControl), std::move(layerCtx->fCurrentMatte));
    }

    return layerControl;
}

sk_sp<sksg::RenderNode> AttachComposition(const Json::Value& comp, AttachContext* ctx) {
    if (!comp.isObject())
        return nullptr;

    const auto& jlayers = comp["layers"];
    if (!jlayers.isArray())
        return nullptr;

    SkSTArray<16, sk_sp<sksg::RenderNode>, true> layers;
    AttachLayerContext                           layerCtx(jlayers, ctx);

    for (const auto& l : jlayers) {
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

    LOG("** Attached composition '%s': %d layers.\n",
        ParseString(comp["id"], "").c_str(), layers.count());

    return comp_group;
}

} // namespace

std::unique_ptr<Animation> Animation::Make(SkStream* stream, const ResourceProvider& res) {
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
        if (!reader.parse(dataStart, dataStart + data->size(), json, false) || !json.isObject()) {
            LOG("!! failed to parse json: %s\n", reader.getFormattedErrorMessages().c_str());
            return nullptr;
        }
    }

    const auto version = ParseString(json["v"], "");
    const auto size    = SkSize::Make(ParseScalar(json["w"], -1), ParseScalar(json["h"], -1));
    const auto fps     = ParseScalar(json["fr"], -1);

    if (size.isEmpty() || version.isEmpty() || fps < 0) {
        LOG("!! invalid animation params (version: %s, size: [%f %f], frame rate: %f)",
            version.c_str(), size.width(), size.height(), fps);
        return nullptr;
    }

    return std::unique_ptr<Animation>(new Animation(res, std::move(version), size, fps, json));
}

std::unique_ptr<Animation> Animation::MakeFromFile(const char path[], const ResourceProvider* res) {
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

    return Make(jsonStream.get(), res ? *res : *defaultProvider);
}

Animation::Animation(const ResourceProvider& resources,
                     SkString version, const SkSize& size, SkScalar fps, const Json::Value& json)
    : fVersion(std::move(version))
    , fSize(size)
    , fFrameRate(fps)
    , fInPoint(ParseScalar(json["ip"], 0))
    , fOutPoint(SkTMax(ParseScalar(json["op"], SK_ScalarMax), fInPoint)) {

    AssetMap assets;
    for (const auto& asset : json["assets"]) {
        if (!asset.isObject()) {
            continue;
        }

        assets.set(ParseString(asset["id"], ""), &asset);
    }

    AttachContext ctx = { resources, assets, fAnimators };
    fDom = AttachComposition(json, &ctx);

    // In case the client calls render before the first tick.
    this->animationTick(0);

    LOG("** Attached %d animators\n", fAnimators.count());
}

Animation::~Animation() = default;

void Animation::render(SkCanvas* canvas, const SkRect* dstR) const {
    if (!fDom)
        return;

    sksg::InvalidationController ic;
    fDom->revalidate(&ic, SkMatrix::I());

    // TODO: proper inval
    SkAutoCanvasRestore restore(canvas, true);
    const SkRect srcR = SkRect::MakeSize(this->size());
    if (dstR) {
        canvas->concat(SkMatrix::MakeRectToRect(srcR, *dstR, SkMatrix::kCenter_ScaleToFit));
    }
    canvas->clipRect(srcR);
    fDom->render(canvas);

    if (!fShowInval)
        return;

    SkPaint fill, stroke;
    fill.setAntiAlias(true);
    fill.setColor(0x40ff0000);
    stroke.setAntiAlias(true);
    stroke.setColor(0xffff0000);
    stroke.setStyle(SkPaint::kStroke_Style);

    for (const auto& r : ic) {
        canvas->drawRect(r, fill);
        canvas->drawRect(r, stroke);
    }
}

void Animation::animationTick(SkMSec ms) {
    // 't' in the BM model really means 'frame #'
    auto t = static_cast<float>(ms) * fFrameRate / 1000;

    t = fInPoint + std::fmod(t, fOutPoint - fInPoint);

    // TODO: this can be optimized quite a bit with some sorting/state tracking.
    for (const auto& a : fAnimators) {
        a->tick(t);
    }
}

} // namespace skottie
