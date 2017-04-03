/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLatticeOp_DEFINED
#define GLatticeOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrLegacyMeshDrawOp;
class SkLatticeIter;
class SkMatrix;
struct SkRect;

namespace GrLatticeOp {
std::unique_ptr<GrLegacyMeshDrawOp> MakeNonAA(GrColor color, const SkMatrix& viewMatrix,
                                              int imageWidth, int imageHeight,
                                              std::unique_ptr<SkLatticeIter> iter,
                                              const SkRect& dst);
};

#endif
