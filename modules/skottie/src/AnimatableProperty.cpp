/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/AnimatableProperty.h"

#include "include/core/SkCubicMap.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/TextValue.h"

#include <vector>

namespace skottie {
namespace internal {

namespace  {

class ValueStoreProvider {
public:
    virtual ~ValueStoreProvider() = default;
    virtual int parseValue(const AnimationBuilder&, const skjson::Value&) = 0;
};

class KeyframeStore final : SkNoncopyable {
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

    const KeyframeRec& frame(float t) const {
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

    void parseKeyFrames(const AnimationBuilder& abuilder,
                        const skjson::ArrayValue& jframes,
                        ValueStoreProvider* value_store) {
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

            const auto v0_idx = value_store->parseValue(abuilder, (*jframe)["s"]),
                       v1_idx = value_store->parseValue(abuilder, (*jframe)["e"]);

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

    void reserve(size_t frame_count) {
        fRecs.reserve(frame_count);
        fCubicMaps.reserve(frame_count);
    }

    bool isEmpty() const { return fRecs.empty(); }

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
class KeyframeDynamicEvaluator final : public AnimatableProperty<T>::DynamicEvaluator,
                                       public ValueStoreProvider {
public:
    static std::unique_ptr<KeyframeDynamicEvaluator> Make(const AnimationBuilder& abuilder,
                                                          const skjson::ArrayValue* jframes,
                                                          T* const_storage) {
        if (!jframes) {
            return nullptr;
        }

        std::unique_ptr<KeyframeDynamicEvaluator> evaluator(
                    new KeyframeDynamicEvaluator(abuilder, *jframes));

        if (evaluator->fKeyframes.isEmpty()) {
            abuilder.log(Logger::Level::kError, jframes, "Could not parse property keyframes.");
            return nullptr;
        }

        SkASSERT(!evaluator->fValues.empty());
        if (evaluator->fValues.size() == 1ul) {
            // the property is static.
            *const_storage = *evaluator->eval(0, const_storage);
            return nullptr;
        }

        return evaluator;
    }

private:
    KeyframeDynamicEvaluator(const AnimationBuilder& abuilder, const skjson::ArrayValue& jframes) {
        // Generally, each keyframe holds two values (start, end) and a cubic mapper. Except
        // the last frame, which only holds a marker timestamp.  Then, the values series is
        // contiguous (keyframe[i].end == keyframe[i + 1].start), and we dedupe them.
        //   => we'll store (keyframes.size) values and (keyframe.size - 1) recs and cubic maps.
        fValues.reserve(jframes.size());
        fKeyframes.reserve(SkTMax<size_t>(jframes.size(), 1) - 1);
        fKeyframes.parseKeyFrames(abuilder, jframes, this);
        fValues.shrink_to_fit();
    }

    int parseValue(const AnimationBuilder& abuilder, const skjson::Value& jv) override {
        T val;
        if (!ValueTraits<T>::FromJSON(jv, &abuilder, &val) ||
            (!fValues.empty() && !ValueTraits<T>::CanLerp(val, fValues.back()))) {
            return -1;
        }

        // TODO: full deduping?
        if (fValues.empty() || val != fValues.back()) {
            fValues.push_back(std::move(val));
        }
        return SkToInt(fValues.size()) - 1;
    }

    const T* eval(float t, T* storage) const override {
        const auto& rec = fKeyframes.frame(t);
        SkASSERT(rec.isValid());

        if (rec.isConstant() || t <= rec.t0) {
            return &fValues[SkToSizeT(rec.vidx0)];
        } else if (t >= rec.t1) {
            return &fValues[SkToSizeT(rec.vidx1)];
        }

        const auto lt = fKeyframes.localT(rec, t);
        const auto& v0 = fValues[SkToSizeT(rec.vidx0)];
        const auto& v1 = fValues[SkToSizeT(rec.vidx1)];
        ValueTraits<T>::Lerp(v0, v1, lt, storage);

        return storage;
    }

    std::vector<T> fValues;
    KeyframeStore  fKeyframes;
};

} // namespace

// Explicit instantiations.

template <>
AnimatableProperty<ScalarValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop) {
    this->init(abuilder, jprop);
}

template <>
const ScalarValue& AnimatableProperty<ScalarValue>::eval(float t) const {
    SkASSERT(fEvaluator);
    return *fEvaluator->eval(t, &fStorage);
}

template <>
AnimatableProperty<VectorValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop) {
    this->init(abuilder, jprop);
}

template <>
const VectorValue& AnimatableProperty<VectorValue>::eval(float t) const {
    SkASSERT(fEvaluator);
    return *fEvaluator->eval(t, &fStorage);
}

template <>
AnimatableProperty<ShapeValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                   const skjson::ObjectValue* jprop) {
    this->init(abuilder, jprop);
}

template <>
const ShapeValue& AnimatableProperty<ShapeValue>::eval(float t) const {
    SkASSERT(fEvaluator);
    return *fEvaluator->eval(t, &fStorage);
}

template <>
AnimatableProperty<TextValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop) {
    this->init(abuilder, jprop);
}

template <>
const TextValue& AnimatableProperty<TextValue>::eval(float t) const {
    SkASSERT(fEvaluator);
    return *fEvaluator->eval(t, &fStorage);
}

namespace {

class SeparateDimensionsDynamicEvaluator final :
        public AnimatableProperty<VectorValue>::DynamicEvaluator {
public:
    static std::unique_ptr<SeparateDimensionsDynamicEvaluator> Make(
            const AnimationBuilder& abuilder,
            const skjson::ObjectValue& jprop,
            VectorValue* const_storage) {
        std::unique_ptr<SeparateDimensionsDynamicEvaluator> evaluator(
                    new SeparateDimensionsDynamicEvaluator(abuilder, jprop));

        if (evaluator->fX.isStatic() && evaluator->fY.isStatic() && evaluator->fZ.isStatic()) {
            evaluator->eval(0, const_storage);
            return nullptr;
        }

        return evaluator;
    }

private:
    SeparateDimensionsDynamicEvaluator(const AnimationBuilder& abuilder,
                                       const skjson::ObjectValue& jprop)
        : fX(abuilder, jprop["x"])
        , fY(abuilder, jprop["y"])
        , fZ(abuilder, jprop["z"]) {}

    const VectorValue* eval(float t, VectorValue* storage) const override {
        *storage = { fX(t), fY(t), fZ(t) };
        return storage;
    }

    AnimatableProperty<ScalarValue> fX, fY, fZ;
};

template <typename T> std::unique_ptr<typename AnimatableProperty<T>::DynamicEvaluator>
MakeEvaluator(const AnimationBuilder&, const skjson::ObjectValue&, const skjson::ArrayValue*, T*);

template <>
std::unique_ptr<AnimatableProperty<ScalarValue>::DynamicEvaluator>
MakeEvaluator<ScalarValue>(const AnimationBuilder& abuilder,
                           const skjson::ObjectValue&,
                           const skjson::ArrayValue* jframes,
                           ScalarValue* storage) {
    return KeyframeDynamicEvaluator<ScalarValue>::Make(abuilder, jframes, storage);
}

template <>
std::unique_ptr<AnimatableProperty<VectorValue>::DynamicEvaluator>
MakeEvaluator<VectorValue>(const AnimationBuilder& abuilder,
                           const skjson::ObjectValue& jprop,
                           const skjson::ArrayValue* jframes,
                           VectorValue* storage) {
    if (ParseDefault(jprop["s"], false)) {
        return SeparateDimensionsDynamicEvaluator::Make(abuilder, jprop, storage);
    }

    return KeyframeDynamicEvaluator<VectorValue>::Make(abuilder, jframes, storage);
}

template <>
std::unique_ptr<AnimatableProperty<ShapeValue>::DynamicEvaluator>
MakeEvaluator<ShapeValue>(const AnimationBuilder& abuilder,
                          const skjson::ObjectValue&,
                          const skjson::ArrayValue* jframes,
                          ShapeValue* storage) {
    return KeyframeDynamicEvaluator<ShapeValue>::Make(abuilder, jframes, storage);
}

template <>
std::unique_ptr<AnimatableProperty<TextValue>::DynamicEvaluator>
MakeEvaluator<TextValue>(const AnimationBuilder& abuilder,
                         const skjson::ObjectValue&,
                         const skjson::ArrayValue* jframes,
                         TextValue* storage) {
    return KeyframeDynamicEvaluator<TextValue>::Make(abuilder, jframes, storage);
}

} // namespace

template <typename T>
void AnimatableProperty<T>::init(const AnimationBuilder& abuilder,
                                 const skjson::ObjectValue* jprop) {
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
        if (ValueTraits<T>::FromJSON(jpropK, &abuilder, &fStorage)) {
            // Static property.
            return;
        }

        if (!jpropA.is<skjson::NullValue>()) {
            abuilder.log(Logger::Level::kError, jprop,
                         "Could not parse (explicit) static property.");
            return;
        }
    }

    // Keyframed property.
    fEvaluator = MakeEvaluator<T>(abuilder, *jprop, jpropK, &fStorage);
}

} // namespace internal
} // namespace skottie
