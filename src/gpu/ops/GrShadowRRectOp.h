/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShadowRRectOp_DEFINED
#define GrShadowRRectOp_DEFINED

#include <memory>
#include "src/gpu/GrColor.h"

class GrDrawOp;
class GrRecordingContext;

class SkMatrix;
class SkRRect;

namespace GrShadowRRectOp {

std::unique_ptr<GrDrawOp> Make(GrRecordingContext*,
                               GrColor,
                               const SkMatrix& viewMatrix,
                               const SkRRect&,
                               SkScalar blurWidth,
                               SkScalar insetWidth);
}

#endif
