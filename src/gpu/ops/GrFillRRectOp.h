/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFillRRectOp_DEFINED
#define GrFillRRectOp_DEFINED

#include "include/private/GrTypesPriv.h"

class GrCaps;
class GrDrawOp;
class GrPaint;
class GrRecordingContext;
class SkMatrix;
class SkRRect;

namespace GrFillRRectOp {
    std::unique_ptr<GrDrawOp> Make(GrRecordingContext*,
                                   GrPaint&&,
                                   const SkMatrix& viewMatrix,
                                   const SkRRect&,
                                   GrAAType);
};

#endif
