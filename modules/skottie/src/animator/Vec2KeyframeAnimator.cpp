/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkContourMeasure.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/animator/KeyframeAnimator.h"

namespace skottie::internal {

namespace  {

// Spatial 2D specialization: stores SkV2s and optional contour interpolators externally.
class Vec2KeyframeAnimator final : public KeyframeAnimator {
    struct SpatialValue {
        Vec2Value               v2;
        sk_sp<SkContourMeasure> cmeasure;
    };

public:
    class Builder final : public KeyframeAnimatorBuilder {
    public:
        sk_sp<KeyframeAnimator> make(const AnimationBuilder& abuilder,
                                         const skjson::ArrayValue& jkfs,
                                         void* target_value) override {
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
                                                 static_cast<Vec2Value*>(target_value)));
        }

        bool parseValue(const AnimationBuilder&, const skjson::Value& jv, void* v) const override {
            return Parse(jv, static_cast<Vec2Value*>(v));
        }

    private:
        void backfill_spatial(const SpatialValue& val) {
            if (fTi == SkV2{0,0} && fTo == SkV2{0,0}) {
                // no tangents => linear
                return;
            }

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
            SkPath p;
            p.moveTo (prev_val.v2.x        , prev_val.v2.y);
            p.cubicTo(prev_val.v2.x + fTo.x, prev_val.v2.y + fTo.y,
                           val.v2.x + fTi.x,      val.v2.y + fTi.y,
                           val.v2.x,              val.v2.y);
            prev_val.cmeasure = SkContourMeasureIter(p, false).next();
        }

        bool parseKFValue(const AnimationBuilder&,
                          const skjson::ObjectValue& jkf,
                          const skjson::Value& jv,
                          Keyframe::Value* v) override {
            SpatialValue val;
            if (!Parse(jv, &val.v2)) {
                return false;
            }

            this->backfill_spatial(val);

            // Track the last keyframe spatial tangents (checked on next parseValue).
            fTi = ParseDefault<SkV2>(jkf["ti"], {0,0});
            fTo = ParseDefault<SkV2>(jkf["to"], {0,0});

            if (fValues.empty() || val.v2 != fValues.back().v2) {
                fValues.push_back(std::move(val));
            }

            v->idx = SkToU32(fValues.size() - 1);

            return true;
        }

        std::vector<SpatialValue> fValues;
        SkV2                      fTi{0,0},
                                  fTo{0,0};
    };

private:
    Vec2KeyframeAnimator(std::vector<Keyframe> kfs, std::vector<SkCubicMap> cms,
                         std::vector<SpatialValue> vs, Vec2Value* target_value)
        : INHERITED(std::move(kfs), std::move(cms))
        , fValues(std::move(vs))
        , fTarget(target_value) {}

    StateChanged update(const Vec2Value& new_value) {
        const auto changed = (new_value != *fTarget);
        *fTarget = new_value;

        return changed;
    }

    StateChanged onSeek(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);

        const auto& v0 = fValues[lerp_info.vrec0.idx];
        if (v0.cmeasure) {
            // Spatial keyframe: the computed weight is relative to the interpolation path
            // arc length.
            SkPoint pos;
            if (v0.cmeasure->getPosTan(lerp_info.weight * v0.cmeasure->length(), &pos, nullptr)) {
                return this->update({ pos.fX, pos.fY });
            }
        }

        const auto& v1 = fValues[lerp_info.vrec1.idx];
        return this->update(Lerp(v0.v2, v1.v2, lerp_info.weight));
    }

    const std::vector<SpatialValue> fValues;
    Vec2Value*                      fTarget;

    using INHERITED = KeyframeAnimator;
};

} // namespace

template <>
bool AnimatablePropertyContainer::bind<Vec2Value>(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop,
                                                  Vec2Value* v) {
    if (!jprop) {
        return false;
    }

    if (!ParseDefault<bool>((*jprop)["s"], false)) {
        // Regular (static or keyframed) 2D value.
        Vec2KeyframeAnimator::Builder builder;
        return this->bindImpl(abuilder, jprop, builder, v);
    }

    // Separate-dimensions vector value: each component is animated independently.
    return this->bind(abuilder, (*jprop)["x"], &v->x)
         | this->bind(abuilder, (*jprop)["y"], &v->y);
}

} // namespace skottie::internal
