/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectOpFactory_DEFINED
#define GrRectOpFactory_DEFINED

#include <memory>
#include "GrTypes.h"

enum class GrAAType : unsigned;
class GrDrawOp;
class GrPaint;
struct GrUserStencilSettings;
class SkMatrix;
struct SkRect;
class SkStrokeRec;

namespace GrRectOpFactory {
/** AA Fill */

std::unique_ptr<GrDrawOp> MakeAAFill(GrPaint&&, const SkMatrix&, const SkRect&,
                                     const GrUserStencilSettings* = nullptr);

// Using this method to supply a device rect simply saves recalculation if the device rect is known.
std::unique_ptr<GrDrawOp> MakeAAFillWithDevRect(GrPaint&&, const SkMatrix& viewMatrix,
                                                const SkRect& rect, const SkRect& devRect);

std::unique_ptr<GrDrawOp> MakeAAFillWithLocalMatrix(GrPaint&&, const SkMatrix& viewMatrix,
                                                    const SkMatrix& localMatrix, const SkRect&);

std::unique_ptr<GrDrawOp> MakeAAFillWithLocalRect(GrPaint&&, const SkMatrix&, const SkRect& rect,
                                                  const SkRect& localRect);

/** Non-AA Fill - GrAAType must be either kNone or kMSAA. */

std::unique_ptr<GrDrawOp> MakeNonAAFill(GrPaint&&, const SkMatrix& viewMatrix, const SkRect& rect,
                                        GrAAType, const GrUserStencilSettings* = nullptr);

std::unique_ptr<GrDrawOp> MakeNonAAFillWithLocalMatrix(GrPaint&&, const SkMatrix& viewMatrix,
                                                       const SkMatrix& localMatrix, const SkRect&,
                                                       GrAAType,
                                                       const GrUserStencilSettings* = nullptr);

std::unique_ptr<GrDrawOp> MakeNonAAFillWithLocalRect(GrPaint&&, const SkMatrix&, const SkRect& rect,
                                                     const SkRect& localRect, GrAAType);

/** AA Stroke */

std::unique_ptr<GrDrawOp> MakeAAStroke(GrPaint&&, const SkMatrix&, const SkRect&,
                                       const SkStrokeRec&);

// rects[0] == outer rectangle, rects[1] == inner rectangle
std::unique_ptr<GrDrawOp> MakeAAFillNestedRects(GrPaint&&, const SkMatrix&, const SkRect rects[2]);

/** Non-AA Stroke - GrAAType must be either kNone or kMSAA. */

std::unique_ptr<GrDrawOp> MakeNonAAStroke(GrPaint&&, const SkMatrix&, const SkRect&,
                                          const SkStrokeRec&, bool snapToPixelCenters, GrAAType);

}  // namespace GrRectOpFactory

#endif
