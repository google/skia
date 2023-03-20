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
};

#endif // SkEffectPriv_DEFINED
