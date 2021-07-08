/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFillRRectOp_DEFINED
#define GrFillRRectOp_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrOp.h"

class GrCaps;
class GrDrawOp;
class GrPaint;
class GrRecordingContext;
class SkMatrix;
class SkRRect;

namespace GrFillRRectOp {
    GrOp::Owner Make(GrRecordingContext*,
                     SkArenaAlloc*,
                     GrPaint&&,
                     const SkMatrix& viewMatrix,
                     const SkRRect&,
                     const SkRect& localRect,
                     GrAA);
}  // namespace GrFillRRectOp

#endif
