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

template <typename T>
class AnimatableProperty final : SkNoncopyable {
public:
    AnimatableProperty(const AnimationBuilder&, const skjson::ObjectValue*);
    ~AnimatableProperty();

    bool isStatic() const { return !fEvaluator; }

    const T& operator()(float t) const {
        return this->isStatic() ? fStorage : this->eval(t);
    }

    class DynamicEvaluator : SkNoncopyable {
    public:
        virtual ~DynamicEvaluator() = default;

        virtual const T* eval(float t, T* storage) const = 0;
    };

private:
    void init(const AnimationBuilder&, const skjson::ObjectValue*);

    const T& eval(float) const;

    std::unique_ptr<DynamicEvaluator> fEvaluator;
    mutable T                         fStorage = T();
};

using AnimatableScalarProperty = AnimatableProperty<ScalarValue>;

} // namespace internal
} // namespace skottie

#endif // SkottieAnimatableProperty_DEFINED
