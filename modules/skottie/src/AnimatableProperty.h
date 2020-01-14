/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAnimatableProperty_DEFINED
#define SkottieAnimatableProperty_DEFINED

#include "include/private/SkNoncopyable.h"

#include <memory>

namespace skjson {

class ObjectValue;

} // namespace skjson

namespace skottie {
namespace internal {

class AnimationBuilder;

template <typename T>
class AnimatableProperty {
public:
    AnimatableProperty(const AnimationBuilder&, const skjson::ObjectValue*);

//    AnimatableProperty(AnimatableProperty&&)            = default;
//    AnimatableProperty& operator=(AnimatableProperty&&) = default;

    const T& operator()(float t) const {
        return this->isStatic() ? fCurrentValue : *fEvaluator->eval(t, &fCurrentValue);
    }

    bool isStatic() const { return !fEvaluator; }

    bool isStaticValue(const T& v) const { return this->isStatic() && fCurrentValue == v; }

    class DynamicEvaluator : SkNoncopyable {
    public:
        virtual ~DynamicEvaluator() = default;

        virtual const T* eval(float t, T* dst) const;
    };

private:
    AnimatableProperty(const skjson::ObjectValue*, const AnimationBuilder&);

    std::unique_ptr<DynamicEvaluator> fEvaluator;
    mutable T                         fCurrentValue = T();
};

} // namespace internal
} // namespace skottie

#endif // SkottieAnimatableProperty_DEFINED
