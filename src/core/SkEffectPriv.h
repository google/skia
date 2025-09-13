/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEffectPriv_DEFINED
#define SkEffectPriv_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkRect.h"

class SkArenaAlloc;
class SkColorSpace;
class SkRasterPipeline;
class SkSurfaceProps;

// Passed to effects that will add stages to rasterpipeline
struct SkStageRec {
    SkRasterPipeline*       fPipeline;
    SkArenaAlloc*           fAlloc;
    SkColorType             fDstColorType;
    SkColorSpace*           fDstCS;         // may be nullptr
    SkColor4f               fPaintColor;
    const SkSurfaceProps&   fSurfaceProps;
    // The device-space bounding box of the geometry being drawn.
    // An empty value can be used when it is expensive to compute,
    // in which case a heuristic will be used if necessary.
    SkRect fDstBounds;
};

#endif // SkEffectPriv_DEFINED
