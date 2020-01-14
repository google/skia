/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCubicMap.h"
#include "include/core/SkString.h"
#include "modules/skottie/src/AnimatableProperty.h"
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

template <typename T>
class AnimatablePropertyWrapper final : public sksg::Animator {
public:
    static sk_sp<AnimatablePropertyWrapper> Make(const AnimationBuilder& abuilder,
                                                 const skjson::ObjectValue* jprop,
                                                 std::function<void(const T&)>&& apply) {
        if (!jprop) {
            return nullptr;
        }

        return sk_sp<AnimatablePropertyWrapper>(new AnimatablePropertyWrapper(abuilder, jprop,
                                                                              std::move(apply)));
    }

    bool isStatic() const { return fProperty.isStatic(); }

    const T& eval(float t) const { return fProperty(t); }

private:
    AnimatablePropertyWrapper(const AnimationBuilder& abuilder,
                              const skjson::ObjectValue* jprop,
                              std::function<void(const T&)>&& apply)
        : fApplyFunc(std::move(apply))
        , fProperty(abuilder, jprop) {}

    void onTick(float t) override {
        fApplyFunc(fProperty(t));
    }

    const std::function<void(const T&)> fApplyFunc;
    AnimatableProperty<T>               fProperty;
};

template <typename T>
bool BindPropertyImpl(const AnimationBuilder& abuilder,
                      const skjson::ObjectValue* jprop,
                      AnimatorScope* ascope,
                      std::function<void(const T&)>&& apply,
                      const T* noop = nullptr) {
    if (auto wrapper = AnimatablePropertyWrapper<T>::Make(abuilder, jprop, std::move(apply))) {
        if (wrapper->isStatic()) {
            if (noop && *noop == wrapper->eval(0)) {
                return false;
            }
            wrapper->tick(0);
        } else {
            ascope->push_back(std::move(wrapper));
        }
        return true;
    }

    return false;
}

} // namespace

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const ScalarValue&)>&& apply,
                  const ScalarValue* noop) const {
    return BindPropertyImpl(*this, jv, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const VectorValue&)>&& apply,
                  const VectorValue* noop) const {
    return BindPropertyImpl(*this, jv, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const ShapeValue&)>&& apply,
                  const ShapeValue* noop) const {
    return BindPropertyImpl(*this, jv, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const TextValue&)>&& apply,
                  const TextValue* noop) const {
    return BindPropertyImpl(*this, jv, fCurrentAnimatorScope, std::move(apply), noop);
}

} // namespace internal
} // namespace skottie
