/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ShadowRRectOp_DEFINED
#define ShadowRRectOp_DEFINED

#include "include/core/SkScalar.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/ops/GrOp.h"

class GrRecordingContext;

class SkMatrix;
class SkRRect;

namespace skgpu::ganesh::ShadowRRectOp {

GrOp::Owner Make(GrRecordingContext*,
                 GrColor,
                 const SkMatrix& viewMatrix,
                 const SkRRect&,
                 SkScalar blurWidth,
                 SkScalar insetWidth);

}  // namespace skgpu::ganesh::ShadowRRectOp

#endif // ShadowRRectOp_DEFINED
