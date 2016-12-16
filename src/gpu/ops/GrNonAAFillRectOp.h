/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNonAAFillRectOp_DEFINED
#define GrNonAAFillRectOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrDrawOp;
class SkMatrix;
struct SkRect;

namespace GrNonAAFillRectOp {

sk_sp<GrDrawOp> Make(GrColor color,
                     const SkMatrix& viewMatrix,
                     const SkRect& rect,
                     const SkRect* localRect,
                     const SkMatrix* localMatrix);

sk_sp<GrDrawOp> MakeWithPerspective(GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect* localRect,
                                    const SkMatrix* localMatrix);
};

#endif
