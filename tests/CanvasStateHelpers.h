/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef CanvasStateHelpers_DEFINED
#define CanvasStateHelpers_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
class SkCanvas;
class SkCanvasState;
class SkRegion;

/*
 *  Helper function to perform drawing to an SkCanvas. Used by both
 *  test_complex_layers and complex_layers_draw_from_canvas_state.
 */
void complex_layers_draw(SkCanvas* canvas, float left, float top,
                         float right, float bottom, int32_t spacer);

/*
 *  Create an SkCanvas from state and draw to it. Return true on success.
 *
 *  Used by test_complex_layers test in CanvasStateTest. Marked as extern
 *  so it can be called from a separate library.
 */
extern "C" bool complex_layers_draw_from_canvas_state(SkCanvasState* state,
        float left, float top, float right, float bottom, int32_t spacer);

/*
 *  Helper function to perform drawing to an SkCanvas. Used both by test_complex_clips
 *  and complex_clips_draw_from_canvas_state.
 */
void complex_clips_draw(SkCanvas* canvas, int32_t left, int32_t top,
        int32_t right, int32_t bottom, int32_t clipOp, const SkRegion& localRegion);

/*
 *  Create an SkCanvas from state and draw to it. Return true on success.
 *
 *  Used by test_complex_clips test in CanvasStateTest. Marked as extern
 *  so it can be called from a separate library.
 */
extern "C" bool complex_clips_draw_from_canvas_state(SkCanvasState* state,
        int32_t left, int32_t top, int32_t right, int32_t bottom, int32_t clipOp,
        int32_t regionRects, int32_t* rectCoords);

#endif // SK_SUPPORT_LEGACY_CLIPTOLAYERFLAG
#endif // CanvasStateHelpers_DEFINED
