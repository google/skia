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

struct SkV2;

namespace skjson {

class ObjectValue;
class StringValue;

} // namespace skjson

namespace skottie {

class SlotManager;

namespace internal {

class AnimationBuilder;
class AnimatorBuilder;

class Animator : public SkRefCnt {
public:
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

    // A flavor of bind<Vec2Value> which drives an additional/optional orientation target
    // (rotation in degrees), when bound to a motion path property.
    bool bindAutoOrientable(const AnimationBuilder& abuilder,
                            const skjson::ObjectValue* jobject,
                            SkV2* v, float* orientation);

    bool isStatic() const { return fAnimators.empty() && !fHasSlotID; }

protected:
    friend class skottie::SlotManager;
    virtual void onSync() = 0;

    void shrink_to_fit();

    void attachDiscardableAdapter(sk_sp<AnimatablePropertyContainer>);

private:
    StateChanged onSeek(float) final;

    bool bindImpl(const AnimationBuilder&, const skjson::ObjectValue*, AnimatorBuilder&);

    std::vector<sk_sp<Animator>> fAnimators;
    bool                         fHasSynced = false;
    bool                         fHasSlotID = false;
};

} // namespace internal
} // namespace skottie

#endif // SkottieAnimator_DEFINED
