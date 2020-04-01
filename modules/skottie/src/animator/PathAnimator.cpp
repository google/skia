/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/animator/Vector.h"

namespace skottie::internal {

template <>
bool AnimatablePropertyContainer::bind<PathValue>(const AnimationBuilder& abuilder,
                                                  const skjson::ObjectValue* jprop,
                                                  PathValue* v) {
    VectorKeyframeAnimatorBuilder builder(PathValue::ParseEncodingLen,
                                          PathValue::ParseEncodingData);
    return this->bindImpl(abuilder, jprop, builder, &v->fData);
}

} // namespace skottie::internal
