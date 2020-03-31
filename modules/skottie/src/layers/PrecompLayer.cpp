/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottiePriv.h"

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

    auto precomp_layer = this->attachAssetRef(jlayer,
        [this, layer_info] (const skjson::ObjectValue& jcomp) {
            return CompositionBuilder(*this, layer_info->fSize, jcomp).build(*this);
        });

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
