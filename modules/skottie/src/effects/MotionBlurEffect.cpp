/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/MotionBlurEffect.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkMath.h"
#include "include/core/SkPixmap.h"
#include "include/private/SkVx.h"
#include "modules/skottie/src/animator/Animator.h"
#include "src/core/SkMathPriv.h"

namespace skottie {
namespace internal {

class MotionBlurEffect::AutoInvalBlocker {
public:
    AutoInvalBlocker(const MotionBlurEffect* mb, const sk_sp<RenderNode>& child)
        : fMBNode(const_cast<MotionBlurEffect*>(mb))
        , fChild(child) {
        fMBNode->unobserveInval(fChild);
    }

    ~AutoInvalBlocker() {
        fMBNode->observeInval(fChild);
    }

private:
    MotionBlurEffect*        fMBNode;
    const sk_sp<RenderNode>& fChild;
};

sk_sp<MotionBlurEffect> MotionBlurEffect::Make(sk_sp<Animator> animator,
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

MotionBlurEffect::MotionBlurEffect(sk_sp<Animator> animator,
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

SkRect MotionBlurEffect::seekToSample(size_t sample_idx, const SkMatrix& ctm) const {
    SkASSERT(sample_idx < fSampleCount);
    fAnimator->seek(fT + fPhase + fDT * sample_idx);

    SkASSERT(this->children().size() == 1ul);
    return this->children()[0]->revalidate(nullptr, ctm);
}

SkRect MotionBlurEffect::onRevalidate(sksg::InvalidationController*, const SkMatrix& ctm) {
    SkRect bounds       = SkRect::MakeEmpty();
    fVisibleSampleCount = 0;

    for (size_t i = 0; i < fSampleCount; ++i) {
        bounds.join(this->seekToSample(i, ctm));
        fVisibleSampleCount += SkToSizeT(this->children()[0]->isVisible());
    }

    return bounds;
}

void MotionBlurEffect::renderToRaster8888Pow2Samples(SkCanvas* canvas,
                                                     const RenderContext* ctx) const {
    // canvas is raster backed and RGBA 8888 or BGRA 8888, and fSamples is a power of 2.
    // We can play dirty tricks.

    // Don't worry about "Next"... this is exact.
    const int shift = SkNextLog2(fVisibleSampleCount);
    SkASSERT((size_t(1)<<shift) == fVisibleSampleCount);

    SkASSERT(this->children().size() == 1ul);
    const sk_sp<RenderNode>& child = this->children()[0];

    SkAutoCanvasRestore acr(canvas, false);
    canvas->saveLayer(this->bounds(), nullptr);

    SkImageInfo info;
    size_t rowBytes;
    auto layer = (uint32_t*)canvas->accessTopLayerPixels(&info, &rowBytes);
    SkASSERT(layer);
    SkASSERT(info.colorType() == kRGBA_8888_SkColorType ||
             info.colorType() == kBGRA_8888_SkColorType);

    SkASSERT(!info.isEmpty());
    std::vector<uint64_t> accum(info.width() * info.height());

    SkDEBUGCODE(size_t frames_rendered = 0;)
    bool needs_clear = false;  // Cleared initially by saveLayer().
    for (size_t i = 0; i < fSampleCount; ++i) {
        this->seekToSample(i, canvas->getTotalMatrix());

        if (!child->isVisible()) {
            continue;
        }

        // Draw this subframe.
        if (needs_clear) {
            canvas->clear(0);
        }
        needs_clear = true;
        child->render(canvas, ctx);
        SkDEBUGCODE(frames_rendered++;)

        // Pluck out the pixels we've drawn in the layer.
        const uint32_t* src = layer;
              uint64_t* dst = accum.data();

        for (int y = 0; y < info.height(); y++) {
            // Expand 8-bit to 16-bit and accumulate.
            int n = info.width();
            const auto row = src;
            while (n >= 4) {
                auto s = skvx::Vec<16, uint8_t >::Load(src);
                auto d = skvx::Vec<16, uint16_t>::Load(dst);

                (d + skvx::cast<uint16_t>(s)).store(dst);

                src += 4;
                dst += 4;
                n   -= 4;
            }
            while (n) {
                auto s = skvx::Vec<4, uint8_t >::Load(src);
                auto d = skvx::Vec<4, uint16_t>::Load(dst);

                (d + skvx::cast<uint16_t>(s)).store(dst);

                src += 1;
                dst += 1;
                n   -= 1;
            }
            src = (const uint32_t*)( (const char*)row + rowBytes );
        }
    }
    SkASSERT(frames_rendered == fVisibleSampleCount);

    // Actually draw the frame using the accumulated subframes.
    const uint64_t* src = accum.data();
          uint32_t* dst = layer;
    for (int y = 0; y < info.height(); y++) {
        // Divide accumulated subframes through by sample count.
        int n = info.width();
        const auto row = dst;
        while (n >= 4) {
            auto s = skvx::Vec<16, uint16_t>::Load(src);
            skvx::cast<uint8_t>(s >> shift).store(dst);

            src += 4;
            dst += 4;
            n   -= 4;
        }
        while (n) {
            auto s = skvx::Vec<4, uint16_t>::Load(src);
            skvx::cast<uint8_t>(s >> shift).store(dst);

            src += 1;
            dst += 1;
            n   -= 1;
        }

        dst = (uint32_t*)( (char*)row + rowBytes );
    }
}

void MotionBlurEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    if (!fVisibleSampleCount) {
        return;
    }

    SkASSERT(this->children().size() == 1ul);
    const auto& child = this->children()[0];

    // We're about to mutate/revalidate the subtree for sampling.  Capture the invalidation
    // at this scope, to prevent dirtying ancestor SG nodes (no way to revalidate the global scene).
    AutoInvalBlocker aib(this, child);

    SkPixmap pm;
    if (canvas->peekPixels(&pm) && (canvas->imageInfo().colorType() == kRGBA_8888_SkColorType ||
                                    canvas->imageInfo().colorType() == kBGRA_8888_SkColorType   )
                                && SkIsPow2(fVisibleSampleCount)) {
        this->renderToRaster8888Pow2Samples(canvas, ctx);
        return;
    }

    SkAutoCanvasRestore acr(canvas, false);

    // Accumulate in F16 for more precision.
    canvas->saveLayer(SkCanvas::SaveLayerRec(&this->bounds(), nullptr, SkCanvas::kF16ColorType));

    const float frame_alpha = 1.0f / fVisibleSampleCount;

    // Depending on whether we can defer frame blending,
    // use a local (deferred) RenderContext or an explicit layer for frame/content rendering.
    ScopedRenderContext frame_ctx(canvas, ctx);
    SkPaint             frame_paint;

    const bool isolate_frames = frame_ctx->fBlendMode != SkBlendMode::kSrcOver;
    if (isolate_frames) {
        frame_paint.setAlphaf(frame_alpha);
        frame_paint.setBlendMode(SkBlendMode::kPlus);
    } else {
        frame_ctx = frame_ctx.modulateOpacity(frame_alpha)
                             .modulateBlendMode(SkBlendMode::kPlus);
    }

    SkDEBUGCODE(size_t frames_rendered = 0;)
    for (size_t i = 0; i < fSampleCount; ++i) {
        this->seekToSample(i, canvas->getTotalMatrix());

        if (!child->isVisible()) {
            continue;
        }

        SkAutoCanvasRestore acr(canvas, false);
        if (isolate_frames) {
            canvas->saveLayer(nullptr, &frame_paint);
        }

        child->render(canvas, frame_ctx);
        SkDEBUGCODE(frames_rendered++;)
    }

    SkASSERT(frames_rendered == fVisibleSampleCount);
}

} // namespace internal
} // namespace skottie
