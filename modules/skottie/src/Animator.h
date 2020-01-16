/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAnimator_DEFINED
#define SkottieAnimator_DEFINED

#include "modules/sksg/include/SkSGScene.h"

namespace skjson {

class ObjectValue;

} // namespace skjson

namespace skottie {
namespace internal {

class AnimationBuilder;

class AnimatablePropertyContainer : public sksg::Animator {
public:
    template <typename T>
    bool bind(const AnimationBuilder&, const skjson::ObjectValue*, T*);

    bool isStatic() const { return fAnimators.empty(); }

protected:
    virtual void onSync() = 0;

private:
    void onTick(float) final;

    sksg::AnimatorList fAnimators;
};

} // namespace internal
} // namespace skottie

#endif // SkottieAnimator_DEFINED
