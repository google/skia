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
        explicit Builder(ScalarValue* target) : fTarget(target) {}

        sk_sp<KeyframeAnimator> make(const AnimationBuilder& abuilder,
                                     const skjson::ArrayValue& jkfs) override {
            SkASSERT(jkfs.size() > 0);
            if (!this->parseKeyframes(abuilder, jkfs)) {
                return nullptr;
            }

            return sk_sp<ScalarKeyframeAnimator>(
                        new ScalarKeyframeAnimator(std::move(fKFs), std::move(fCMs), fTarget));
        }

        bool parseValue(const AnimationBuilder&, const skjson::Value& jv) const override {
            return Parse(jv, fTarget);
        }

    private:
        bool parseKFValue(const AnimationBuilder&,
                          const skjson::ObjectValue&,
                          const skjson::Value& jv,
                          Keyframe::Value* v) override {
            return Parse(jv, &v->flt);
        }

        ScalarValue* fTarget;
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
    ScalarKeyframeAnimator::Builder builder(v);

    return this->bindImpl(abuilder, jprop, builder);
}

} // namespace skottie::internal
