/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Layer.h"

#include "modules/skottie/src/Camera.h"
#include "modules/skottie/src/Composition.h"
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

class MaskAdapter final : public AnimatablePropertyContainer {
public:
    MaskAdapter(const skjson::ObjectValue& jmask, const AnimationBuilder& abuilder, SkBlendMode bm)
        : fMaskPaint(sksg::Color::Make(SK_ColorBLACK)) {
        fMaskPaint->setAntiAlias(true);
        fMaskPaint->setBlendMode(bm);

        this->bind(abuilder, jmask["o"], fOpacity);

        if (this->bind(abuilder, jmask["f"], fFeather)) {
            fMaskFilter = sksg::BlurImageFilter::Make();
        }
    }

    bool hasEffect() const {
        return !this->isStatic()
            || fOpacity < 100
            || fFeather != SkV2{0,0};
    }

    sk_sp<sksg::RenderNode> makeMask(sk_sp<sksg::Path> mask_path) const {
        auto mask = sksg::Draw::Make(std::move(mask_path), fMaskPaint);

        // Optional mask blur (feather).
        return sksg::ImageFilterEffect::Make(std::move(mask), fMaskFilter);
    }

private:
    void onSync() override {
        fMaskPaint->setOpacity(fOpacity * 0.01f);
        if (fMaskFilter) {
            // Close enough to AE.
            static constexpr SkScalar kFeatherToSigma = 0.38f;
            fMaskFilter->setSigma({fFeather.x * kFeatherToSigma,
                                   fFeather.y * kFeatherToSigma});
        }
    }

    const sk_sp<sksg::PaintNode> fMaskPaint;
    sk_sp<sksg::BlurImageFilter> fMaskFilter; // optional "feather"

    Vec2Value   fFeather = {0,0};
    ScalarValue fOpacity = 100;
};

sk_sp<sksg::RenderNode> AttachMask(const skjson::ArrayValue* jmask,
                                   const AnimationBuilder* abuilder,
                                   sk_sp<sksg::RenderNode> childNode) {
    if (!jmask) return childNode;

    struct MaskRecord {
        sk_sp<sksg::Path>  mask_path;    // for clipping and masking
        sk_sp<MaskAdapter> mask_adapter; // for masking
        sksg::Merge::Mode  merge_mode;   // for clipping
    };

    SkSTArray<4, MaskRecord, true> mask_stack;
    bool has_effect = false;

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
        mask_path->setFillType(inverted ? SkPathFillType::kInverseWinding
                                        : SkPathFillType::kWinding);

        const auto blend_mode = mask_stack.empty() ? SkBlendMode::kSrc
                                                   : mask_info->fBlendMode;

        auto mask_adapter = sk_make_sp<MaskAdapter>(*m, *abuilder, blend_mode);
        abuilder->attachDiscardableAdapter(mask_adapter);

        has_effect |= mask_adapter->hasEffect();


        mask_stack.push_back({ std::move(mask_path),
                               std::move(mask_adapter),
                               mask_info->fMergeMode });
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

    // Complex masks (non-opaque or blurred) turn into a mask node stack.
    sk_sp<sksg::RenderNode> maskNode;
    if (mask_stack.count() == 1) {
        // no group needed for single mask
        const auto rec = mask_stack.front();
        maskNode = rec.mask_adapter->makeMask(std::move(rec.mask_path));
    } else {
        std::vector<sk_sp<sksg::RenderNode>> masks;
        masks.reserve(SkToSizeT(mask_stack.count()));
        for (auto& rec : mask_stack) {
            masks.push_back(rec.mask_adapter->makeMask(std::move(rec.mask_path)));
        }

        maskNode = sksg::Group::Make(std::move(masks));
    }

    return sksg::MaskEffect::Make(std::move(childNode), std::move(maskNode));
}

class LayerController final : public Animator {
public:
    LayerController(AnimatorScope&& layer_animators,
                    sk_sp<sksg::RenderNode> layer,
                    size_t tanim_count, float in, float out)
        : fLayerAnimators(std::move(layer_animators))
        , fLayerNode(std::move(layer))
        , fTransformAnimatorsCount(tanim_count)
        , fIn(in)
        , fOut(out) {}

protected:
    StateChanged onSeek(float t) override {
        // in/out may be inverted for time-reversed layers
        const auto active = (t >= fIn && t < fOut) || (t > fOut && t <= fIn);

        bool changed = false;
        if (fLayerNode) {
            changed |= (fLayerNode->isVisible() != active);
            fLayerNode->setVisible(active);
        }

        // When active, dispatch ticks to all layer animators.
        // When inactive, we must still dispatch ticks to the layer transform animators
        // (active child layers depend on transforms being updated).
        const auto dispatch_count = active ? fLayerAnimators.size()
                                           : fTransformAnimatorsCount;
        for (size_t i = 0; i < dispatch_count; ++i) {
            changed |= fLayerAnimators[i]->seek(t);
        }

        return changed;
    }

private:
    const AnimatorScope           fLayerAnimators;
    const sk_sp<sksg::RenderNode> fLayerNode;
    const size_t                  fTransformAnimatorsCount;
    const float                   fIn,
                                  fOut;
};

class MotionBlurController final : public Animator {
public:
    explicit MotionBlurController(sk_sp<MotionBlurEffect> mbe)
        : fMotionBlurEffect(std::move(mbe)) {}

protected:
    // When motion blur is present, time ticks are not passed to layer animators
    // but to the motion blur effect. The effect then drives the animators/scene-graph
    // during reval and render phases.
    StateChanged onSeek(float t) override {
        fMotionBlurEffect->setT(t);
        return true;
    }

private:
    const sk_sp<MotionBlurEffect> fMotionBlurEffect;
};

} // namespace

LayerBuilder::LayerBuilder(const skjson::ObjectValue& jlayer)
    : fJlayer(jlayer)
    , fIndex(ParseDefault<int>(jlayer["ind"], -1))
    , fParentIndex(ParseDefault<int>(jlayer["parent"], -1))
    , fType(ParseDefault<int>(jlayer["ty"], -1)) {

    if (this->isCamera() || ParseDefault<int>(jlayer["ddd"], 0)) {
        fFlags |= Flags::kIs3D;
    }
}

LayerBuilder::~LayerBuilder() = default;

bool LayerBuilder::isCamera() const {
    static constexpr int kCameraLayerType = 13;

    return fType == kCameraLayerType;
}

sk_sp<sksg::Transform> LayerBuilder::buildTransform(const AnimationBuilder& abuilder,
                                                    CompositionBuilder* cbuilder) {
    // Depending on the leaf node type, we treat the whole transform chain as either 2D or 3D.
    const auto transform_chain_type = this->is3D() ? TransformType::k3D
                                                   : TransformType::k2D;
    fLayerTransform = this->getTransform(abuilder, cbuilder, transform_chain_type);

    return fLayerTransform;
}

sk_sp<sksg::Transform> LayerBuilder::getTransform(const AnimationBuilder& abuilder,
                                                  CompositionBuilder* cbuilder,
                                                  TransformType ttype) {
    const auto cache_valid_mask = (1ul << ttype);
    if (!(fFlags & cache_valid_mask)) {
        // Set valid flag upfront to break cycles.
        fFlags |= cache_valid_mask;

        const AnimationBuilder::AutoPropertyTracker apt(&abuilder, fJlayer);
        AnimationBuilder::AutoScope ascope(&abuilder, std::move(fLayerScope));
        fTransformCache[ttype] = this->doAttachTransform(abuilder, cbuilder, ttype);
        fLayerScope = ascope.release();
        fTransformAnimatorCount = fLayerScope.size();
    }

    return fTransformCache[ttype];
}

sk_sp<sksg::Transform> LayerBuilder::getParentTransform(const AnimationBuilder& abuilder,
                                                        CompositionBuilder* cbuilder,
                                                        TransformType ttype) {
    if (auto* parent_builder = cbuilder->layerBuilder(fParentIndex)) {
        // Explicit parent layer.
        return parent_builder->getTransform(abuilder, cbuilder, ttype);
    }

    if (ttype == TransformType::k3D) {
        // During camera transform attachment, cbuilder->getCameraTransform() is null.
        // This prevents camera->camera transform chain cycles.
        SkASSERT(!this->isCamera() || !cbuilder->getCameraTransform());

        // 3D transform chains are implicitly rooted onto the camera.
        return cbuilder->getCameraTransform();
    }

    return nullptr;
}

sk_sp<sksg::Transform> LayerBuilder::doAttachTransform(const AnimationBuilder& abuilder,
                                                       CompositionBuilder* cbuilder,
                                                       TransformType ttype) {
    const skjson::ObjectValue* jtransform = fJlayer["ks"];
    if (!jtransform) {
        return nullptr;
    }

    auto parent_transform = this->getParentTransform(abuilder, cbuilder, ttype);

    if (this->isCamera()) {
        // parent_transform applies to the camera itself => it pre-composes inverted to the
        // camera/view/adapter transform.
        //
        //   T_camera' = T_camera x Inv(parent_transform)
        //
        return abuilder.attachCamera(fJlayer,
                                     *jtransform,
                                     sksg::Transform::MakeInverse(std::move(parent_transform)),
                                     cbuilder->fSize);
    }

    return this->is3D()
            ? abuilder.attachMatrix3D(*jtransform, std::move(parent_transform))
            : abuilder.attachMatrix2D(*jtransform, std::move(parent_transform));
}

bool LayerBuilder::hasMotionBlur(const CompositionBuilder* cbuilder) const {
    return cbuilder->fMotionBlurSamples > 1
        && cbuilder->fMotionBlurAngle   > 0
        && ParseDefault(fJlayer["mb"], false);
}

sk_sp<sksg::RenderNode> LayerBuilder::buildRenderTree(const AnimationBuilder& abuilder,
                                                      CompositionBuilder* cbuilder,
                                                      const LayerBuilder* prev_layer) {
    AnimationBuilder::LayerInfo layer_info = {
        cbuilder->fSize,
        ParseDefault<float>(fJlayer["ip"], 0.0f),
        ParseDefault<float>(fJlayer["op"], 0.0f),
    };
    if (SkScalarNearlyEqual(layer_info.fInPoint, layer_info.fOutPoint)) {
        abuilder.log(Logger::Level::kError, nullptr,
                     "Invalid layer in/out points: %f/%f.",
                     layer_info.fInPoint, layer_info.fOutPoint);
        return nullptr;
    }

    const AnimationBuilder::AutoPropertyTracker apt(&abuilder, fJlayer);

    using LayerBuilder =
        sk_sp<sksg::RenderNode> (AnimationBuilder::*)(const skjson::ObjectValue&,
                                                      AnimationBuilder::LayerInfo*) const;

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

    if (SkToSizeT(fType) >= SK_ARRAY_COUNT(gLayerBuildInfo) && !this->isCamera()) {
        return nullptr;
    }

    // Switch to the layer animator scope (which at this point holds transform-only animators).
    AnimationBuilder::AutoScope ascope(&abuilder, std::move(fLayerScope));

    const auto is_hidden = ParseDefault<bool>(fJlayer["hd"], false) || this->isCamera();
    const auto& build_info = gLayerBuildInfo[is_hidden ? kNullLayerType : SkToSizeT(fType)];

    // Build the layer content fragment.
    auto layer = (abuilder.*(build_info.fBuilder))(fJlayer, &layer_info);

    // Clip layers with explicit dimensions.
    float w = 0, h = 0;
    if (Parse<float>(fJlayer["w"], &w) && Parse<float>(fJlayer["h"], &h)) {
        layer = sksg::ClipEffect::Make(std::move(layer),
                                       sksg::Rect::Make(SkRect::MakeWH(w, h)),
                                       true);
    }

    // Optional layer mask.
    layer = AttachMask(fJlayer["masksProperties"], &abuilder, std::move(layer));

    // Does the transform apply to effects also?
    // (AE quirk: it doesn't - except for solid layers)
    const auto transform_effects = (build_info.fFlags & kTransformEffects);

    // Attach the transform before effects, when needed.
    if (fLayerTransform && !transform_effects) {
        layer = sksg::TransformEffect::Make(std::move(layer), fLayerTransform);
    }

    // Optional layer effects.
    if (const skjson::ArrayValue* jeffects = fJlayer["ef"]) {
        layer = EffectBuilder(&abuilder, layer_info.fSize).attachEffects(*jeffects,
                                                                         std::move(layer));
    }

    // Attach the transform after effects, when needed.
    if (fLayerTransform && transform_effects) {
        layer = sksg::TransformEffect::Make(std::move(layer), std::move(fLayerTransform));
    }

    // Optional layer styles.
    if (const skjson::ArrayValue* jstyles = fJlayer["sy"]) {
        layer = EffectBuilder(&abuilder, layer_info.fSize).attachStyles(*jstyles, std::move(layer));
    }

    // Optional layer opacity.
    // TODO: de-dupe this "ks" lookup with matrix above.
    if (const skjson::ObjectValue* jtransform = fJlayer["ks"]) {
        layer = abuilder.attachOpacity(*jtransform, std::move(layer));
    }

    const auto has_animators = !abuilder.fCurrentAnimatorScope->empty();

    sk_sp<Animator> controller = sk_make_sp<LayerController>(ascope.release(),
                                                             layer,
                                                             fTransformAnimatorCount,
                                                             layer_info.fInPoint,
                                                             layer_info.fOutPoint);

    // Optional motion blur.
    if (layer && has_animators && this->hasMotionBlur(cbuilder)) {
        // Wrap both the layer node and the controller.
        auto motion_blur = MotionBlurEffect::Make(std::move(controller), std::move(layer),
                                                  cbuilder->fMotionBlurSamples,
                                                  cbuilder->fMotionBlurAngle,
                                                  cbuilder->fMotionBlurPhase);
        controller = sk_make_sp<MotionBlurController>(motion_blur);
        layer = std::move(motion_blur);
    }

    abuilder.fCurrentAnimatorScope->push_back(std::move(controller));

    // Stash the content tree in case it is needed for later mattes.
    fContentTree = layer;

    if (ParseDefault<bool>(fJlayer["td"], false)) {
        // |layer| is a track matte.  We apply it as a mask to the next layer.
        return nullptr;
    }

    // Optional matte.
    size_t matte_mode;
    if (prev_layer && Parse(fJlayer["tt"], &matte_mode)) {
        static constexpr sksg::MaskEffect::Mode gMatteModes[] = {
            sksg::MaskEffect::Mode::kAlphaNormal, // tt: 1
            sksg::MaskEffect::Mode::kAlphaInvert, // tt: 2
            sksg::MaskEffect::Mode::kLumaNormal,  // tt: 3
            sksg::MaskEffect::Mode::kLumaInvert,  // tt: 4
        };

        if (matte_mode > 0 && matte_mode <= SK_ARRAY_COUNT(gMatteModes)) {
            // The current layer is masked with the previous layer *content*.
            layer = sksg::MaskEffect::Make(std::move(layer),
                                           prev_layer->fContentTree,
                                           gMatteModes[matte_mode - 1]);
        } else {
            abuilder.log(Logger::Level::kError, nullptr,
                         "Unknown track matte mode: %zu\n", matte_mode);
        }
    }

    // Finally, attach an optional blend mode.
    // NB: blend modes are never applied to matte sources (layer content only).
    return abuilder.attachBlendMode(fJlayer, std::move(layer));
}

} // namespace internal
} // namespace skottie
