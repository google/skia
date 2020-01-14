/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCubicMap.h"
#include "modules/skottie/src/AnimatableProperty.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/TextValue.h"

#include <type_traits>

namespace skottie {
namespace internal {

// Explicit instantiations.
template <>
const float* AnimatableProperty<ScalarValue>::DynamicEvaluator::eval(float, float*) const;

template <>
AnimatableProperty<ScalarValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop)
    : AnimatableProperty(jprop, abuilder) {}

template <>
AnimatableProperty<VectorValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop)
    : AnimatableProperty(jprop, abuilder) {}

template <>
AnimatableProperty<ShapeValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                   const skjson::ObjectValue* jprop)
    : AnimatableProperty(jprop, abuilder) {}

template <>
AnimatableProperty<TextValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop)
    : AnimatableProperty(jprop, abuilder) {}

namespace  {

class ValueStore : SkNoncopyable {
public:
    virtual ~ValueStore() = default;

    virtual int parseValue(const AnimationBuilder&, const skjson::Value&) = 0;
};

template <typename T>
class TypedValueStore final : public ValueStore {
public:
    void reserve(size_t count) { fValues.reserve(count);  }
    void shrink_to_fit()       { fValues.shrink_to_fit(); }

    const T& operator[](size_t i) const { return fValues[i]; }

private:
    int parseValue(const AnimationBuilder& abuilder, const skjson::Value& jv) override {
        T val;
        if (!ValueTraits<T>::FromJSON(jv, &abuilder, &val) ||
            (!fValues.empty() && !ValueTraits<T>::CanLerp(val, fValues.back()))) {
            return -1;
        }

        // TODO: is full deduping worth it?
        if (fValues.empty() || val != fValues.back()) {
            fValues.push_back(std::move(val));
        }
        return SkToInt(fValues.size()) - 1;
    }

    std::vector<T> fValues;
};

class KeyframeStore : SkNoncopyable {
public:
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

    bool empty() const { return fRecs.empty(); }

    bool isConstant() const {
        SkASSERT(!this->empty());
        for (const auto& rec : fRecs) {
            if (!rec.isConstant()) {
                return false;
            }
        }
        return true;
    }

    const KeyframeRec& keyframe(float t) const {
        if (!fCachedRec || !fCachedRec->contains(t)) {
            fCachedRec = findFrame(t);
        }
        return *fCachedRec;
    }

    void reserve(size_t frame_count) {
        fRecs.reserve(frame_count);
        fCubicMaps.reserve(frame_count);
    }

    void parseKeyFrames(const AnimationBuilder& abuilder, const skjson::ArrayValue& jframes,
                        ValueStore* vstore) {
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

            const auto v0_idx = vstore->parseValue(abuilder, (*jframe)["s"]),
                       v1_idx = vstore->parseValue(abuilder, (*jframe)["e"]);

            if (!fRecs.empty()) {
                if (fRecs.back().t1 >= t0) {
                    abuilder.log(Logger::Level::kWarning, nullptr,
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

    float localT(const KeyframeRec& rec, float t) const {
        SkASSERT(rec.isValid());
        SkASSERT(!rec.isConstant());
        SkASSERT(t > rec.t0 && t < rec.t1);

        auto lt = (t - rec.t0) / (rec.t1 - rec.t0);

        return rec.cmidx < 0
            ? lt
            : fCubicMaps[rec.cmidx].computeYFromX(lt);
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

    std::vector<KeyframeRec>   fRecs;
    std::vector<SkCubicMap>    fCubicMaps;
    mutable const KeyframeRec* fCachedRec = nullptr;
};

template <typename T>
class KeyframeEvaluator final : public AnimatableProperty<T>::DynamicEvaluator {
public:
    static std::unique_ptr<KeyframeEvaluator> Make(const AnimationBuilder& abuilder,
                                                   const skjson::ArrayValue* jframes,
                                                   T* constant_value) {
        if (!jframes) {
            return nullptr;
        }

        std::unique_ptr<KeyframeEvaluator> evaluator(new KeyframeEvaluator(abuilder, *jframes));

        if (evaluator->empty()) {
            // no valid keyframes
            return nullptr;
        }

        if (evaluator->isConstant()) {
            // all keyframes are constant
            evaluator->eval(0, constant_value);
            return nullptr;
        }

        // TODO: GRRRRR, we need this gone.
        evaluator->eval(0, constant_value);
        return evaluator;
    }

    bool empty() const { return fKeyframeStore.empty(); }

    bool isConstant() const { return fKeyframeStore.isConstant(); }

    const T* eval(float t, T* v_storage) const override {
        const auto& rec = fKeyframeStore.keyframe(t);
        SkASSERT(rec.isValid());

        if (rec.isConstant() || t <= rec.t0) {
            return &fValues[SkToSizeT(rec.vidx0)];
        } else if (t >= rec.t1) {
            return &fValues[SkToSizeT(rec.vidx1)];
        }

        const auto lt = fKeyframeStore.localT(rec, t);
        const auto& v0 = fValues[SkToSizeT(rec.vidx0)];
        const auto& v1 = fValues[SkToSizeT(rec.vidx1)];
        ValueTraits<T>::Lerp(v0, v1, lt, v_storage);

        return v_storage;
    }

private:
    KeyframeEvaluator(const AnimationBuilder& abuilder, const skjson::ArrayValue& jframes) {
        // Generally, each keyframe holds two values (start, end) and a cubic mapper. Except
        // the last frame, which only holds a marker timestamp.  Then, the values series is
        // contiguous (keyframe[i].end == keyframe[i + 1].start), and we dedupe them.
        //   => we'll store (keyframes.size) values and (keyframe.size - 1) recs and cubic maps.
        fKeyframeStore.reserve(SkTMax<size_t>(jframes.size(), 1) - 1);
        fValues.reserve(jframes.size());

        fKeyframeStore.parseKeyFrames(abuilder, jframes, &fValues);

        fValues.shrink_to_fit();
    }

    KeyframeStore      fKeyframeStore;
    TypedValueStore<T> fValues;
};

class SeparateDimensionsEvaluator final : public AnimatableProperty<VectorValue>::DynamicEvaluator {
public:
    static std::unique_ptr<SeparateDimensionsEvaluator> Make(const AnimationBuilder& abuilder,
                                                             const skjson::ObjectValue& jprop,
                                                             VectorValue* constant_value) {
        if (!ParseDefault<bool>(jprop["s"], false)) {
            return nullptr;
        }

        std::unique_ptr<SeparateDimensionsEvaluator> evaluator(
                    new SeparateDimensionsEvaluator(abuilder, jprop));

        if (evaluator->fVx.isStatic() && evaluator->fVy.isStatic() && evaluator->fVz.isStatic()) {
            *constant_value = { evaluator->fVx(0), evaluator->fVy(0), evaluator->fVz(0) };
            return nullptr;
        }

        return evaluator;
    }

private:
    SeparateDimensionsEvaluator(const AnimationBuilder& abuilder,
                                const skjson::ObjectValue& jprop)
        : fVx(abuilder, jprop["x"])
        , fVy(abuilder, jprop["y"])
        , fVz(abuilder, jprop["z"]) {}

    const VectorValue* eval(float t, VectorValue* v_storage) const override {
        v_storage->resize(3);
        (*v_storage)[0] = fVx(t);
        (*v_storage)[1] = fVy(t);
        (*v_storage)[2] = fVz(t);

        return v_storage;
    }

    bool isConstant() const { return fVx.isStatic() && fVy.isStatic() && fVz.isStatic(); }

    AnimatableProperty<ScalarValue> fVx, fVy, fVz;
};

template <typename T>
std::unique_ptr<typename AnimatableProperty<T>::DynamicEvaluator>
MakeKeyframeEvaluator(const AnimationBuilder& abuilder,
                      const skjson::ObjectValue&,
                      const skjson::ArrayValue* jpropK,
                      T* constant_value) {
    return KeyframeEvaluator<T>::Make(abuilder, jpropK, constant_value);
}

template <>
std::unique_ptr<typename AnimatableProperty<VectorValue>::DynamicEvaluator>
MakeKeyframeEvaluator(const AnimationBuilder& abuilder,
                      const skjson::ObjectValue& jprop,
                      const skjson::ArrayValue* jpropK,
                      VectorValue* constant_value) {
    // Vector properties may be keyframed in separate dimensions.
    if (ParseDefault(jprop["s"], false)) {
        return SeparateDimensionsEvaluator::Make(abuilder, jprop, constant_value);
    }

    return KeyframeEvaluator<VectorValue>::Make(abuilder, jpropK, constant_value);
}

} // namespace

template <typename T>
AnimatableProperty<T>::AnimatableProperty(const skjson::ObjectValue* jprop,
                                          const AnimationBuilder& abuilder) {
    if (!jprop) {
        return;
    }

    const auto& jpropA = (*jprop)["a"];
    const auto& jpropK = (*jprop)["k"];

    if (!(*jprop)["x"].is<skjson::NullValue>()) {
        abuilder.log(Logger::Level::kWarning, nullptr, "Unsupported expression.");
    }

    // Older Json versions don't have an "a" animation marker.
    // For those, we attempt to parse both ways.
    if (!ParseDefault<bool>(jpropA, false)) {
        if (ValueTraits<T>::FromJSON(jpropK, &abuilder, &fCurrentValue)) {
            // Static property.
            return;
        }

        if (!jpropA.is<skjson::NullValue>()) {
            // 'a' exists but is not a bool.
            abuilder.log(Logger::Level::kError, jprop,
                         "Could not parse (explicit) static property.");
            return ;
        }
    }

    // Dynamic property.
    fEvaluator = MakeKeyframeEvaluator<T>(abuilder, *jprop, jpropK, &fCurrentValue);

    if (!fEvaluator) {
        abuilder.log(Logger::Level::kError, jprop, "Could not parse keyframed property.");
    }
}

} // namespace internal
} // namespace skottie
