/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAnimator_DEFINED
#define SkottieAnimator_DEFINED

#include "SkSGScene.h"

#include <functional>

namespace skjson { class Value; }

namespace skottie {

// This is the workhorse for property binding: depending on whether the property is animated,
// it will either apply immediately or instantiate and attach a keyframe animator.
template <typename T>
bool BindProperty(const skjson::Value&,
                  sksg::AnimatorList*,
                  std::function<void(const T&)>&&,
                  const T* default_igore = nullptr);

template <typename T>
bool BindProperty(const skjson::Value& jv,
                  sksg::AnimatorList* animators,
                  std::function<void(const T&)>&& apply,
                  const T& default_ignore) {
    return BindProperty(jv, animators, std::move(apply), &default_ignore);
}

} // namespace skottie

#endif // SkottieAnimator_DEFINED
