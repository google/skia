/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

static constexpr float kStrokeWidth = 100;
static constexpr int kTestWidth = 120 * 4;
static constexpr int kTestHeight = 120 * 3 + 140;

static void draw_strokes(SkCanvas* canvas, SkColor strokeColor, const SkPath& path,
                         const SkPath& cubic) {
    SkPaint strokePaint;
    strokePaint.setAntiAlias(true);
    strokePaint.setStrokeWidth(kStrokeWidth);
    strokePaint.setColor(strokeColor);
    strokePaint.setStyle(SkPaint::kStroke_Style);

    SkAutoCanvasRestore arc(canvas, true);
    strokePaint.setStrokeJoin(SkPaint::kBevel_Join);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    strokePaint.setStrokeJoin(SkPaint::kRound_Join);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    strokePaint.setStrokeJoin(SkPaint::kMiter_Join);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    canvas->drawPath(cubic, strokePaint);
}

static void draw_test(SkCanvas* canvas, SkColor strokeColor) {
    SkAutoCanvasRestore arc(canvas, true);
    canvas->translate(60, 60);
    canvas->clear(SK_ColorBLACK);

    draw_strokes(canvas, strokeColor,
            SkPath().lineTo(10,0).lineTo(10,10),
            SkPath().cubicTo(10,0, 10,0, 10,10));
    canvas->translate(0, 120);

    draw_strokes(canvas, strokeColor,
            SkPath().lineTo(0,-10).lineTo(0,10),
            SkPath().cubicTo(0,-10, 0,-10, 0,10));
    canvas->translate(0, 120);

    draw_strokes(canvas, strokeColor,
            SkPath().lineTo(0,-10).lineTo(10,-10).lineTo(10,10).lineTo(0,10),
            SkPath().cubicTo(0,-10, 10,10, 0,10));
    canvas->translate(0, 140);

    draw_strokes(canvas, strokeColor,
            SkPath().lineTo(0,-10).lineTo(10,-10).lineTo(10,0).lineTo(0,0),
            SkPath().cubicTo(0,-10, 10,0, 0,0));
    canvas->translate(0, 120);
}

DEF_SIMPLE_GM(widebuttcaps, canvas, kTestWidth, kTestHeight) {
    draw_test(canvas, SK_ColorGREEN);
}

class WideButtCaps_tess_segs_5 : public skiagm::GpuGM {
    SkString onShortName() override {
        return SkString("widebuttcaps_tess_segs_5");
    }

    SkISize onISize() override {
        return SkISize::Make(kTestWidth, kTestHeight);
    }

    // Pick a very small, odd (and better yet, prime) number of segments.
    //
    // - Odd because it makes the tessellation strip asymmetric, which will be important to test for
    //   future plans that involve drawing in reverse order.
    //
    // - >=4 because the tessellator code will just assume we have enough to combine a miter join
    //   and line in a single patch. (Requires 4 segments. Spec required minimum is 64.)
    static constexpr int kMaxTessellationSegmentsOverride = 5;

    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fMaxTessellationSegmentsOverride = kMaxTessellationSegmentsOverride;
        // Only allow the tessellation path renderer.
        options->fGpuPathRenderers = (GpuPathRenderers)((int)options->fGpuPathRenderers &
                                                        (int)GpuPathRenderers::kTessellation);
    }

    DrawResult onDraw(GrRecordingContext* context,
                      GrSurfaceDrawContext* rtc, SkCanvas* canvas,
                      SkString* errorMsg) override {
        if (!context->priv().caps()->shaderCaps()->tessellationSupport() ||
            !GrTessellationPathRenderer::IsSupported(*context->priv().caps())) {
            errorMsg->set("Tessellation not supported.");
            return DrawResult::kSkip;
        }
        auto opts = context->priv().drawingManager()->testingOnly_getOptionsForPathRendererChain();
        if (!(opts.fGpuPathRenderers & GpuPathRenderers::kTessellation)) {
            errorMsg->set("GrTessellationPathRenderer disabled.");
            return DrawResult::kSkip;
        }
        if (context->priv().caps()->shaderCaps()->maxTessellationSegments() !=
            kMaxTessellationSegmentsOverride) {
            errorMsg->set("modifyGrContextOptions() did not limit maxTessellationSegments. "
                          "(Are you running viewer? If so use '--maxTessellationSegments 5'.)");
            return DrawResult::kFail;
        }
        // Suppress a tessellator warning message that caps.maxTessellationSegments is too small.
        GrRecordingContextPriv::AutoSuppressWarningMessages aswm(context);
        draw_test(canvas, SK_ColorRED);
        return DrawResult::kOk;
    }
};

DEF_GM( return new WideButtCaps_tess_segs_5; )
