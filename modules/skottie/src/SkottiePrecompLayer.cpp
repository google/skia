/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottiePriv.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGScene.h"
#include "src/core/SkMakeUnique.h"
#include "src/core/SkTLazy.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

sk_sp<sksg::RenderNode> AnimationBuilder::attachPrecompLayer(const skjson::ObjectValue& jlayer,
                                                             const LayerInfo&,
                                                             AnimatorScope* ascope) const {
    const skjson::ObjectValue* time_remap = jlayer["tm"];
    // Empirically, a time mapper supersedes start/stretch.
    const auto start_time = time_remap ? 0.0f : ParseDefault<float>(jlayer["st"], 0.0f),
             stretch_time = time_remap ? 1.0f : ParseDefault<float>(jlayer["sr"], 1.0f);
    const auto requires_time_mapping = !SkScalarNearlyEqual(start_time  , 0) ||
                                       !SkScalarNearlyEqual(stretch_time, 1) ||
                                       time_remap;

    AnimatorScope local_animators;
    auto precomp_layer = this->attachAssetRef(jlayer,
                                              requires_time_mapping ? &local_animators : ascope,
                                              [this] (const skjson::ObjectValue& jcomp,
                                                      AnimatorScope* ascope) {
                                                  return this->attachComposition(jcomp, ascope);
                                              });

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
        auto time_mapper =
            skstd::make_unique<CompTimeMapper>(std::move(local_animators), t_bias,
                                               sk_float_isfinite(t_scale) ? t_scale : 0);
        if (time_remap) {
            // The lambda below captures a raw pointer to the mapper object.  That should be safe,
            // because both the lambda and the mapper are scoped/owned by ctx->fAnimators.
            auto* raw_mapper = time_mapper.get();
            auto  frame_rate = fFrameRate;
            this->bindProperty<ScalarValue>(*time_remap, ascope,
                    [raw_mapper, frame_rate](const ScalarValue& t) {
                        raw_mapper->remapTime(t * frame_rate);
                    });
        }
        ascope->push_back(std::move(time_mapper));
    }

    return precomp_layer;
}

} // namespace internal
} // namespace skottie
