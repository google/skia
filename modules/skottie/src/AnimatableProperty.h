/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAnimatableProperty_DEFINED
#define SkottieAnimatableProperty_DEFINED

#include "include/private/SkNoncopyable.h"
#include "modules/skottie/src/SkottieValue.h"

#include <memory>

namespace skjson {

class ObjectValue;

} // namespace skjson

namespace skottie {
namespace internal {

class AnimationBuilder;

// A delegate for properties which may change over time.
template <typename T>
class AnimatableProperty final : SkNoncopyable {
public:
    AnimatableProperty(const AnimationBuilder&, const skjson::ObjectValue*);

    bool isStatic() const { return !fEvaluator; }

    // The property value at a given time.
    const T& operator()(float t) const {
        return this->isStatic() ? fStorage : this->eval(t);
    }

    class DynamicEvaluator : SkNoncopyable {
    public:
        virtual ~DynamicEvaluator() = default;

        virtual const T* eval(float t, T* storage) const = 0;
    };

private:
    void parse(const AnimationBuilder&, const skjson::ObjectValue*);

    const T& eval(float) const;

    std::unique_ptr<DynamicEvaluator> fEvaluator;     // null in the case of static properties
    mutable T                         fStorage = T();
};

} // namespace internal
} // namespace skottie

#endif // SkottieAnimatableProperty_DEFINED
