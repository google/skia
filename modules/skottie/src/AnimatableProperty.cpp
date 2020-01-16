/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/AnimatableProperty.h"

#include "modules/skottie/src/Keyframe.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/TextValue.h"

#include <vector>

namespace skottie {
namespace internal {

// Explicit instantiations for known value types.

// == ScalarValue ==
template <>
AnimatableProperty<ScalarValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop) {
    this->parse(abuilder, jprop);
}

template <>
const ScalarValue& AnimatableProperty<ScalarValue>::eval(float t) const {
    SkASSERT(fEvaluator);
    return *fEvaluator->eval(t, &fStorage);
}

// == VectorValue ==
template <>
AnimatableProperty<VectorValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop) {
    this->parse(abuilder, jprop);
}

template <>
const VectorValue& AnimatableProperty<VectorValue>::eval(float t) const {
    SkASSERT(fEvaluator);
    return *fEvaluator->eval(t, &fStorage);
}

// == ShapeValue ==
template <>
AnimatableProperty<ShapeValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                   const skjson::ObjectValue* jprop) {
    this->parse(abuilder, jprop);
}

template <>
const ShapeValue& AnimatableProperty<ShapeValue>::eval(float t) const {
    SkASSERT(fEvaluator);
    return *fEvaluator->eval(t, &fStorage);
}

// == TextValue ==
template <>
AnimatableProperty<TextValue>::AnimatableProperty(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop) {
    this->parse(abuilder, jprop);
}

template <>
const TextValue& AnimatableProperty<TextValue>::eval(float t) const {
    SkASSERT(fEvaluator);
    return *fEvaluator->eval(t, &fStorage);
}

namespace  {

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
        const auto kf = fKeyframes.frame(t);

        if (kf.isConstant() || t <= kf.t0) {
            return &fValues[SkToSizeT(kf.vidx0)];
        }
        if (t >= kf.t1) {
            return &fValues[SkToSizeT(kf.vidx1)];
        }

        ValueTraits<T>::Lerp(fValues[SkToSizeT(kf.vidx0)],
                             fValues[SkToSizeT(kf.vidx1)],
                             fKeyframes.remapForKeyframe(kf, t),
                             storage);

        return storage;
    }

    std::vector<T> fValues;
    KeyframeStore  fKeyframes;
};

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

// Default dynamic evaluator factory: most value types only support simple keyframe animation.
template <typename T> std::unique_ptr<typename AnimatableProperty<T>::DynamicEvaluator>
MakeDynamicEvaluator(const AnimationBuilder& abuilder,
                     const skjson::ObjectValue&,
                     const skjson::ArrayValue* jframes,
                     T* storage) {
    return KeyframeDynamicEvaluator<T>::Make(abuilder, jframes, storage);
}

// Vector values can be keyframed directly or separately/independently in each dimension:
// https://helpx.adobe.com/after-effects/using/selecting-arranging-layers.html#separate_dimensions_of_position_to_animate_components_individually
template <>
std::unique_ptr<AnimatableProperty<VectorValue>::DynamicEvaluator>
MakeDynamicEvaluator<VectorValue>(const AnimationBuilder& abuilder,
                                  const skjson::ObjectValue& jprop,
                                  const skjson::ArrayValue* jframes,
                                  VectorValue* storage) {
    if (ParseDefault(jprop["s"], false)) {
        return SeparateDimensionsDynamicEvaluator::Make(abuilder, jprop, storage);
    }

    return KeyframeDynamicEvaluator<VectorValue>::Make(abuilder, jframes, storage);
}

} // namespace

template <typename T>
void AnimatableProperty<T>::parse(const AnimationBuilder& abuilder,
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
    fEvaluator = MakeDynamicEvaluator<T>(abuilder, *jprop, jpropK, &fStorage);
}

} // namespace internal
} // namespace skottie
