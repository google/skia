/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/animator/Animator.h"

#include "include/core/SkContourMeasure.h"
#include "include/core/SkCubicMap.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/text/TextValue.h"

#include <cmath>
#include <vector>

#define DUMP_KF_RECORDS 0

namespace skottie {
namespace internal {

void AnimatablePropertyContainer::onTick(float t) {
    for (const auto& animator : fAnimators) {
        animator->tick(t);
    }
    this->onSync();
}

void AnimatablePropertyContainer::attachDiscardableAdapter(
        sk_sp<AnimatablePropertyContainer> child) {
    if (!child) {
        return;
    }

    if (child->isStatic()) {
        child->tick(0);
        return;
    }

    fAnimators.push_back(child);
}

void AnimatablePropertyContainer::shrink_to_fit() {
    fAnimators.shrink_to_fit();
}

} // namespace internal
} // namespace skottie
