/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/MotionBlurEffect.h"

namespace skottie {
namespace internal {

sk_sp<MotionBlurEffect> MotionBlurEffect::Make(std::unique_ptr<sksg::Animator> animator,
                                               sk_sp<sksg::RenderNode>, size_t samples,
                                               float shutter_angle, float shutter_phase) {
    return nullptr;
}

MotionBlurEffect::MotionBlurEffect(std::unique_ptr<sksg::Animator> animator,
                                   sk_sp<sksg::RenderNode> child,
                                   size_t samples, float t_offset, float dt)
    : INHERITED({std::move(child)})
    , fAnimator(std::move(animator))
    , fSamples(samples)
    , fTOffset(t_offset)
    , fDT(dt) {}

} // namespace internal
} // namespace skottie
