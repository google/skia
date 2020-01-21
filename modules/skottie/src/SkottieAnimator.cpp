/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Animator.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/TextValue.h"

namespace skottie {
namespace internal {

namespace {

template <typename T>
class LegacyAnimatorAdapter final : public AnimatablePropertyContainer {
public:
    LegacyAnimatorAdapter(const AnimationBuilder& abuilder,
                          const skjson::ObjectValue* jprop,
                          std::function<void(const T&)>&& apply)
        : fApplyFunc(std::move(apply)) {
        if (!this->bind<T>(abuilder, jprop, &fValue)) {
            fValue = T();
        }
    }

    const T& value() const { return fValue; }

private:
    void onSync() override {
        fApplyFunc(fValue);
    }

    const std::function<void(const T&)> fApplyFunc;
    T                                   fValue;
};

template <typename T>
bool BindLegacyPropertyImpl(const AnimationBuilder& abuilder,
                            const skjson::ObjectValue* jprop,
                            AnimatorScope* ascope,
                            std::function<void(const T&)>&& apply,
                            const T* noop) {
    if (!jprop) {
        return false;
    }

    auto adapter = sk_make_sp<LegacyAnimatorAdapter<T>>(abuilder, jprop, std::move(apply));

    if (adapter->isStatic()) {
        if (noop && *noop == adapter->value()) {
            return false;
        }
        adapter->tick(0);
    } else {
        ascope->push_back(std::move(adapter));
    }

    return true;
}

} // namespace

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const ScalarValue&)>&& apply,
                  const ScalarValue* noop) const {
    return BindLegacyPropertyImpl(*this, jv, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const VectorValue&)>&& apply,
                  const VectorValue* noop) const {
    return BindLegacyPropertyImpl(*this, jv, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const ShapeValue&)>&& apply,
                  const ShapeValue* noop) const {
    return BindLegacyPropertyImpl(*this, jv, fCurrentAnimatorScope, std::move(apply), noop);
}

template <>
bool AnimationBuilder::bindProperty(const skjson::Value& jv,
                  std::function<void(const TextValue&)>&& apply,
                  const TextValue* noop) const {
    return BindLegacyPropertyImpl(*this, jv, fCurrentAnimatorScope, std::move(apply), noop);
}

} // namespace internal
} // namespace skottie
