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

namespace Json { class Value; }

namespace skottie {

// This is the workhorse for property binding: depending on whether the property is animated,
// it will either apply immediately or instantiate and attach a keyframe animator.
template <typename T>
bool BindProperty(const Json::Value&,
                  sksg::AnimatorList*,
                  std::function<void(const T&)>&&,
                  const T* noop = nullptr);

} // namespace skottie

#endif // SkottieAnimator_DEFINED
