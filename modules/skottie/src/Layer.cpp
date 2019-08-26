/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottiePriv.h"

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/skottie/src/effects/MotionBlurEffect.h"
#include "modules/sksg/include/SkSGClipEffect.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGMaskEffect.h"
#include "modules/sksg/include/SkSGMerge.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

namespace  {

static constexpr int kNullLayerType   =  3;
static constexpr int kCameraLayerType = 13;

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
                                   const AnimationBuilder* abuilder,
                                   sk_sp<sksg::RenderNode> childNode) {
    if (!jmask) return childNode;

    struct MaskRecord {
        sk_sp<sksg::Path>            mask_path;  // for clipping and masking
        sk_sp<sksg::Color>           mask_paint; // for masking
        sk_sp<sksg::BlurImageFilter> mask_blur;  // for masking
        sksg::Merge::Mode            merge_mode; // for clipping
    };

    SkSTArray<4, MaskRecord, true> mask_stack;

    bool has_effect = false;
    auto blur_effect = sksg::BlurImageFilter::Make();

    for (const skjson::ObjectValue* m : *jmask) {
        if (!m) continue;

        const skjson::StringValue* jmode = (*m)["mode"];
        if (!jmode || jmode->size() != 1) {
            abuilder->log(Logger::Level::kError, &(*m)["mode"], "Invalid mask mode.");
            continue;
        }

        const auto mode = *jmode->begin();
        if (mode == 'n') {
            // "None" masks have no effect.
            continue;
        }

        const auto* mask_info = GetMaskInfo(mode);
        if (!mask_info) {
            abuilder->log(Logger::Level::kWarning, nullptr, "Unsupported mask mode: '%c'.", mode);
            continue;
        }

        auto mask_path = abuilder->attachPath((*m)["pt"]);
        if (!mask_path) {
            abuilder->log(Logger::Level::kError, m, "Could not parse mask path.");
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

        has_effect |= abuilder->bindProperty<ScalarValue>((*m)["o"],
            [mask_paint](const ScalarValue& o) {
                mask_paint->setOpacity(o * 0.01f);
        }, 100.0f);

        static const VectorValue default_feather = { 0, 0 };
        if (abuilder->bindProperty<VectorValue>((*m)["f"],
            [blur_effect](const VectorValue& feather) {
                // Close enough to AE.
                static constexpr SkScalar kFeatherToSigma = 0.38f;
                auto sX = feather.size() > 0 ? feather[0] * kFeatherToSigma : 0,
                     sY = feather.size() > 1 ? feather[1] * kFeatherToSigma : 0;
                blur_effect->setSigma({ sX, sY });
            }, default_feather)) {

            has_effect = true;
            mask_stack.push_back({ mask_path,
                                   mask_paint,
                                   std::move(blur_effect),
                                   mask_info->fMergeMode});
            blur_effect = sksg::BlurImageFilter::Make();
        } else {
            mask_stack.push_back({mask_path, mask_paint, nullptr, mask_info->fMergeMode});
        }
    }

    if (mask_stack.empty())
        return childNode;

    // If the masks are fully opaque, we can clip.
    if (!has_effect) {
        sk_sp<sksg::GeometryNode> clip_node;

        if (mask_stack.count() == 1) {
            // Single path -> just clip.
            clip_node = std::move(mask_stack.front().mask_path);
        } else {
            // Multiple clip paths -> merge.
            std::vector<sksg::Merge::Rec> merge_recs;
            merge_recs.reserve(SkToSizeT(mask_stack.count()));

            for (auto& mask : mask_stack) {
                const auto mode = merge_recs.empty() ? sksg::Merge::Mode::kMerge : mask.merge_mode;
                merge_recs.push_back({std::move(mask.mask_path), mode});
            }
            clip_node = sksg::Merge::Make(std::move(merge_recs));
        }

        return sksg::ClipEffect::Make(std::move(childNode), std::move(clip_node), true);
    }

    const auto make_mask = [](const MaskRecord& rec) {
        auto mask = sksg::Draw::Make(std::move(rec.mask_path),
                                     std::move(rec.mask_paint));
        // Optional mask blur (feather).
        return sksg::ImageFilterEffect::Make(std::move(mask), std::move(rec.mask_blur));
    };

    sk_sp<sksg::RenderNode> maskNode;
    if (mask_stack.count() == 1) {
        // no group needed for single mask
        maskNode = make_mask(mask_stack.front());
    } else {
        std::vector<sk_sp<sksg::RenderNode>> masks;
        masks.reserve(SkToSizeT(mask_stack.count()));
        for (auto& rec : mask_stack) {
            masks.push_back(make_mask(rec));
        }

        maskNode = sksg::Group::Make(std::move(masks));
    }

    return sksg::MaskEffect::Make(std::move(childNode), std::move(maskNode));
}

class LayerController final : public sksg::Animator {
public:
    LayerController(sksg::AnimatorList&& layer_animators,
                    sk_sp<sksg::RenderNode> layer,
                    size_t tanim_count, float in, float out)
        : fLayerAnimators(std::move(layer_animators))
        , fLayerNode(std::move(layer))
        , fTransformAnimatorsCount(tanim_count)
        , fIn(in)
        , fOut(out) {}

protected:
    void onTick(float t) override {
        const auto active = (t >= fIn && t < fOut);

        if (fLayerNode) {
            fLayerNode->setVisible(active);
        }

        // When active, dispatch ticks to all layer animators.
        // When inactive, we must still dispatch ticks to the layer transform animators
        // (active child layers depend on transforms being updated).
        const auto dispatch_count = active ? fLayerAnimators.size()
                                           : fTransformAnimatorsCount;
        for (size_t i = 0; i < dispatch_count; ++i) {
            fLayerAnimators[i]->tick(t);
        }
    }

private:
    const sksg::AnimatorList      fLayerAnimators;
    const sk_sp<sksg::RenderNode> fLayerNode;
    const size_t                  fTransformAnimatorsCount;
    const float                   fIn,
                                  fOut;
};

class MotionBlurController final : public sksg::Animator {
public:
    explicit MotionBlurController(sk_sp<MotionBlurEffect> mbe)
        : fMotionBlurEffect(std::move(mbe)) {}

protected:
    // When motion blur is present, time ticks are not passed to layer animators
    // but to the motion blur effect. The effect then drives the animators/scene-graph
    // during reval and render phases.
    void onTick(float t) override {
        fMotionBlurEffect->setT(t);
    }

private:
    const sk_sp<MotionBlurEffect> fMotionBlurEffect;
};

} // namespace

AnimationBuilder::AttachLayerContext::AttachLayerContext(const skjson::ArrayValue& jlayers)
    : fLayerList(jlayers) {}

AnimationBuilder::AttachLayerContext::~AttachLayerContext() = default;

AnimationBuilder::AttachLayerContext::TransformRec
AnimationBuilder::AttachLayerContext::attachLayerTransform(const skjson::ObjectValue& jlayer,
                                                           const AnimationBuilder* abuilder,
                                                           TransformType type) {
    TransformRec result;

    const auto layer_index = ParseDefault<int>(jlayer["ind"], -1);
    if (layer_index >= 0) {
        auto* rec = fLayerTransformMap.find(layer_index);
        if (!rec) {
            rec = this->attachLayerTransformImpl(jlayer, abuilder, type, layer_index);
        }
        SkASSERT(rec);

        // Note: the transform animator scope is *moved* to the result, because
        // we want the animators transferred to the LayerController.
        //
        // This is safe because a) the scope is not used internally, and
        // b) there is exactly one attachLayerTransform call per layer.
        // The transform node OTOH may be used at a later time for parenting.
        result.fTransformNode = rec->fTransformNode;
        result.fTransformScope = std::move(rec->fTransformScope);
    }

    return result;
}

sk_sp<sksg::Transform>
AnimationBuilder::AttachLayerContext::attachParentLayerTransform(const skjson::ObjectValue& jlayer,
                                                                 const AnimationBuilder* abuilder,
                                                                 int layer_index) {
    const auto parent_index = ParseDefault<int>(jlayer["parent"], -1);
    if (parent_index < 0 || parent_index == layer_index)
        return nullptr;

    if (const auto* rec = fLayerTransformMap.find(parent_index))
        return rec->fTransformNode;

    for (const skjson::ObjectValue* l : fLayerList) {
        if (!l) continue;

        if (ParseDefault<int>((*l)["ind"], -1) == parent_index) {
            const auto parent_type = ParseDefault<int>((*l)["ty"], -1) == kCameraLayerType
                    ? TransformType::kCamera
                    : TransformType::kLayer;
            return this->attachLayerTransformImpl(*l,
                                                  abuilder,
                                                  parent_type,
                                                  parent_index)->fTransformNode;
        }
    }

    return nullptr;
}

sk_sp<sksg::Transform>
AnimationBuilder::AttachLayerContext::attachTransformNode(const skjson::ObjectValue& jlayer,
                                                          const AnimationBuilder* abuilder,
                                                          sk_sp<sksg::Transform> parent_transform,
                                                          TransformType type) const {
    const skjson::ObjectValue* jtransform = jlayer["ks"];
    if (!jtransform) {
        return nullptr;
    }

    if (type == TransformType::kCamera) {
        auto camera_adapter = sk_make_sp<CameraAdapter>(abuilder->fSize);

        abuilder->bindProperty<ScalarValue>(jlayer["pe"],
            [camera_adapter] (const ScalarValue& pe) {
                // 'pe' (perspective?) corresponds to AE's "zoom" camera property.
                camera_adapter->setZoom(pe);
            });

        // parent_transform applies to the camera itself => it pre-composes inverted to the
        // camera/view/adapter transform.
        //
        //   T_camera' = T_camera x Inv(parent_transform)
        //
        parent_transform = sksg::Transform::MakeInverse(std::move(parent_transform));

        return abuilder->attachMatrix3D(*jtransform,
                                        std::move(parent_transform),
                                        std::move(camera_adapter),
                                        true); // pre-compose parent
    }

    return (ParseDefault<int>(jlayer["ddd"], 0) == 0)
            ? abuilder->attachMatrix2D(*jtransform, std::move(parent_transform))
            : abuilder->attachMatrix3D(*jtransform, std::move(parent_transform));
}

AnimationBuilder::AttachLayerContext::TransformRec*
AnimationBuilder::AttachLayerContext::attachLayerTransformImpl(const skjson::ObjectValue& jlayer,
                                                               const AnimationBuilder* abuilder,
                                                               TransformType type,
                                                               int layer_index) {
    SkASSERT(!fLayerTransformMap.find(layer_index));

    // Add a stub entry to break recursion cycles.
    fLayerTransformMap.set(layer_index, { nullptr, {} });

    auto parent_matrix = this->attachParentLayerTransform(jlayer, abuilder, layer_index);

    AutoScope ascope(abuilder);
    auto transform = this->attachTransformNode(jlayer,
                                               abuilder,
                                               std::move(parent_matrix),
                                               type);

    return fLayerTransformMap.set(layer_index, { std::move(transform), ascope.release() });
}

bool AnimationBuilder::AttachLayerContext::hasMotionBlur(const skjson::ObjectValue& jlayer) const {
    return fMotionBlurSamples > 1
        && fMotionBlurAngle   > 0
        && ParseDefault(jlayer["mb"], false);
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachLayer(const skjson::ObjectValue* jlayer,
                                                      AttachLayerContext* layerCtx) const {
    if (!jlayer) {
        return nullptr;
    }

    LayerInfo layer_info = {
        fSize,
        ParseDefault<float>((*jlayer)["ip"], 0.0f),
        ParseDefault<float>((*jlayer)["op"], 0.0f),
    };
    if (layer_info.fInPoint >= layer_info.fOutPoint) {
        this->log(Logger::Level::kError, nullptr,
                  "Invalid layer in/out points: %f/%f.", layer_info.fInPoint, layer_info.fOutPoint);
        return nullptr;
    }

    const AutoPropertyTracker apt(this, *jlayer);

    using LayerBuilder = sk_sp<sksg::RenderNode> (AnimationBuilder::*)(const skjson::ObjectValue&,
                                                                       LayerInfo*) const;

    // AE is annoyingly inconsistent in how effects interact with layer transforms: depending on
    // the layer type, effects are applied before or after the content is transformed.
    //
    // Empirically, pre-rendered layers (for some loose meaning of "pre-rendered") are in the
    // former category (effects are subject to transformation), while the remaining types are in
    // the latter.
    enum : uint32_t {
        kTransformEffects = 1, // The layer transform also applies to its effects.
    };

    static constexpr struct {
        LayerBuilder                      fBuilder;
        uint32_t                          fFlags;
    } gLayerBuildInfo[] = {
        { &AnimationBuilder::attachPrecompLayer, kTransformEffects },  // 'ty': 0 -> precomp
        { &AnimationBuilder::attachSolidLayer  , kTransformEffects },  // 'ty': 1 -> solid
        { &AnimationBuilder::attachImageLayer  , kTransformEffects },  // 'ty': 2 -> image
        { &AnimationBuilder::attachNullLayer   ,                 0 },  // 'ty': 3 -> null
        { &AnimationBuilder::attachShapeLayer  ,                 0 },  // 'ty': 4 -> shape
        { &AnimationBuilder::attachTextLayer   ,                 0 },  // 'ty': 5 -> text
    };

    const auto type = ParseDefault<int>((*jlayer)["ty"], -1);
    if ((type < 0) ||
        (type >= SkTo<int>(SK_ARRAY_COUNT(gLayerBuildInfo)) && type != kCameraLayerType)) {
        return nullptr;
    }

    // Optional layer transform.
    const auto transform_type = (type == kCameraLayerType)
            ? AttachLayerContext::TransformType::kCamera
            : AttachLayerContext::TransformType::kLayer;
    auto layer_transform_rec = layerCtx->attachLayerTransform(*jlayer, this, transform_type);

    if (type == kCameraLayerType) {
        // Camera layers are special: they don't build normal SG fragments, but drive a root-level
        // transform.
        if (layerCtx->fCameraTransform) {
            this->log(Logger::Level::kWarning, jlayer, "Ignoring duplicate camera layer.");
            return nullptr;
        }

        layerCtx->fCameraTransform = layer_transform_rec.fTransformNode;
    }

    AutoScope ascope(this, std::move(layer_transform_rec.fTransformScope));
    const auto transform_animator_count = fCurrentAnimatorScope->size();

    const auto is_hidden = ParseDefault<bool>((*jlayer)["hd"], false) || type == kCameraLayerType;
    const auto& build_info = gLayerBuildInfo[is_hidden ? kNullLayerType : type];

    // Build the layer content fragment.
    auto layer = (this->*(build_info.fBuilder))(*jlayer, &layer_info);

    // Clip layers with explicit dimensions.
    float w = 0, h = 0;
    if (Parse<float>((*jlayer)["w"], &w) && Parse<float>((*jlayer)["h"], &h)) {
        layer = sksg::ClipEffect::Make(std::move(layer),
                                       sksg::Rect::Make(SkRect::MakeWH(w, h)),
                                       true);
    }

    // Optional layer mask.
    layer = AttachMask((*jlayer)["masksProperties"], this, std::move(layer));

    // Does the transform apply to effects also?
    // (AE quirk: it doesn't - except for solid layers)
    const auto transform_effects = (build_info.fFlags & kTransformEffects);

    // Attach the transform before effects, when needed.
    if (layer_transform_rec.fTransformNode && !transform_effects) {
        layer = sksg::TransformEffect::Make(std::move(layer), layer_transform_rec.fTransformNode);
    }

    // Optional layer effects.
    if (const skjson::ArrayValue* jeffects = (*jlayer)["ef"]) {
        layer = EffectBuilder(this, layer_info.fSize).attachEffects(*jeffects, std::move(layer));
    }

    // Attach the transform after effects, when needed.
    if (layer_transform_rec.fTransformNode && transform_effects) {
        layer = sksg::TransformEffect::Make(std::move(layer),
                                            std::move(layer_transform_rec.fTransformNode));
    }

    // Optional layer opacity.
    // TODO: de-dupe this "ks" lookup with matrix above.
    if (const skjson::ObjectValue* jtransform = (*jlayer)["ks"]) {
        layer = this->attachOpacity(*jtransform, std::move(layer));
    }

    // Optional blend mode.
    layer = this->attachBlendMode(*jlayer, std::move(layer));

    const auto has_animators = !fCurrentAnimatorScope->empty();

    sk_sp<sksg::Animator> controller = sk_make_sp<LayerController>(ascope.release(),
                                                                   layer,
                                                                   transform_animator_count,
                                                                   layer_info.fInPoint,
                                                                   layer_info.fOutPoint);

    // Optional motion blur.
    if (layer && has_animators && layerCtx->hasMotionBlur(*jlayer)) {
        SkASSERT(layerCtx->fMotionBlurAngle >= 0);

        // Wrap both the layer node and the controller.
        auto motion_blur = MotionBlurEffect::Make(std::move(controller), std::move(layer),
                                                  layerCtx->fMotionBlurSamples,
                                                  layerCtx->fMotionBlurAngle,
                                                  layerCtx->fMotionBlurPhase);
        controller = sk_make_sp<MotionBlurController>(motion_blur);
        layer = std::move(motion_blur);
    }

    fCurrentAnimatorScope->push_back(std::move(controller));

    if (!layer) {
        return nullptr;
    }

    if (ParseDefault<bool>((*jlayer)["td"], false)) {
        // This layer is a matte.  We apply it as a mask to the next layer.
        layerCtx->fCurrentMatte = std::move(layer);
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
            return sksg::MaskEffect::Make(std::move(layer),
                                          std::move(layerCtx->fCurrentMatte),
                                          gMaskModes[matteType]);
        }
        layerCtx->fCurrentMatte.reset();
    }

    return layer;
}

} // namespace internal
} // namespace skottie
