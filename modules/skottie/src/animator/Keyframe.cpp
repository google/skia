/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/animator/Keyframe.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/text/TextValue.h"

#define DUMP_KF_RECORDS 0

namespace skottie::internal {

KeyframeAnimatorBase::~KeyframeAnimatorBase() = default;

KeyframeAnimatorBase::LERPInfo KeyframeAnimatorBase::getLERPInfo(float t) const {
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

    if (fCurrentSegment.kf0->mapping == Keyframe::kConstantMapping) {
        // Constant/hold segment.
        return { 0, fCurrentSegment.kf0->v, fCurrentSegment.kf0->v };
    }

    return {
        this->compute_weight(fCurrentSegment, t),
        fCurrentSegment.kf0->v,
        fCurrentSegment.kf1->v,
    };
}

KeyframeAnimatorBase::KFSegment KeyframeAnimatorBase::find_segment(float t) const {
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

float KeyframeAnimatorBase::compute_weight(const KFSegment &seg, float t) const {
    SkASSERT(seg.contains(t));

    // Linear weight.
    auto w = (t - seg.kf0->t) / (seg.kf1->t - seg.kf0->t);

    // Optional cubic mapper.
    if (seg.kf0->mapping >= Keyframe::kCubicIndexOffset) {
        SkASSERT(seg.kf0->v != seg.kf1->v);
        const auto mapper_index = SkToSizeT(seg.kf0->mapping - Keyframe::kCubicIndexOffset);
        w = fCMs[mapper_index].computeYFromX(w);
    }

    return w;
}

KeyframeAnimatorBuilder::~KeyframeAnimatorBuilder() = default;

bool KeyframeAnimatorBuilder::parseKeyframes(const AnimationBuilder& abuilder,
                                             const skjson::ArrayValue& jkfs) {
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

    const auto parse_value = [&](const skjson::ObjectValue& jkf, size_t i, Keyframe::Value* v) {
        auto parsed = this->parseKFValue(abuilder, jkf, jkf["s"], v);

        // A missing value is only OK for the last legacy KF
        // (where it is pulled from prev KF 'end' value).
        if (!parsed && i > 0 && i == jkfs.size() - 1) {
            const skjson::ObjectValue* prev_kf = jkfs[i - 1];
            SkASSERT(prev_kf);
            parsed = this->parseKFValue(abuilder, jkf, (*prev_kf)["e"], v);
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

        Keyframe::Value v;
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
                prev_kf.mapping = Keyframe::kConstantMapping;
            }
        }

        fKFs.push_back({t, v, this->parseMapping(*jkf)});

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

uint32_t KeyframeAnimatorBuilder::parseMapping(const skjson::ObjectValue& jkf) {
    if (ParseDefault(jkf["h"], false)) {
        return Keyframe::kConstantMapping;
    }

    SkPoint c0, c1;
    if (!Parse(jkf["o"], &c0) ||
        !Parse(jkf["i"], &c1) ||
        SkCubicMap::IsLinear(c0, c1)) {
        return Keyframe::kLinearMapping;
    }

    // De-dupe sequential cubic mappers.
    if (c0 != prev_c0 || c1 != prev_c1 || fCMs.empty()) {
        fCMs.emplace_back(c0, c1);
        prev_c0 = c0;
        prev_c1 = c1;
    }

    SkASSERT(!fCMs.empty());
    return SkToU32(fCMs.size()) - 1 + Keyframe::kCubicIndexOffset;
}

namespace  {

// Stores generic Ts in dedicated storage, and uses indices to track in keyframes.
// TODO: we only have one instantiation left (TextValue) - specialize explicitly.
template <typename T>
class KeyframeAnimator final : public KeyframeAnimatorBase {
public:
    class Builder final : public KeyframeAnimatorBuilder {
    public:
        sk_sp<KeyframeAnimatorBase> make(const AnimationBuilder& abuilder,
                                         const skjson::ArrayValue& jkfs,
                                         void* target_value) override {
            SkASSERT(jkfs.size() > 0);

            fValues.reserve(jkfs.size());
            if (!this->parseKeyframes(abuilder, jkfs)) {
                return nullptr;
            }
            fValues.shrink_to_fit();

            return sk_sp<KeyframeAnimatorBase>(
                        new KeyframeAnimator(std::move(fKFs),
                                             std::move(fCMs),
                                             std::move(fValues),
                                             static_cast<T*>(target_value)));
        }

        bool parseValue(const AnimationBuilder& abuilder,
                          const skjson::Value& jv, void* v) const override {
            return ValueTraits<T>::FromJSON(jv, &abuilder, static_cast<T*>(v));
        }

    private:
        bool parseKFValue(const AnimationBuilder& abuilder,
                          const skjson::ObjectValue&,
                          const skjson::Value& jv,
                          Keyframe::Value* v) override {
            T val;
            if (!ValueTraits<T>::FromJSON(jv, &abuilder, &val)) {
                return false;
            }

            // TODO: full deduping?
            if (fValues.empty() || val != fValues.back()) {
                fValues.push_back(std::move(val));
            }

            v->idx = SkToU32(fValues.size() - 1);

            return true;
        }

        std::vector<T> fValues;
    };

private:
    KeyframeAnimator(std::vector<Keyframe> kfs, std::vector<SkCubicMap> cms,
                     std::vector<T> vs, T* target_value)
        : INHERITED(std::move(kfs), std::move(cms))
        , fValues(std::move(vs))
        , fTarget(target_value) {}

    StateChanged onSeek(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);

        bool changed;
        if (lerp_info.isConstant()) {
            changed = (*fTarget != fValues[SkToSizeT(lerp_info.vrec0.idx)]);
            if (changed) {
                *fTarget = fValues[SkToSizeT(lerp_info.vrec0.idx)];
            }
        } else {
            changed = ValueTraits<T>::Lerp(fValues[lerp_info.vrec0.idx],
                                           fValues[lerp_info.vrec1.idx],
                                           lerp_info.weight,
                                           fTarget);
        }

        return changed;
    }

    const std::vector<T> fValues;
    T*                   fTarget;

    using INHERITED = KeyframeAnimatorBase;
};

} // namespace

template <>
bool AnimatablePropertyContainer::bind<TextValue>(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop,
                                                  TextValue* v) {
    KeyframeAnimator<TextValue>::Builder builder;
    return this->bindImpl(abuilder, jprop, builder, v);
}

} // namespace skottie::internal
