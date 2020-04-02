/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/animator/KeyframeAnimator.h"

namespace skottie::internal {

namespace  {

// Scalar specialization: stores scalar values (floats) inline in keyframes.
class ScalarKeyframeAnimator final : public KeyframeAnimator {
public:
    class Builder final : public KeyframeAnimatorBuilder {
    public:
        sk_sp<KeyframeAnimator> make(const AnimationBuilder& abuilder,
                                     const skjson::ArrayValue& jkfs,
                                     void* target_value) override {
            SkASSERT(jkfs.size() > 0);
            if (!this->parseKeyframes(abuilder, jkfs)) {
                return nullptr;
            }

            return sk_sp<ScalarKeyframeAnimator>(
                        new ScalarKeyframeAnimator(std::move(fKFs),
                                                   std::move(fCMs),
                                                   static_cast<ScalarValue*>(target_value)));
        }

        bool parseValue(const AnimationBuilder&, const skjson::Value& jv, void* v) const override {
            return Parse(jv, static_cast<float*>(v));
        }

    private:
        bool parseKFValue(const AnimationBuilder&,
                          const skjson::ObjectValue&,
                          const skjson::Value& jv,
                          Keyframe::Value* v) override {
            return Parse(jv, &v->flt);
        }
    };

private:
    ScalarKeyframeAnimator(std::vector<Keyframe> kfs,
                           std::vector<SkCubicMap> cms,
                           ScalarValue* target_value)
        : INHERITED(std::move(kfs), std::move(cms))
        , fTarget(target_value) {}

    StateChanged onSeek(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);
        const auto  old_value = *fTarget;

        *fTarget = Lerp(lerp_info.vrec0.flt, lerp_info.vrec1.flt, lerp_info.weight);

        return *fTarget != old_value;
    }

    ScalarValue* fTarget;

    using INHERITED = KeyframeAnimator;
};

} // namespace

template <>
bool AnimatablePropertyContainer::bind<ScalarValue>(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop,
                                                    ScalarValue* v) {
    ScalarKeyframeAnimator::Builder builder;

    return this->bindImpl(abuilder, jprop, builder, v);
}

} // namespace skottie::internal
