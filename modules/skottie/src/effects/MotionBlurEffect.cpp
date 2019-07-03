/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/MotionBlurEffect.h"

#include "include/core/SkCanvas.h"
#include "modules/sksg/include/SkSGInvalidationController.h"

namespace skottie {
namespace internal {

sk_sp<MotionBlurEffect> MotionBlurEffect::Make(std::unique_ptr<sksg::Animator> animator,
                                               sk_sp<sksg::RenderNode> child,
                                               size_t samples_per_frame,
                                               float shutter_angle, float shutter_phase) {
    if (!samples_per_frame || shutter_angle <= 0) {
        return nullptr;
    }

    // shutter_angle is [   0 .. 720], mapped to [ 0 .. 2] (frame space)
    // shutter_phase is [-360 .. 360], mapped to [-1 .. 1] (frame space)
    const auto samples_duration = shutter_angle / 360,
                          phase = shutter_phase / 360,
                             dt = samples_duration / (samples_per_frame - 1);

    return sk_sp<MotionBlurEffect>(new MotionBlurEffect(std::move(animator),
                                                        std::move(child),
                                                        samples_per_frame,
                                                        phase, dt));
}

MotionBlurEffect::MotionBlurEffect(std::unique_ptr<sksg::Animator> animator,
                                   sk_sp<sksg::RenderNode> child,
                                   size_t samples, float phase, float dt)
    : INHERITED({std::move(child)})
    , fAnimator(std::move(animator))
    , fSampleCount(samples)
    , fPhase(phase)
    , fDT(dt) {}

const sksg::RenderNode* MotionBlurEffect::onNodeAt(const SkPoint&) const {
    return nullptr;
}

SkRect MotionBlurEffect::onRevalidate(sksg::InvalidationController*, const SkMatrix& ctm) {
    SkASSERT(this->children().size() == 1ul);
    const auto& child = this->children()[0];

    auto bounds = SkRect::MakeEmpty();

    // Use a local inval controller to suppress descendent invals during sampling
    // (superseded by our local inval bounds).
    sksg::InvalidationController ic;

    auto t = fT + fPhase;

    for (size_t i = 0; i < fSampleCount; ++i) {
        fAnimator->tick(t);
        t += fDT;

        if (!child->isVisible()) {
            continue;
        }

        bounds.join(child->revalidate(&ic, ctm));
    }

    return bounds;
}

void MotionBlurEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    SkASSERT(this->children().size() == 1ul);
    const auto& child = this->children()[0];

    SkAutoCanvasRestore acr(canvas, false);

    canvas->saveLayer(this->bounds(), nullptr);

    SkPaint frame_paint;
    frame_paint.setAlphaf(1.0f / fSampleCount);
    frame_paint.setBlendMode(SkBlendMode::kPlus);

    sksg::InvalidationController ic;

    auto t = fT + fPhase;

    for (size_t i = 0; i < fSampleCount; ++i) {
        fAnimator->tick(t);
        t += fDT;

        if (!child->isVisible()) {
            continue;
        }

        child->revalidate(&ic, canvas->getTotalMatrix());

        canvas->saveLayer(nullptr, &frame_paint);
        child->render(canvas, ctx);
        canvas->restore();
    }
}

} // namespace internal
} // namespace skottie
