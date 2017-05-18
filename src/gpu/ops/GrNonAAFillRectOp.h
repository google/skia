/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNonAAFillRectOp_DEFINED
#define GrNonAAFillRectOp_DEFINED

#include <memory>
#include "GrColor.h"

class GrDrawOp;
class GrPaint;
class SkMatrix;
struct SkRect;
struct GrUserStencilSettings;
enum class GrAAType : unsigned;

namespace GrNonAAFillRectOp {
std::unique_ptr<GrDrawOp> Make(GrPaint&&,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect,
                               const SkRect* localRect,
                               const SkMatrix* localMatrix,
                               GrAAType,
                               const GrUserStencilSettings* = nullptr);
};

#endif
