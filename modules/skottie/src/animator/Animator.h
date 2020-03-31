/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAnimator_DEFINED
#define SkottieAnimator_DEFINED

#include "include/core/SkRefCnt.h"

#include <vector>

namespace skjson {

class ObjectValue;

} // namespace skjson

namespace skottie {
namespace internal {

class AnimationBuilder;
class KeyframeAnimatorBuilder;

class Animator : public SkRefCnt {
public:
    virtual ~Animator() = default;

    using StateChanged = bool;
    StateChanged seek(float t) { return this->onSeek(t); }

protected:
    Animator() = default;

    virtual StateChanged onSeek(float t) = 0;

private:
    Animator(const Animator&) = delete;
    Animator& operator=(const Animator&) = delete;
};

class AnimatablePropertyContainer : public Animator {
public:
    // This is the workhorse for property binding: depending on whether the property is animated,
    // it will either apply immediately or instantiate and attach a keyframe animator, scoped to
    // this container.
    template <typename T>
    bool bind(const AnimationBuilder&, const skjson::ObjectValue*, T*);

    template <typename T>
    bool bind(const AnimationBuilder& abuilder, const skjson::ObjectValue* jobject, T& v) {
        return this->bind<T>(abuilder, jobject, &v);
    }

    bool isStatic() const { return fAnimators.empty(); }

protected:
    virtual void onSync() = 0;

    void shrink_to_fit();

    void attachDiscardableAdapter(sk_sp<AnimatablePropertyContainer>);

private:
    StateChanged onSeek(float) final;

    bool bindImpl(const AnimationBuilder&,
                  const skjson::ObjectValue*,
                  KeyframeAnimatorBuilder&,
                  void*);

    std::vector<sk_sp<Animator>> fAnimators;
    bool                         fHasSynced = false;
};

} // namespace internal
} // namespace skottie

#endif // SkottieAnimator_DEFINED
