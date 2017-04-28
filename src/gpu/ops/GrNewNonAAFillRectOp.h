/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNewNonAAFillRectOp_DEFINED
#define GrNewNonAAFillRectOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrMeshDrawOp;
class GrPaint;
class SkMatrix;
struct SkRect;
struct GrUserStencilSettings;

namespace GrNewNonAAFillRectOp {
std::unique_ptr<GrMeshDrawOp> Make(GrPaint&&,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rect,
                                   const SkRect* localRect,
                                   const SkMatrix* localMatrix,
                                   bool useHWAA,
                                   const GrUserStencilSettings* = nullptr);
};

#endif
