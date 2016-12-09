/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CanvasStateHelpers.h"
#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
#include "SkCanvas.h"
#include "SkCanvasStateUtils.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkRegion.h"

void complex_layers_draw(SkCanvas* canvas, float left, float top,
                         float right, float bottom, int32_t spacer) {
    SkPaint bluePaint;
    bluePaint.setColor(SK_ColorBLUE);
    bluePaint.setStyle(SkPaint::kFill_Style);

    SkRect rect = SkRect::MakeLTRB(left, top, right, bottom);
    canvas->drawRect(rect, bluePaint);
    canvas->translate(0, rect.height() + spacer);
    canvas->drawRect(rect, bluePaint);
}

extern "C" bool complex_layers_draw_from_canvas_state(SkCanvasState* state,
        float left, float top, float right, float bottom, int32_t spacer) {
    std::unique_ptr<SkCanvas> canvas = SkCanvasStateUtils::MakeFromCanvasState(state);
    if (!canvas) {
        return false;
    }
    complex_layers_draw(canvas.get(), left, top, right, bottom, spacer);
    return true;
}

void complex_clips_draw(SkCanvas* canvas, int32_t left, int32_t top,
        int32_t right, int32_t bottom, int32_t clipOp, const SkRegion& localRegion) {
    canvas->save();
    SkRect clipRect = SkRect::MakeLTRB(SkIntToScalar(left), SkIntToScalar(top),
                                       SkIntToScalar(right), SkIntToScalar(bottom));
    canvas->clipRect(clipRect, (SkRegion::Op) clipOp);
    canvas->drawColor(SK_ColorBLUE);
    canvas->restore();

    canvas->clipRegion(localRegion, (SkClipOp) clipOp);
    canvas->drawColor(SK_ColorBLUE);
}

extern "C" bool complex_clips_draw_from_canvas_state(SkCanvasState* state,
        int32_t left, int32_t top, int32_t right, int32_t bottom, int32_t clipOp,
        int32_t regionRects, int32_t* rectCoords) {
    std::unique_ptr<SkCanvas> canvas = SkCanvasStateUtils::MakeFromCanvasState(state);
    if (!canvas) {
        return false;
    }

    SkRegion localRegion;
    for (int32_t i = 0; i < regionRects; ++i) {
        localRegion.op(rectCoords[0], rectCoords[1], rectCoords[2], rectCoords[3],
                       SkRegion::kUnion_Op);
        rectCoords += 4;
    }

    complex_clips_draw(canvas.get(), left, top, right, bottom, clipOp, localRegion);
    return true;
}
#endif // SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
