/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "modules/canvaskit/bidi_c_api.h"

#include <emscripten.h>

extern "C" {

EMSCRIPTEN_KEEPALIVE
int skbidi_getBidiRegions(const uint16_t* text, int textCount, int dir, int32_t* out_regions) {
    return skbidi::getBidiRegions(text, textCount, dir, out_regions);
}

EMSCRIPTEN_KEEPALIVE
int skbidi_reorderVisual(const SkUnicode::BidiLevel* runLevels,
                         int levelsCount,
                         int32_t* logicalFromVisual) {
    return skbidi::reorderVisual(runLevels, levelsCount, logicalFromVisual);
}

EMSCRIPTEN_KEEPALIVE
int skbidi_computeCodeUnitFlags(const uint16_t* text, int textCount, uint16_t* out_flags) {
    return skbidi::computeCodeUnitFlags(text, textCount, out_flags);
}

}  // extern "C"
