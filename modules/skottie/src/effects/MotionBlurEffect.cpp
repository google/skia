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
#include "modules/sksg/include/SkSGInvalidationController.h"
#include "src/core/SkMathPriv.h"

namespace skottie {
namespace internal {

sk_sp<MotionBlurEffect> MotionBlurEffect::Make(sk_sp<sksg::Animator> animator,
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

MotionBlurEffect::MotionBlurEffect(sk_sp<sksg::Animator> animator,
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
    const sk_sp<RenderNode>& child = this->children()[0];

    SkRect bounds = SkRect::MakeEmpty();

    // Use a local inval controller to suppress descendent invals during sampling
    // (superseded by our local inval bounds).
    sksg::InvalidationController ic;

    float t = fT + fPhase;

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

void MotionBlurEffect::renderToRaster8888Pow2Samples(SkCanvas* canvas,
                                                     const RenderContext* ctx) const {
    // canvas is raster backed and RGBA 8888 or BGRA 8888, and fSamples is a power of 2.
    // We can play dirty tricks.

    // Don't worry about "Next"... this is exact.
    const int shift = SkNextLog2(fSampleCount);
    SkASSERT((size_t(1)<<shift) == fSampleCount);

    SkASSERT(this->children().size() == 1ul);
    const sk_sp<RenderNode>& child = this->children()[0];

    std::vector<uint64_t> accum;
    canvas->saveLayer(this->bounds(), nullptr);
    {
        SkImageInfo info;
        size_t rowBytes;
        auto layer = (uint32_t*)canvas->accessTopLayerPixels(&info, &rowBytes);
        SkASSERT(layer);
        SkASSERT(info.colorType() == kRGBA_8888_SkColorType ||
                 info.colorType() == kBGRA_8888_SkColorType);

        float t = fT + fPhase;
        bool needs_clear = false;  // Cleared initially by saveLayer().
        for (size_t i = 0; i < fSampleCount; ++i) {
            fAnimator->tick(t);
            t += fDT;

            if (!child->isVisible()) {
                continue;
            }

            child->revalidate(nullptr, canvas->getTotalMatrix());

            // Draw this subframe.
            if (needs_clear) {
                canvas->clear(0);
            }
            needs_clear = true;
            child->render(canvas, ctx);

            // Pluck out the pixels we've drawn in the layer.
            const uint32_t* src = layer;
            if (accum.empty()) {
                accum.resize(info.width() * info.height());
            }
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

        // Actually draw the frame using the accumulated subframes.
        if (!accum.empty()) {
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
    }
    canvas->restore();
}

void MotionBlurEffect::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    SkASSERT(this->children().size() == 1ul);
    const auto& child = this->children()[0];

    SkPixmap pm;
    if (canvas->peekPixels(&pm) && (canvas->imageInfo().colorType() == kRGBA_8888_SkColorType ||
                                    canvas->imageInfo().colorType() == kBGRA_8888_SkColorType   )
                                && SkIsPow2(fSampleCount)) {
        this->renderToRaster8888Pow2Samples(canvas, ctx);
        return;
    }

    SkAutoCanvasRestore acr(canvas, false);

    // Accumulate in F16 for more precision.
    canvas->saveLayer(SkCanvas::SaveLayerRec(&this->bounds(), nullptr, SkCanvas::kF16ColorType));

    const float frame_alpha = 1.0f / fSampleCount;

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

    float t = fT + fPhase;

    for (size_t i = 0; i < fSampleCount; ++i) {
        fAnimator->tick(t);
        t += fDT;

        if (!child->isVisible()) {
            continue;
        }

        child->revalidate(nullptr, canvas->getTotalMatrix());

        SkAutoCanvasRestore acr(canvas, false);
        if (isolate_frames) {
            canvas->saveLayer(nullptr, &frame_paint);
        }

        child->render(canvas, frame_ctx);
    }
}

} // namespace internal
} // namespace skottie
