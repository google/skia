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

class GrLegacyMeshDrawOp;
class SkMatrix;
struct SkRect;

namespace GrAAFillRectOp {
std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& rect,
                                         const SkRect& devRect);

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkMatrix& localMatrix,
                                         const SkRect& rect);

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkMatrix& localMatrix,
                                         const SkRect& rect,
                                         const SkRect& devRect);

std::unique_ptr<GrLegacyMeshDrawOp> MakeWithLocalRect(GrColor color,
                                                      const SkMatrix& viewMatrix,
                                                      const SkRect& rect,
                                                      const SkRect& localRect);
};

#endif
