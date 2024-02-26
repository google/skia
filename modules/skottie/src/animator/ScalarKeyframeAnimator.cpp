/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCubicMap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SlotManager.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/animator/KeyframeAnimator.h"
#include "src/utils/SkJSON.h"

#include <utility>
#include <vector>

namespace skottie::internal {

namespace  {

    // Scalar specialization: stores scalar values (floats) inline in keyframes.
class ScalarKeyframeAnimator final : public KeyframeAnimator {
public:
    ScalarKeyframeAnimator(std::vector<Keyframe> kfs,
                           std::vector<SkCubicMap> cms,
                           ScalarValue* target_value)
        : INHERITED(std::move(kfs), std::move(cms))
        , fTarget(target_value) {}

private:

    StateChanged onSeek(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);
        const auto  old_value = *fTarget;

        *fTarget = Lerp(lerp_info.vrec0.flt, lerp_info.vrec1.flt, lerp_info.weight);

        return *fTarget != old_value;
    }

    ScalarValue* fTarget;

    using INHERITED = KeyframeAnimator;
};

    // Scalar specialization: stores scalar values (floats).
class ScalarExpressionAnimator final : public Animator {
public:
    ScalarExpressionAnimator(sk_sp<ExpressionEvaluator<ScalarValue>> expression_evaluator,
        ScalarValue* target_value)
        : fExpressionEvaluator(std::move(expression_evaluator))
        , fTarget(target_value) {}

private:

    StateChanged onSeek(float t) override {
        auto old_value = *fTarget;

        *fTarget = fExpressionEvaluator->evaluate(t);

        return *fTarget != old_value;
    }

    sk_sp<ExpressionEvaluator<ScalarValue>> fExpressionEvaluator;
    ScalarValue* fTarget;
};

class ScalarAnimatorBuilder final : public AnimatorBuilder {
    public:
        explicit ScalarAnimatorBuilder(ScalarValue* target)
            : INHERITED(Keyframe::Value::Type::kScalar)
            , fTarget(target) {}

        sk_sp<KeyframeAnimator> makeFromKeyframes(const AnimationBuilder& abuilder,
                                     const skjson::ArrayValue& jkfs) override {
            SkASSERT(jkfs.size() > 0);
            if (!this->parseKeyframes(abuilder, jkfs)) {
                return nullptr;
            }

            return sk_sp<ScalarKeyframeAnimator>(
                        new ScalarKeyframeAnimator(std::move(fKFs), std::move(fCMs), fTarget));
        }

        sk_sp<Animator> makeFromExpression(ExpressionManager& em, const char* expr) override {
            sk_sp<ExpressionEvaluator<ScalarValue>> expression_evaluator =
                em.createNumberExpressionEvaluator(expr);
            return sk_make_sp<ScalarExpressionAnimator>(expression_evaluator, fTarget);
        }


        bool parseValue(const AnimationBuilder&, const skjson::Value& jv) const override {
            return ::skottie::Parse(jv, fTarget);
        }

    private:
        bool parseKFValue(const AnimationBuilder&,
                          const skjson::ObjectValue&,
                          const skjson::Value& jv,
                          Keyframe::Value* v) override {
            return ::skottie::Parse(jv, &v->flt);
        }

        ScalarValue* fTarget;

        using INHERITED = AnimatorBuilder;
    };

} // namespace

template <>
bool AnimatablePropertyContainer::bind<ScalarValue>(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop,
                                                    ScalarValue* v) {
    if (const auto* sid = ParseSlotID(jprop)) {
        fHasSlotID = true;
        abuilder.fSlotManager->trackScalarValue(SkString(sid->begin()), v, sk_ref_sp(this));
    }
    ScalarAnimatorBuilder builder(v);

    return this->bindImpl(abuilder, jprop, builder);
}

} // namespace skottie::internal
