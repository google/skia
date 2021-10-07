/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottiePriv.h"

#include "modules/skottie/include/ExternalLayer.h"
#include "modules/skottie/src/Composition.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGScene.h"
#include "src/core/SkTLazy.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

namespace  {

// "Animates" time based on the layer's "tm" property.
class TimeRemapper final : public AnimatablePropertyContainer {
public:
    TimeRemapper(const skjson::ObjectValue& jtm, const AnimationBuilder* abuilder, float scale)
        : fScale(scale) {
        this->bind(*abuilder, jtm, fT);
    }

    float t() const { return fT * fScale; }

private:
    void onSync() override {
        // nothing to sync - we just track t
    }

    const float fScale;

    ScalarValue fT = 0;
};

// Applies a bias/scale/remap t-adjustment to child animators.
class CompTimeMapper final : public Animator {
public:
    CompTimeMapper(AnimatorScope&& layer_animators,
                   sk_sp<TimeRemapper> remapper,
                   float time_bias, float time_scale)
        : fAnimators(std::move(layer_animators))
        , fRemapper(std::move(remapper))
        , fTimeBias(time_bias)
        , fTimeScale(time_scale) {}

    StateChanged onSeek(float t) override {
        if (fRemapper) {
            // When time remapping is active, |t| is fully driven externally.
            fRemapper->seek(t);
            t = fRemapper->t();
        } else {
            t = (t + fTimeBias) * fTimeScale;
        }

        bool changed = false;

        for (const auto& anim : fAnimators) {
            changed |= anim->seek(t);
        }

        return changed;
    }

private:
    const AnimatorScope       fAnimators;
    const sk_sp<TimeRemapper> fRemapper;
    const float               fTimeBias,
                              fTimeScale;
};

} // namespace

sk_sp<sksg::RenderNode> AnimationBuilder::attachExternalPrecompLayer(
        const skjson::ObjectValue& jlayer,
        const LayerInfo& layer_info) const {

    if (!fPrecompInterceptor) {
        return nullptr;
    }

    const skjson::StringValue* id = jlayer["refId"];
    const skjson::StringValue* nm = jlayer["nm"];

    if (!id || !nm) {
        return nullptr;
    }

    auto external_layer = fPrecompInterceptor->onLoadPrecomp(id->begin(),
                                                             nm->begin(),
                                                             layer_info.fSize);
    if (!external_layer) {
        return nullptr;
    }

    // Attaches an ExternalLayer implementation to the animation scene graph.
    class SGAdapter final : public sksg::RenderNode {
    public:
        SG_ATTRIBUTE(T, float, fCurrentT)

        SGAdapter(sk_sp<ExternalLayer> external, const SkSize& layer_size)
            : fExternal(std::move(external))
            , fSize(layer_size) {}

    private:
        SkRect onRevalidate(sksg::InvalidationController*, const SkMatrix&) override {
            return SkRect::MakeSize(fSize);
        }

        void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
            // Commit all pending effects via a layer if needed,
            // since we don't have knowledge of the external content.
            const auto local_scope =
                ScopedRenderContext(canvas, ctx).setIsolation(this->bounds(),
                                                              canvas->getTotalMatrix(),
                                                              true);
            fExternal->render(canvas, static_cast<double>(fCurrentT));
        }

        const RenderNode* onNodeAt(const SkPoint& pt) const override {
            SkASSERT(this->bounds().contains(pt.fX, pt.fY));
            return this;
        }

        const sk_sp<ExternalLayer> fExternal;
        const SkSize               fSize;
        float                      fCurrentT = 0;
    };

    // Connects an SGAdapter to the animator tree and dispatches seek events.
    class AnimatorAdapter final : public Animator {
    public:
        AnimatorAdapter(sk_sp<SGAdapter> sg_adapter, float fps)
            : fSGAdapter(std::move(sg_adapter))
            , fFps(fps) {}

    private:
        StateChanged onSeek(float t) override {
            fSGAdapter->setT(t / fFps);

            return true;
        }

        const sk_sp<SGAdapter> fSGAdapter;
        const float            fFps;
    };

    auto sg_adapter = sk_make_sp<SGAdapter>(std::move(external_layer), layer_info.fSize);

    fCurrentAnimatorScope->push_back(sk_make_sp<AnimatorAdapter>(sg_adapter, fFrameRate));

    return std::move(sg_adapter);
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachPrecompLayer(const skjson::ObjectValue& jlayer,
                                                             LayerInfo* layer_info) const {
    sk_sp<TimeRemapper> time_remapper;
    if (const skjson::ObjectValue* jtm = jlayer["tm"]) {
        time_remapper = sk_make_sp<TimeRemapper>(*jtm, this, fFrameRate);
    }

    const auto start_time = ParseDefault<float>(jlayer["st"], 0.0f),
             stretch_time = ParseDefault<float>(jlayer["sr"], 1.0f);
    const auto requires_time_mapping = !SkScalarNearlyEqual(start_time  , 0) ||
                                       !SkScalarNearlyEqual(stretch_time, 1) ||
                                       time_remapper;

    // Precomp layers are sized explicitly.
    layer_info->fSize = SkSize::Make(ParseDefault<float>(jlayer["w"], 0.0f),
                                     ParseDefault<float>(jlayer["h"], 0.0f));

    SkTLazy<AutoScope> local_scope;
    if (requires_time_mapping) {
        local_scope.init(this);
    }

    auto precomp_layer = this->attachExternalPrecompLayer(jlayer, *layer_info);

    if (!precomp_layer) {
        const ScopedAssetRef precomp_asset(this, jlayer);
        if (precomp_asset) {
            AutoPropertyTracker apt(this, *precomp_asset, PropertyObserver::NodeType::COMPOSITION);
            precomp_layer =
                CompositionBuilder(*this, layer_info->fSize, *precomp_asset).build(*this);
        }
    }

    if (requires_time_mapping) {
        const auto t_bias  = -start_time,
                   t_scale = sk_ieee_float_divide(1, stretch_time);
        auto time_mapper = sk_make_sp<CompTimeMapper>(local_scope->release(),
                                                      std::move(time_remapper),
                                                      t_bias,
                                                      sk_float_isfinite(t_scale) ? t_scale : 0);

        fCurrentAnimatorScope->push_back(std::move(time_mapper));
    }

    return precomp_layer;
}

} // namespace internal
} // namespace skottie
