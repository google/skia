/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkContourMeasure.h"
#include "include/core/SkPathBuilder.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/animator/KeyframeAnimator.h"

#include <cmath>

namespace skottie::internal {

namespace  {

// Spatial 2D specialization: stores SkV2s and optional contour interpolators externally.
class Vec2KeyframeAnimator final : public KeyframeAnimator {
public:
    struct SpatialValue {
        Vec2Value               v2;
        sk_sp<SkContourMeasure> cmeasure;
    };

    Vec2KeyframeAnimator(std::vector<Keyframe> kfs, std::vector<SkCubicMap> cms,
                         std::vector<SpatialValue> vs, Vec2Value* vec_target, float* rot_target)
        : INHERITED(std::move(kfs), std::move(cms))
        , fValues(std::move(vs))
        , fVecTarget(vec_target)
        , fRotTarget(rot_target) {}

private:
    StateChanged update(const Vec2Value& new_vec_value, const Vec2Value& new_tan_value) {
        auto changed = (new_vec_value != *fVecTarget);
        *fVecTarget = new_vec_value;

        if (fRotTarget) {
            const auto new_rot_value = SkRadiansToDegrees(std::atan2(new_tan_value.y,
                                                                     new_tan_value.x));
            changed |= new_rot_value != *fRotTarget;
            *fRotTarget = new_rot_value;
        }

        return changed;
    }

    StateChanged onSeek(float t) override {
        auto get_lerp_info = [this](float t) {
            auto lerp_info = this->getLERPInfo(t);

            // When tracking rotation/orientation, the last keyframe requires special handling:
            // it doesn't store any spatial information but it is expected to maintain the
            // previous orientation (per AE semantics).
            //
            // The easiest way to achieve this is to actually swap with the previous keyframe,
            // with an adjusted weight of 1.
            const auto vidx = lerp_info.vrec0.idx;
            if (fRotTarget && vidx == fValues.size() - 1 && vidx > 0) {
                SkASSERT(!fValues[vidx].cmeasure);
                SkASSERT(lerp_info.vrec1.idx == vidx);

                // Change LERPInfo{0, SIZE - 1, SIZE - 1}
                // to     LERPInfo{1, SIZE - 2, SIZE - 1}
                lerp_info.weight = 1;
                lerp_info.vrec0  = {vidx - 1};

                // This yields equivalent lerp results because keyframed values are contiguous
                // i.e frame[n-1].end_val == frame[n].start_val.
            }

            return lerp_info;
        };

        const auto lerp_info = get_lerp_info(t);

        const auto& v0 = fValues[lerp_info.vrec0.idx];
        if (v0.cmeasure) {
            // Spatial keyframe: the computed weight is relative to the interpolation path
            // arc length.
            SkPoint  pos;
            SkVector tan;
            const float len = v0.cmeasure->length(),
                   distance = len * lerp_info.weight;
            if (v0.cmeasure->getPosTan(distance, &pos, &tan)) {
                // Easing can yield a sub/super normal weight, which in turn can cause the
                // interpolation position to become negative or larger than the path length.
                // In those cases the expectation is to extrapolate using the endpoint tangent.
                if (distance < 0 || distance > len) {
                    const float overshoot = std::copysign(std::max(-distance, distance - len),
                                                          distance);
                    pos += tan * overshoot;
                }

                return this->update({ pos.fX, pos.fY }, {tan.fX, tan.fY});
            }
        }

        const auto& v1 = fValues[lerp_info.vrec1.idx];
        const auto tan = v1.v2 - v0.v2;

        return this->update(Lerp(v0.v2, v1.v2, lerp_info.weight), tan);
    }

    const std::vector<Vec2KeyframeAnimator::SpatialValue> fValues;
    Vec2Value*                      fVecTarget;
    float*                          fRotTarget;

    using INHERITED = KeyframeAnimator;
};

class Vec2ExpressionAnimator final : public Animator {
public:
    Vec2ExpressionAnimator(sk_sp<ExpressionEvaluator<std::vector<float>>> expression_evaluator,
        Vec2Value* target_value)
        : fExpressionEvaluator(std::move(expression_evaluator))
        , fTarget(target_value) {}

private:

    StateChanged onSeek(float t) override {
        auto old_value = *fTarget;

        std::vector<float> result = fExpressionEvaluator->evaluate(t);
        fTarget->x = result.size() > 0 ? result[0] : 0;
        fTarget->y = result.size() > 1 ? result[1] : 0;

        return *fTarget != old_value;
    }

    sk_sp<ExpressionEvaluator<std::vector<float>>> fExpressionEvaluator;
    Vec2Value* fTarget;
};

class Vec2AnimatorBuilder final : public AnimatorBuilder {
    public:
        Vec2AnimatorBuilder(Vec2Value* vec_target, float* rot_target)
            : INHERITED(Keyframe::Value::Type::kIndex)
            , fVecTarget(vec_target)
            , fRotTarget(rot_target) {}

        sk_sp<KeyframeAnimator> makeFromKeyframes(const AnimationBuilder& abuilder,
                                     const skjson::ArrayValue& jkfs) override {
            SkASSERT(jkfs.size() > 0);

            fValues.reserve(jkfs.size());
            if (!this->parseKeyframes(abuilder, jkfs)) {
                return nullptr;
            }
            fValues.shrink_to_fit();

            return sk_sp<Vec2KeyframeAnimator>(
                        new Vec2KeyframeAnimator(std::move(fKFs),
                                                 std::move(fCMs),
                                                 std::move(fValues),
                                                 fVecTarget,
                                                 fRotTarget));
        }

        sk_sp<Animator> makeFromExpression(ExpressionManager& em, const char* expr) override {
            sk_sp<ExpressionEvaluator<std::vector<SkScalar>>> expression_evaluator =
                em.createArrayExpressionEvaluator(expr);
            return sk_make_sp<Vec2ExpressionAnimator>(expression_evaluator, fVecTarget);
        }

        bool parseValue(const AnimationBuilder&, const skjson::Value& jv) const override {
            return Parse(jv, fVecTarget);
        }

    private:
        void backfill_spatial(const Vec2KeyframeAnimator::SpatialValue& val) {
            SkASSERT(!fValues.empty());
            auto& prev_val = fValues.back();
            SkASSERT(!prev_val.cmeasure);

            if (val.v2 == prev_val.v2) {
                // spatial interpolation only make sense for noncoincident values
                return;
            }

            // Check whether v0 and v1 have the same direction AND ||v0||>=||v1||
            auto check_vecs = [](const SkV2& v0, const SkV2& v1) {
                const auto v0_len2 = v0.lengthSquared(),
                           v1_len2 = v1.lengthSquared();

                // check magnitude
                if (v0_len2 < v1_len2) {
                    return false;
                }

                // v0, v1 have the same direction iff dot(v0,v1) = ||v0||*||v1||
                // <=>    dot(v0,v1)^2 = ||v0||^2 * ||v1||^2
                const auto dot = v0.dot(v1);
                return SkScalarNearlyEqual(dot * dot, v0_len2 * v1_len2);
            };

            if (check_vecs(val.v2 - prev_val.v2, fTo) &&
                check_vecs(prev_val.v2 - val.v2, fTi)) {
                // Both control points lie on the [prev_val..val] segment
                //   => we can power-reduce the Bezier "curve" to a straight line.
                return;
            }

            // Finally, this looks like a legitimate spatial keyframe.
            SkPathBuilder p;
            p.moveTo (prev_val.v2.x        , prev_val.v2.y);
            p.cubicTo(prev_val.v2.x + fTo.x, prev_val.v2.y + fTo.y,
                           val.v2.x + fTi.x,      val.v2.y + fTi.y,
                           val.v2.x,              val.v2.y);
            prev_val.cmeasure = SkContourMeasureIter(p.detach(), false).next();
        }

        bool parseKFValue(const AnimationBuilder&,
                          const skjson::ObjectValue& jkf,
                          const skjson::Value& jv,
                          Keyframe::Value* v) override {
            Vec2KeyframeAnimator::SpatialValue val;
            if (!Parse(jv, &val.v2)) {
                return false;
            }

            if (fPendingSpatial) {
                this->backfill_spatial(val);
            }

            // Track the last keyframe spatial tangents (checked on next parseValue).
            fTi             = ParseDefault<SkV2>(jkf["ti"], {0,0});
            fTo             = ParseDefault<SkV2>(jkf["to"], {0,0});
            fPendingSpatial = fTi != SkV2{0,0} || fTo != SkV2{0,0};

            if (fValues.empty() || val.v2 != fValues.back().v2 || fPendingSpatial) {
                fValues.push_back(std::move(val));
            }

            v->idx = SkToU32(fValues.size() - 1);

            return true;
        }

        std::vector<Vec2KeyframeAnimator::SpatialValue> fValues;
        Vec2Value*                fVecTarget; // required
        float*                    fRotTarget; // optional
        SkV2                      fTi{0,0},
                                  fTo{0,0};
        bool                      fPendingSpatial = false;

        using INHERITED = AnimatorBuilder;
    };

} // namespace

bool AnimatablePropertyContainer::bindAutoOrientable(const AnimationBuilder& abuilder,
                                                     const skjson::ObjectValue* jprop,
                                                     Vec2Value* v, float* orientation) {
    if (!jprop) {
        return false;
    }

    if (const auto* sid = ParseSlotID(jprop)) {
        fHasSlotID = true;
        abuilder.fSlotManager->trackVec2Value(SkString(sid->begin()), v, sk_ref_sp(this));
    }

    if (!ParseDefault<bool>((*jprop)["s"], false)) {
        // Regular (static or keyframed) 2D value.
        Vec2AnimatorBuilder builder(v, orientation);
        return this->bindImpl(abuilder, jprop, builder);
    }

    // Separate-dimensions vector value: each component is animated independently.
    bool boundX = this->bind(abuilder, (*jprop)["x"], &v->x);
    bool boundY = this->bind(abuilder, (*jprop)["y"], &v->y);
    return boundX || boundY;
}

template <>
bool AnimatablePropertyContainer::bind<Vec2Value>(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop,
                                                  Vec2Value* v) {
    return this->bindAutoOrientable(abuilder, jprop, v, nullptr);
}

} // namespace skottie::internal
