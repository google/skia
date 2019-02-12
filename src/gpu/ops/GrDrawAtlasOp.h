/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawAtlasOp_DEFINED
#define GrDrawAtlasOp_DEFINED

#include "GrTypesPriv.h"
#include "SkRefCnt.h"

class GrDrawOp;
class GrPaint;
class GrRecordingContext;
class SkMatrix;

namespace GrDrawAtlasOp {
    std::unique_ptr<GrDrawOp> Make(GrRecordingContext*,
                                   GrPaint&&,
                                   const SkMatrix& viewMatrix,
                                   GrAAType,
                                   int spriteCount,
                                   const SkRSXform* xforms,
                                   const SkRect* rects,
                                   const SkColor* colors);
};

#endif
