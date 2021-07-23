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
#include "include/gpu/GrDirectContext.h"
#include "include/utils/SkRandom.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

static constexpr float kStrokeWidth = 100;
static constexpr int kTestWidth = 120 * 4;
static constexpr int kTestHeight = 120 * 3 + 140;

static void draw_strokes(SkCanvas* canvas, SkRandom* rand, const SkPath& path,
                         const SkPath& cubic) {
    SkPaint strokePaint;
    strokePaint.setAntiAlias(true);
    strokePaint.setStrokeWidth(kStrokeWidth);
    strokePaint.setStyle(SkPaint::kStroke_Style);

    SkAutoCanvasRestore arc(canvas, true);
    strokePaint.setStrokeJoin(SkPaint::kBevel_Join);
    strokePaint.setColor(rand->nextU() | 0xff808080);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    strokePaint.setStrokeJoin(SkPaint::kRound_Join);
    strokePaint.setColor(rand->nextU() | 0xff808080);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    strokePaint.setStrokeJoin(SkPaint::kMiter_Join);
    strokePaint.setColor(rand->nextU() | 0xff808080);
    canvas->drawPath(path, strokePaint);

    canvas->translate(120, 0);
    strokePaint.setColor(rand->nextU() | 0xff808080);
    canvas->drawPath(cubic, strokePaint);
}

static void draw_test(SkCanvas* canvas) {
    SkRandom rand;

    if (canvas->recordingContext() &&
        canvas->recordingContext()->priv().caps()->shaderCaps()->tessellationSupport() &&
        canvas->recordingContext()->priv().caps()->shaderCaps()->maxTessellationSegments() == 5) {
        // The caller successfully overrode the max tessellation segments to 5. Indicate this in the
        // background color.
        canvas->clear(SkColorSetARGB(255, 64, 0, 0));
    } else {
        canvas->clear(SK_ColorBLACK);
    }

    SkAutoCanvasRestore arc(canvas, true);
    canvas->translate(60, 60);

    draw_strokes(canvas, &rand,
            SkPath().lineTo(10,0).lineTo(10,10),
            SkPath().cubicTo(10,0, 10,0, 10,10));
    canvas->translate(0, 120);

    draw_strokes(canvas, &rand,
            SkPath().lineTo(0,-10).lineTo(0,10),
            SkPath().cubicTo(0,-10, 0,-10, 0,10));
    canvas->translate(0, 120);

    draw_strokes(canvas, &rand,
            SkPath().lineTo(0,-10).lineTo(10,-10).lineTo(10,10).lineTo(0,10),
            SkPath().cubicTo(0,-10, 10,10, 0,10));
    canvas->translate(0, 140);

    draw_strokes(canvas, &rand,
            SkPath().lineTo(0,-10).lineTo(10,-10).lineTo(10,0).lineTo(0,0),
            SkPath().cubicTo(0,-10, 10,0, 0,0));
    canvas->translate(0, 120);
}

DEF_SIMPLE_GM(widebuttcaps, canvas, kTestWidth, kTestHeight) {
    canvas->clear(SK_ColorBLACK);
    draw_test(canvas);
}

class WideButtCaps_tess_segs_5 : public skiagm::GM {
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
        options->fAlwaysPreferHardwareTessellation = true;
        // Only allow the tessellation path renderer.
        options->fGpuPathRenderers = (GpuPathRenderers)((int)options->fGpuPathRenderers &
                                                        (int)GpuPathRenderers::kTessellation);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!dContext) {
            *errorMsg = "GM relies on having access to a live direct context.";
            return DrawResult::kSkip;
        }

        if (!dContext->priv().caps()->shaderCaps()->tessellationSupport() ||
            !GrTessellationPathRenderer::IsSupported(*dContext->priv().caps())) {
            errorMsg->set("Tessellation not supported.");
            return DrawResult::kSkip;
        }
        auto opts = dContext->priv().drawingManager()->testingOnly_getOptionsForPathRendererChain();
        if (!(opts.fGpuPathRenderers & GpuPathRenderers::kTessellation)) {
            errorMsg->set("GrTessellationPathRenderer disabled.");
            return DrawResult::kSkip;
        }
        if (dContext->priv().caps()->shaderCaps()->maxTessellationSegments() !=
            kMaxTessellationSegmentsOverride) {
            errorMsg->set("modifyGrContextOptions() did not limit maxTessellationSegments. "
                          "(Are you running viewer? If so use '--maxTessellationSegments 5'.)");
            return DrawResult::kFail;
        }
        // Suppress a tessellator warning message that caps.maxTessellationSegments is too small.
        GrRecordingContextPriv::AutoSuppressWarningMessages aswm(dContext);
        draw_test(canvas);
        return DrawResult::kOk;
    }
};

DEF_GM( return new WideButtCaps_tess_segs_5; )
