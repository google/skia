/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieMotionBlurEffect_DEFINED
#define SkottieMotionBlurEffect_DEFINED

#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGScene.h"

namespace skottie {
namespace internal {

class MotionBlurEffect final : public sksg::CustomRenderNode {
public:
    static sk_sp<MotionBlurEffect> Make(sk_sp<sksg::Animator> animator,
                                        sk_sp<sksg::RenderNode> child,
                                        size_t samples_per_frame,
                                        float shutter_angle, float shutter_phase);

    SG_ATTRIBUTE(T, float, fT)

private:
    class AutoInvalBlocker;

    const RenderNode* onNodeAt(const SkPoint&) const override;

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override;

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override;

    void renderToRaster8888Pow2Samples(SkCanvas* canvas, const RenderContext* ctx) const;

    SkRect seekToSample(size_t sample_idx, const SkMatrix& ctm) const;

    MotionBlurEffect(sk_sp<sksg::Animator> animator,
                     sk_sp<sksg::RenderNode> child,
                     size_t sample_count, float phase, float dt);

    const sk_sp<sksg::Animator> fAnimator;
    const size_t                fSampleCount;
    const float                 fPhase,
                                fDT;

    float  fT                  = 0;
    size_t fVisibleSampleCount = 0;

    using INHERITED = sksg::CustomRenderNode;
};

} // namespace internal
} // namespace skottie

#endif // SkottieMotionBlurEffect_DEFINED
