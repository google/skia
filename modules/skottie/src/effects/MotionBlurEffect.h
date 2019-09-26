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

namespace sksg { class Transform; }

namespace skottie {
namespace internal {

class MotionBlurEffect : public sksg::CustomRenderNode {
public:
    static sk_sp<MotionBlurEffect> Make(sk_sp<sksg::Animator> animator,
                                        sk_sp<sksg::RenderNode> child,
                                        size_t samples_per_frame,
                                        float shutter_angle, float shutter_phase);
    static sk_sp<MotionBlurEffect> MakeTransformOnly(sk_sp<sksg::Animator> animator,
                                                     sk_sp<sksg::Transform> xform,
                                                     sk_sp<sksg::RenderNode> child,
                                                     size_t samples_per_frame,
                                                     float shutter_angle, float shutter_phase);

    SG_ATTRIBUTE(T, float, fT)

protected:
    class AutoInvalBlocker;

    void seekSample(size_t index) const;

    const RenderNode* onNodeAt(const SkPoint&) const final;

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override;

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override;

    void renderToRaster8888Pow2Samples(SkCanvas* canvas, const RenderContext* ctx) const;

    MotionBlurEffect(sk_sp<sksg::Animator> animator,
                     sk_sp<sksg::RenderNode> child,
                     size_t samples_per_frame, float shutter_angle, float shutter_phase);

    const sk_sp<sksg::Animator> fAnimator;
    const size_t                fSampleCount;
    const float                 fPhase,
                                fDT;

    float fT = 0;

private:
    using INHERITED = sksg::CustomRenderNode;
};

} // namespace internal
} // namespace skottie

#endif // SkottieMotionBlurEffect_DEFINED
