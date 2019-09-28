/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCubicMap.h"
#include "include/core/SkString.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/TextValue.h"
#include "modules/sksg/include/SkSGScene.h"

#include <memory>
#include <vector>

namespace skottie {
namespace internal {

namespace {

class KeyframeAnimatorBase : public sksg::Animator {
public:
    size_t count() const { return fRecs.size(); }

protected:
    KeyframeAnimatorBase() = default;

    struct KeyframeRec {
        float t0, t1;
        int   vidx0, vidx1, // v0/v1 indices
              cmidx;        // cubic map index

        bool contains(float t) const { return t0 <= t && t <= t1; }
        bool isConstant() const { return vidx0 == vidx1; }
        bool isValid() const {
            SkASSERT(t0 <= t1);
            // Constant frames don't need/use t1 and vidx1.
            return t0 < t1 || this->isConstant();
        }
    };

    const KeyframeRec& frame(float t) {
        if (!fCachedRec || !fCachedRec->contains(t)) {
            fCachedRec = findFrame(t);
        }
        return *fCachedRec;
    }

    float localT(const KeyframeRec& rec, float t) const {
        SkASSERT(rec.isValid());
        SkASSERT(!rec.isConstant());
        SkASSERT(t > rec.t0 && t < rec.t1);

        auto lt = (t - rec.t0) / (rec.t1 - rec.t0);

        return rec.cmidx < 0
            ? lt
            : fCubicMaps[rec.cmidx].computeYFromX(lt);
    }

    virtual int parseValue(const skjson::Value&, const AnimationBuilder* abuilder) = 0;

    void parseKeyFrames(const skjson::ArrayValue& jframes, const AnimationBuilder* abuilder) {
        // Logically, a keyframe is defined as a (t0, t1, v0, v1) tuple: a given value
        // is interpolated in the [v0..v1] interval over the [t0..t1] time span.
        //
        // There are three interestingly-different keyframe formats handled here.
        //
        // 1) Legacy keyframe format
        //
        //      - normal keyframes specify t0 ("t"), v0 ("s") and v1 ("e")
        //      - last frame only specifies a t0
        //      - t1[frame] == t0[frame + 1]
        //      - the last entry (where we cannot determine t1) is ignored
        //
        // 2) Regular (new) keyframe format
        //
        //      - all keyframes specify t0 ("t") and v0 ("s")
        //      - t1[frame] == t0[frame + 1]
        //      - v1[frame] == v0[frame + 1]
        //      - the last entry (where we cannot determine t1/v1) is ignored
        //
        // 3) Text value keyframe format
        //
        //      - similar to case #2, all keyframes specify t0 & v0
        //      - unlike case #2, all keyframes are assumed to be constant (v1 == v0),
        //        and the last frame is not discarded (its t1 is assumed -> inf)
        //

        SkPoint prev_c0 = { 0, 0 },
                prev_c1 = prev_c0;

        for (const skjson::ObjectValue* jframe : jframes) {
            if (!jframe) continue;

            float t0;
            if (!Parse<float>((*jframe)["t"], &t0))
                continue;

            const auto v0_idx = this->parseValue((*jframe)["s"], abuilder),
                       v1_idx = this->parseValue((*jframe)["e"], abuilder);

            if (!fRecs.empty()) {
                if (fRecs.back().t1 >= t0) {
                    abuilder->log(Logger::Level::kWarning, nullptr,
                                  "Ignoring out-of-order key frame (t:%f < t:%f).",
                                  t0, fRecs.back().t1);
                    continue;
                }

                // Back-fill t1 and v1 (if needed).
                auto& prev = fRecs.back();
                prev.t1 = t0;

                // Previous keyframe did not specify an end value (case #2, #3).
                if (prev.vidx1 < 0) {
                    // If this frame has no v0, we're in case #3 (constant text value),
                    // otherwise case #2 (v0 for current frame is the same as prev frame v1).
                    prev.vidx1 = v0_idx < 0 ? prev.vidx0 : v0_idx;
                }
            }

            // Start value 's' is required.
            if (v0_idx < 0)
                continue;

            if ((v1_idx < 0) && ParseDefault((*jframe)["h"], false)) {
                // Constant keyframe ("h": true).
                fRecs.push_back({t0, t0, v0_idx, v0_idx, -1 });
                continue;
            }

            const auto cubic_mapper_index = [&]() -> int {
                // Do we have non-linear control points?
                SkPoint c0, c1;
                if (!Parse((*jframe)["o"], &c0) ||
                    !Parse((*jframe)["i"], &c1) ||
                    SkCubicMap::IsLinear(c0, c1)) {
                    // No need for a cubic mapper.
                    return -1;
                }

                // De-dupe sequential cubic mappers.
                if (c0 != prev_c0 || c1 != prev_c1) {
                    fCubicMaps.emplace_back(c0, c1);
                    prev_c0 = c0;
                    prev_c1 = c1;
                }

                SkASSERT(!fCubicMaps.empty());
                return SkToInt(fCubicMaps.size()) - 1;
            };

            fRecs.push_back({t0, t0, v0_idx, v1_idx, cubic_mapper_index()});
        }

        if (!fRecs.empty()) {
            auto& last = fRecs.back();

            // If the last entry has only a v0, we're in case #3 - make it a constant frame.
            if (last.vidx0 >= 0 && last.vidx1 < 0) {
                last.vidx1 = last.vidx0;
                last.t1 = last.t0;
            }

            // If we couldn't determine a valid t1 for the last frame, discard it
            // (most likely the last frame entry for all 3 cases).
            if (!last.isValid()) {
                fRecs.pop_back();
            }
        }

        fRecs.shrink_to_fit();
        fCubicMaps.shrink_to_fit();

        SkASSERT(fRecs.empty() || fRecs.back().isValid());
    }

    void reserve(size_t frame_count) {
        fRecs.reserve(frame_count);
        fCubicMaps.reserve(frame_count);
    }

private:
    const KeyframeRec* findFrame(float t) const {
        SkASSERT(!fRecs.empty());

        auto f0 = &fRecs.front(),
             f1 = &fRecs.back();

        SkASSERT(f0->isValid());
        SkASSERT(f1->isValid());

        if (t < f0->t0) {
            return f0;
        }

        if (t > f1->t1) {
            return f1;
        }

        while (f0 != f1) {
            SkASSERT(f0 < f1);
            SkASSERT(t >= f0->t0 && t <= f1->t1);

            const auto f = f0 + (f1 - f0) / 2;
            SkASSERT(f->isValid());

            if (t > f->t1) {
                f0 = f + 1;
            } else {
                f1 = f;
            }
        }

        SkASSERT(f0 == f1);
        SkASSERT(f0->contains(t));

        return f0;
    }

    std::vector<KeyframeRec> fRecs;
    std::vector<SkCubicMap>  fCubicMaps;
    const KeyframeRec*       fCachedRec = nullptr;

    using INHERITED = sksg::Animator;
};

template <typename T>
class KeyframeAnimator final : public KeyframeAnimatorBase {
public:
    static sk_sp<KeyframeAnimator> Make(const skjson::ArrayValue* jv,
                                        const AnimationBuilder* abuilder,
                                        std::function<void(const T&)>&& apply) {
        if (!jv) return nullptr;

        sk_sp<KeyframeAnimator> animator(new KeyframeAnimator(*jv, abuilder, std::move(apply)));

        return animator->count() ? animator : nullptr;
    }

    bool isConstant() const {
        SkASSERT(!fVs.empty());
        return fVs.size() == 1ul;
    }

protected:
    void onTick(float t) override {
        fApplyFunc(*this->eval(this->frame(t), t, &fScratch));
    }

private:
    KeyframeAnimator(const skjson::ArrayValue& jframes,
                     const AnimationBuilder* abuilder,
                     std::function<void(const T&)>&& apply)
        : fApplyFunc(std::move(apply)) {
        // Generally, each keyframe holds two values (start, end) and a cubic mapper. Except
        // the last frame, which only holds a marker timestamp.  Then, the values series is
        // contiguous (keyframe[i].end == keyframe[i + 1].start), and we dedupe them.
        //   => we'll store (keyframes.size) values and (keyframe.size - 1) recs and cubic maps.
        fVs.reserve(jframes.size());
        this->reserve(SkTMax<size_t>(jframes.size(), 1) - 1);

        this->parseKeyFrames(jframes, abuilder);

        fVs.shrink_to_fit();
    }

    int parseValue(const skjson::Value& jv, const AnimationBuilder* abuilder) override {
        T val;
        if (!ValueTraits<T>::FromJSON(jv, abuilder, &val) ||
            (!fVs.empty() && !ValueTraits<T>::CanLerp(val, fVs.back()))) {
            return -1;
        }

        // TODO: full deduping?
        if (fVs.empty() || val != fVs.back()) {
            fVs.push_back(std::move(val));
        }
        return SkToInt(fVs.size()) - 1;
    }

    const T* eval(const KeyframeRec& rec, float t, T* v) const {
        SkASSERT(rec.isValid());
        if (rec.isConstant() || t <= rec.t0) {
            return &fVs[rec.vidx0];
        } else if (t >= rec.t1) {
            return &fVs[rec.vidx1];
        }

        const auto lt = this->localT(rec, t);
        const auto& v0 = fVs[rec.vidx0];
        const auto& v1 = fVs[rec.vidx1];
        ValueTraits<T>::Lerp(v0, v1, lt, v);

        return v;
    }

    const std::function<void(const T&)> fApplyFunc;
    std::vector<T>                      fVs;

    // LERP storage: we use this to temporarily store interpolation results.
    // Alternatively, the temp result could live on the stack -- but for vector values that would
    // involve dynamic allocations on each tick.  This a trade-off to avoid allocator pressure
    // during animation.
    T                                   fScratch; // lerp storage

    using INHERITED = KeyframeAnimatorBase;
};

template <typename T>
static inline bool BindPropertyImpl(const skjson::ObjectValue* jprop,
                                    const AnimationBuilder* abuilder,
                                    AnimatorScope* ascope,
                                    std::function<void(const T&)>&& apply,
                                    const T* noop = nullptr) {
    if (!jprop) return false;

    const auto& jpropA = (*jprop)["a"];
    const auto& jpropK = (*jprop)["k"];

    if (!(*jprop)["x"].is<skjson::NullValue>()) {
        abuilder->log(Logger::Level::kWarning, nullptr, "Unsupported expression.");
    }

    // Older Json versions don't have an "a" animation marker.
    // For those, we attempt to parse both ways.
    if (!ParseDefault<bool>(jpropA, false)) {
        T val;
        if (ValueTraits<T>::FromJSON(jpropK, abuilder, &val)) {
            // Static property.
            if (noop && val == *noop)
                return false;

            apply(val);
            return true;
        }

        if (!jpropA.is<skjson::NullValue>()) {
            abuilder->log(Logger::Level::kError, jprop,
                          "Could not parse (explicit) static property.");
            return false;
        }
    }

    // Keyframe property.
    auto animator = KeyframeAnimator<T>::Make(jpropK, abuilder, std::move(apply));

    if (!animator) {
        abuilder->log(Logger::Level::kError, jprop, "Could not parse keyframed property.");
        return false;
    }

    if (animator->isConstant()) {
        // If all keyframes are constant, there is no reason to treat this
        // as an animated property - apply immediately and discard the animator.
        animator->tick(0);
    } else {
        ascope->push_back(std::move(animator));
    }

    return true;
}

class SplitPointAnimator final : public sksg::Animator {
public:
    static sk_sp<SplitPointAnimator> Make(const skjson::ObjectValue* jprop,
                                          const AnimationBuilder* abuilder,
                                          std::function<void(const VectorValue&)>&& apply,
                                          const VectorValue*) {
        if (!jprop) return nullptr;

        sk_sp<SplitPointAnimator> split_animator(new SplitPointAnimator(std::move(apply)));

        // This raw pointer is captured in lambdas below. But the lambdas are owned by
        // the object itself, so the scope is bound to the life time of the object.
        auto* split_animator_ptr = split_animator.get();

        if (!BindPropertyImpl<ScalarValue>((*jprop)["x"], abuilder, &split_animator->fAnimators,
                [split_animator_ptr](const ScalarValue& x) { split_animator_ptr->setX(x); }) ||
            !BindPropertyImpl<ScalarValue>((*jprop)["y"], abuilder, &split_animator->fAnimators,
                [split_animator_ptr](const ScalarValue& y) { split_animator_ptr->setY(y); })) {
            abuilder->log(Logger::Level::kError, jprop, "Could not parse split property.");
            return nullptr;
        }

        if (split_animator->fAnimators.empty()) {
            // Static split property: commit the (buffered) value and discard.
            split_animator->onTick(0);
            return nullptr;
        }

        return split_animator;
    }

    void onTick(float t) override {
        for (const auto& animator : fAnimators) {
            animator->tick(t);
        }

        const VectorValue vec = { fX, fY };
        fApplyFunc(vec);
    }

    void setX(const ScalarValue& x) { fX = x; }
    void setY(const ScalarValue& y) { fY = y; }

private:
    explicit SplitPointAnimator(std::function<void(const VectorValue&)>&& apply)
        : fApplyFunc(std::move(apply)) {}

    const std::function<void(const VectorValue&)> fApplyFunc;
    sksg::AnimatorList                            fAnimators;

    ScalarValue                                   fX = 0,
                                                  fY = 0;

    using INHERITED = sksg::Animator;
};

bool BindSplitPositionProperty(const skjson::Value& jv,
                               const AnimationBuilder* abuilder,
                               AnimatorScope* ascope,
                               std::function<void(const VectorValue&)>&& apply,
                               const VectorValue* noop) {
    if (auto split_animator = SplitPointAnimator::Make(jv, abuilder, std::move(apply), noop)) {
        ascope->push_back(std::move(split_animator));
        return true;
    }

    return false;
}

} // namespace

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const ScalarValue&)>&& apply,
                  const ScalarValue* noop) const {
    return BindPropertyImpl(jv, this, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const VectorValue&)>&& apply,
                  const VectorValue* noop) const {
    if (!jv.is<skjson::ObjectValue>())
        return false;

    return ParseDefault<bool>(jv.as<skjson::ObjectValue>()["s"], false)
        ? BindSplitPositionProperty(jv, this, fCurrentAnimatorScope, std::move(apply), noop)
        : BindPropertyImpl(jv, this, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const ShapeValue&)>&& apply,
                  const ShapeValue* noop) const {
    return BindPropertyImpl(jv, this, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const TextValue&)>&& apply,
                  const TextValue* noop) const {
    return BindPropertyImpl(jv, this, fCurrentAnimatorScope, std::move(apply), noop);
}

} // namespace internal
} // namespace skottie
