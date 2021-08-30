/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ShadowRRectOp_DEFINED
#define ShadowRRectOp_DEFINED

#include <memory>
#include "src/gpu/GrColor.h"
#include "src/gpu/ops/GrOp.h"

class GrRecordingContext;

class SkMatrix;
class SkRRect;

namespace skgpu::v1::ShadowRRectOp {

GrOp::Owner Make(GrRecordingContext*,
                 GrColor,
                 const SkMatrix& viewMatrix,
                 const SkRRect&,
                 SkScalar blurWidth,
                 SkScalar insetWidth);

} // namespace skgpu::v1::ShadowRRectOp

#endif // ShadowRRectOp_DEFINED
