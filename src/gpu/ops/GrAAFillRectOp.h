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
class SkMatrix;
struct SkRect;

namespace GrAAFillRectOp {
sk_sp<GrDrawOp> Make(GrColor color,
                     const SkMatrix& viewMatrix,
                     const SkRect& rect,
                     const SkRect& devRect);

sk_sp<GrDrawOp> Make(GrColor color,
                     const SkMatrix& viewMatrix,
                     const SkMatrix& localMatrix,
                     const SkRect& rect);

sk_sp<GrDrawOp> Make(GrColor color,
                     const SkMatrix& viewMatrix,
                     const SkMatrix& localMatrix,
                     const SkRect& rect,
                     const SkRect& devRect);

sk_sp<GrDrawOp> MakeWithLocalRect(GrColor color,
                                  const SkMatrix& viewMatrix,
                                  const SkRect& rect,
                                  const SkRect& localRect);
};

#endif
