/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAFillRectOp_DEFINED
#define GrAAFillRectOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrDrawOp;
class GrPaint;
class GrUserStencilSettings;
class SkMatrix;
struct SkRect;

namespace GrAAFillRectOp {
std::unique_ptr<GrDrawOp> Make(GrPaint&&, const SkMatrix& viewMatrix, const SkRect&,
                               const GrUserStencilSettings* = nullptr);

std::unique_ptr<GrDrawOp> Make(GrPaint&&, const SkMatrix& viewMatrix, const SkMatrix& localMatrix,
                               const SkRect& rect, const GrUserStencilSettings* = nullptr);

std::unique_ptr<GrDrawOp> Make(GrPaint&&, const SkMatrix& viewMatrix, const SkMatrix& localMatrix,
                               const SkRect& rect, const SkRect& devRect,
                               const GrUserStencilSettings* = nullptr);

std::unique_ptr<GrDrawOp> MakeWithLocalRect(GrPaint&&, const SkMatrix& viewMatrix,
                                            const SkRect& rect, const SkRect& localRect,
                                            const GrUserStencilSettings* = nullptr);
};

#endif
