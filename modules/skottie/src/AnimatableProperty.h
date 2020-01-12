/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAnimatableProperty_DEFINED
#define SkottieAnimatableProperty_DEFINED

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

    const T& operator()(float t) {
        if (this->isAnimated()) {
            this->eval(t);
        }

        return fCurrentValue;
    }

    bool isAnimated() const { return fKeyframes; }

    bool isStaticValue(const T& v) const { return !this->isAnimated() && fCurrentValue == v; }

private:
    AnimatableProperty(const skjson::ObjectValue*, const AnimationBuilder&);

    class KeyFrames;

    std::unique_ptr<KeyFrames> fKeyframes;
    T                          fCurrentValue = T();
};

} // namespace internal
} // namespace skottie

#endif // SkottieAnimatableProperty_DEFINED
