/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Animator.h"

#include "include/core/SkCubicMap.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/TextValue.h"

#include <cmath>
#include <vector>

#define DUMP_KF_RECORDS 0

namespace skottie {
namespace internal {

void AnimatablePropertyContainer::onTick(float t) {
    for (const auto& animator : fAnimators) {
        animator->tick(t);
    }
    this->onSync();
}

void AnimatablePropertyContainer::attachDiscardableAdapter(
        sk_sp<AnimatablePropertyContainer> child) {
    if (!child) {
        return;
    }

    if (child->isStatic()) {
        child->tick(0);
        return;
    }

    fAnimators.push_back(child);
}

void AnimatablePropertyContainer::shrink_to_fit() {
    fAnimators.shrink_to_fit();
}

namespace  {

class KeyframeAnimatorBase : public sksg::Animator {
public:
    bool isConstant() const {
        SkASSERT(!fKFs.empty());

        // parseKeyFrames() ensures we only keep a single frame for constant properties.
        return fKFs.size() == 1;
    }

protected:
    KeyframeAnimatorBase() = default;

    // We can store scalar values inline; other types are stored externally,
    // and we track them by index.
    struct Value {
        union {
            uint32_t idx;
            float    flt;
        };

        bool operator==(const Value& other) const {
            return idx == other.idx
                || flt == other.flt; // +/-0
        }
        bool operator!=(const Value& other) const { return !((*this) == other); }
    };

    struct LERPInfo {
        float weight; // vrec0/vrec1 weight [0..1]
        Value vrec0, vrec1;

        bool isConstant() const { return vrec0 == vrec1; }
    };

    // Main entry point: |t| -> LERPInfo
    LERPInfo getLERPInfo(float t) const {
        SkASSERT(!fKFs.empty());

        if (t <= fKFs.front().t) {
            // Constant/clamped segment.
            return { 0, fKFs.front().v, fKFs.front().v };
        }
        if (t >= fKFs.back().t) {
            // Constant/clamped segment.
            return { 0, fKFs.back().v, fKFs.back().v };
        }

        // Cache the current segment (most queries have good locality).
        if (!fCurrentSegment.contains(t)) {
            fCurrentSegment = this->find_segment(t);
        }
        SkASSERT(fCurrentSegment.contains(t));

        if (fCurrentSegment.kf0->mapping == KFRec::kConstantMapping) {
            // Constant/hold segment.
            return { 0, fCurrentSegment.kf0->v, fCurrentSegment.kf0->v };
        }

        return {
            this->compute_weight(fCurrentSegment, t),
            fCurrentSegment.kf0->v,
            fCurrentSegment.kf1->v,
        };
    }

    virtual bool parseValue(const AnimationBuilder&, const skjson::Value&, Value*) = 0;

    bool parseKeyFrames(const AnimationBuilder& abuilder, const skjson::ArrayValue& jkfs) {
        // Keyframe format:
        //
        // [                        // array of
        //   {
        //     "t": <float>         // keyframe time
        //     "s": <T>             // keyframe value
        //     "h": <bool>          // optional constant/hold keyframe marker
        //     "i": [<float,float>] // optional "in" Bezier control point
        //     "o": [<float,float>] // optional "out" Bezier control point
        //   },
        //   ...
        // ]
        //
        // Legacy keyframe format:
        //
        // [                        // array of
        //   {
        //     "t": <float>         // keyframe time
        //     "s": <T>             // keyframe start value
        //     "e": <T>             // keyframe end value
        //     "h": <bool>          // optional constant/hold keyframe marker (constant mapping)
        //     "i": [<float,float>] // optional "in" Bezier control point (cubic mapping)
        //     "o": [<float,float>] // optional "out" Bezier control point (cubic mapping)
        //   },
        //   ...
        //   {
        //     "t": <float>         // last keyframe only specifies a t
        //                          // the value is prev. keyframe end value
        //   }
        // ]
        //
        // Note: the legacy format contains duplicates, as normal frames are contiguous:
        //       frame(n).e == frame(n+1).s

        // Tracks previous cubic map parameters (for deduping).
        SkPoint prev_c0 = { 0, 0 },
                prev_c1 = { 0, 0 };

        const auto parse_mapping = [&](const skjson::ObjectValue& jkf) {
            if (ParseDefault(jkf["h"], false)) {
                return KFRec::kConstantMapping;
            }

            SkPoint c0, c1;
            if (!Parse(jkf["o"], &c0) ||
                !Parse(jkf["i"], &c1) ||
                SkCubicMap::IsLinear(c0, c1)) {
                return KFRec::kLinearMapping;
            }

            // De-dupe sequential cubic mappers.
            if (c0 != prev_c0 || c1 != prev_c1 || fCMs.empty()) {
                fCMs.emplace_back(c0, c1);
                prev_c0 = c0;
                prev_c1 = c1;
            }

            SkASSERT(!fCMs.empty());
            return SkToU32(fCMs.size()) - 1 + KFRec::kCubicIndexOffset;
        };

        const auto parse_value = [&](const skjson::ObjectValue& jkf, size_t i, Value* v) {
            auto parsed = this->parseValue(abuilder, jkf["s"], v);

            // A missing value is only OK for the last legacy KF
            // (where it is pulled from prev KF 'end' value).
            if (!parsed && i > 0 && i == jkfs.size() - 1) {
                const skjson::ObjectValue* prev_kf = jkfs[i - 1];
                SkASSERT(prev_kf);
                parsed = this->parseValue(abuilder, (*prev_kf)["e"], v);
            }

            return parsed;
        };

        bool constant_value = true;

        fKFs.reserve(jkfs.size());

        for (size_t i = 0; i < jkfs.size(); ++i) {
            const skjson::ObjectValue* jkf = jkfs[i];
            if (!jkf) {
                return false;
            }

            float t;
            if (!Parse<float>((*jkf)["t"], &t)) {
                return false;
            }

            Value v;
            if (!parse_value(*jkf, i, &v)) {
                return false;
            }

            if (i > 0) {
                auto& prev_kf = fKFs.back();

                // Ts must be strictly monotonic.
                if (t <= prev_kf.t) {
                    return false;
                }

                // We can power-reduce the mapping of repeated values (implicitly constant).
                if (v == prev_kf.v) {
                    prev_kf.mapping = KFRec::kConstantMapping;
                }
            }

            fKFs.push_back({t, v, parse_mapping(*jkf)});

            constant_value = constant_value && (v == fKFs.front().v);
        }

        SkASSERT(fKFs.size() == jkfs.size());
        fCMs.shrink_to_fit();

        if (constant_value) {
            // When all keyframes hold the same value, we can discard all but one
            // (interpolation has no effect).
            fKFs.resize(1);
        }

#if(DUMP_KF_RECORDS)
        SkDEBUGF("Animator[%p], values: %lu, KF records: %zu\n",
                 this, fKFs.back().v_idx + 1, fKFs.size());
        for (const auto& kf : fKFs) {
            SkDEBUGF("  { t: %1.3f, v_idx: %lu, mapping: %lu }\n", kf.t, kf.v_idx, kf.mapping);
        }
#endif
        return true;
    }

private:
    // We store one KFRec for each AE/Lottie keyframe.
    struct KFRec {
        float    t;
        Value    v;
        uint32_t mapping; // Encodes the value interpolation in [KFRec_n .. KFRec_n+1):
                          //   0 -> constant
                          //   1 -> linear
                          //   n -> cubic: fCMs[n-2]

        static constexpr uint32_t kConstantMapping  = 0;
        static constexpr uint32_t kLinearMapping    = 1;
        static constexpr uint32_t kCubicIndexOffset = 2;
    };

    // Two sequential KFRecs determine how the value varies within [kf0 .. kf1)
    struct KFSegment {
        const KFRec* kf0;
        const KFRec* kf1;

        bool contains(float t) const {
            SkASSERT(!!kf0 == !!kf1);
            SkASSERT(!kf0 || kf1 == kf0 + 1);

            return kf0 && kf0->t <= t && t < kf1->t;
        }
    };

    // Find the KFSegment containing |t|.
    KFSegment find_segment(float t) const {
        SkASSERT(fKFs.size() > 1);
        SkASSERT(t > fKFs.front().t);
        SkASSERT(t < fKFs.back().t);

        auto kf0 = &fKFs.front(),
             kf1 = &fKFs.back();

        // Binary-search, until we reduce to sequential keyframes.
        while (kf0 + 1 != kf1) {
            SkASSERT(kf0 < kf1);
            SkASSERT(kf0->t <= t && t < kf1->t);

            const auto mid_kf = kf0 + (kf1 - kf0) / 2;

            if (t >= mid_kf->t) {
                kf0 = mid_kf;
            } else {
                kf1 = mid_kf;
            }
        }

        return {kf0, kf1};
    }

    // Given a |t| and a containing KFSegment, compute the local interpolation weight.
    float compute_weight(const KFSegment& seg, float t) const {
        SkASSERT(seg.contains(t));

        // Linear weight.
        auto w = (t - seg.kf0->t) / (seg.kf1->t - seg.kf0->t);

        // Optional cubic mapper.
        if (seg.kf0->mapping >= KFRec::kCubicIndexOffset) {
            SkASSERT(seg.kf0->v != seg.kf1->v);
            const auto mapper_index = SkToSizeT(seg.kf0->mapping - KFRec::kCubicIndexOffset);
            w = fCMs[mapper_index].computeYFromX(w);
        }

        return w;
    }

    std::vector<KFRec>      fKFs; // Keyframe records, one per AE/Lottie keyframe.
    std::vector<SkCubicMap> fCMs; // Optional cubic mappers (Bezier interpolation).
    mutable KFSegment       fCurrentSegment = { nullptr, nullptr }; // Cached segment.
};

// Stores generic Ts in dedicated storage, and uses indices to track in keyframes.
template <typename T>
class KeyframeAnimator final : public KeyframeAnimatorBase {
public:
    static sk_sp<KeyframeAnimator> Make(const AnimationBuilder& abuilder,
                                        const skjson::ArrayValue* jkfs,
                                        T* target_value) {
        if (!jkfs || jkfs->size() < 1) {
            return nullptr;
        }

        sk_sp<KeyframeAnimator> animator(new KeyframeAnimator(target_value));

        animator->fValues.reserve(jkfs->size());
        if (!animator->parseKeyFrames(abuilder, *jkfs)) {
            return nullptr;
        }
        animator->fValues.shrink_to_fit();

        return animator;
    }

private:
    explicit KeyframeAnimator(T* target_value)
        : fTarget(target_value) {}

    bool parseValue(const AnimationBuilder& abuilder, const skjson::Value& jv, Value* v) override {
        T val;
        if (!ValueTraits<T>::FromJSON(jv, &abuilder, &val) ||
            (!fValues.empty() && !ValueTraits<T>::CanLerp(val, fValues.back()))) {
            return false;
        }

        // TODO: full deduping?
        if (fValues.empty() || val != fValues.back()) {
            fValues.push_back(std::move(val));
        }

        v->idx = SkToU32(fValues.size() - 1);

        return true;
    }

    void onTick(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);

        if (lerp_info.isConstant()) {
            *fTarget = fValues[SkToSizeT(lerp_info.vrec0.idx)];
        } else {
            ValueTraits<T>::Lerp(fValues[lerp_info.vrec0.idx],
                                 fValues[lerp_info.vrec1.idx],
                                 lerp_info.weight,
                                 fTarget);
        }
    }

    std::vector<T> fValues;
    T*             fTarget;
};

// Scalar specialization: stores scalar values (floats) inline in keyframes.
class ScalarKeyframeAnimator final : public KeyframeAnimatorBase {
public:
    static sk_sp<ScalarKeyframeAnimator> Make(const AnimationBuilder& abuilder,
                                              const skjson::ArrayValue* jkfs,
                                              ScalarValue* target_value) {
        if (!jkfs || jkfs->size() < 1) {
            return nullptr;
        }

        sk_sp<ScalarKeyframeAnimator> animator(new ScalarKeyframeAnimator(target_value));

        return animator->parseKeyFrames(abuilder, *jkfs)
                ? animator
                : nullptr;
    }

private:
    explicit ScalarKeyframeAnimator(ScalarValue* target_value)
        : fTarget(target_value) {}

    bool parseValue(const AnimationBuilder&, const skjson::Value& jv, Value* v) override {
        return Parse(jv, &v->flt);
    }

    void onTick(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);

        *fTarget = lerp_info.vrec0.flt +
                  (lerp_info.vrec1.flt - lerp_info.vrec0.flt) * lerp_info.weight;
    }

    ScalarValue* fTarget;
};

template <typename T>
auto make_animator(const AnimationBuilder& abuilder,
                   const skjson::ArrayValue* jkfs,
                   T* target_value) {
    return KeyframeAnimator<T>::Make(abuilder, jkfs, target_value);
}

auto make_animator(const AnimationBuilder& abuilder,
                   const skjson::ArrayValue* jkfs,
                   ScalarValue* target_value) {
    return ScalarKeyframeAnimator::Make(abuilder, jkfs, target_value);
}

template <typename T>
bool BindPropertyImpl(const AnimationBuilder& abuilder,
                      const skjson::ObjectValue* jprop,
                      AnimatorScope* ascope,
                      T* target_value) {
    if (!jprop) {
        return false;
    }

    const auto& jpropA = (*jprop)["a"];
    const auto& jpropK = (*jprop)["k"];

    if (!(*jprop)["x"].is<skjson::NullValue>()) {
        abuilder.log(Logger::Level::kWarning, nullptr, "Unsupported expression.");
    }

    // Older Json versions don't have an "a" animation marker.
    // For those, we attempt to parse both ways.
    if (!ParseDefault<bool>(jpropA, false)) {
        if (ValueTraits<T>::FromJSON(jpropK, &abuilder, target_value)) {
            // Static property.
            return true;
        }

        if (!jpropA.is<skjson::NullValue>()) {
            abuilder.log(Logger::Level::kError, jprop,
                         "Could not parse (explicit) static property.");
            return false;
        }
    }

    // Keyframed property.
    auto animator = make_animator(abuilder, jpropK, target_value);

    if (!animator) {
        abuilder.log(Logger::Level::kError, jprop, "Could not parse keyframed property.");
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

} // namespace

// Explicit instantiations
template <>
bool AnimatablePropertyContainer::bind<ScalarValue>(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop,
                                                    ScalarValue* v) {
    return BindPropertyImpl(abuilder, jprop, &fAnimators, v);
}

template <>
bool AnimatablePropertyContainer::bind<ShapeValue>(const AnimationBuilder& abuilder,
                                                   const skjson::ObjectValue* jprop,
                                                   ShapeValue* v) {
    return BindPropertyImpl(abuilder, jprop, &fAnimators, v);
}

template <>
bool AnimatablePropertyContainer::bind<TextValue>(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop,
                                                  TextValue* v) {
    return BindPropertyImpl(abuilder, jprop, &fAnimators, v);
}

template <>
bool AnimatablePropertyContainer::bind<VectorValue>(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop,
                                                    VectorValue* v) {
    if (!jprop) {
        return false;
    }

    if (!ParseDefault<bool>((*jprop)["s"], false)) {
        // Regular (static or keyframed) vector value.
        return BindPropertyImpl(abuilder, jprop, &fAnimators, v);
    }

    // Separate-dimensions vector value: each component is animated independently.
    class SeparateDimensionsAnimator final : public AnimatablePropertyContainer {
    public:
        static sk_sp<SeparateDimensionsAnimator> Make(const AnimationBuilder& abuilder,
                                                      const skjson::ObjectValue& jprop,
                                                      VectorValue* v) {

            sk_sp<SeparateDimensionsAnimator> animator(new SeparateDimensionsAnimator(v));
            auto bound  = animator->bind(abuilder, jprop["x"], &animator->fX);
                 bound |= animator->bind(abuilder, jprop["y"], &animator->fY);
                 bound |= animator->bind(abuilder, jprop["z"], &animator->fZ);

            return bound ? animator : nullptr;
        }

    private:
        explicit SeparateDimensionsAnimator(VectorValue* v)
            : fTarget(v) {}

        void onSync() override {
            *fTarget = { fX, fY, fZ };
        }

        VectorValue* fTarget;
        ScalarValue  fX = 0,
                     fY = 0,
                     fZ = 0;
    };

    if (auto sd_animator = SeparateDimensionsAnimator::Make(abuilder, *jprop, v)) {
        if (sd_animator->isStatic()) {
            sd_animator->tick(0);
        } else {
            fAnimators.push_back(std::move(sd_animator));
        }
        return true;
    }

    return false;
}

} // namespace internal
} // namespace skottie
