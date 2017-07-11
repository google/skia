/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GLatticeOp_DEFINED
#define GLatticeOp_DEFINED

#include <memory>
#include "GrTypes.h"

class GrDrawOp;
class GrPaint;
class SkLatticeIter;
class SkMatrix;
struct SkRect;

namespace GrLatticeOp {
std::unique_ptr<GrDrawOp> MakeNonAA(GrPaint&& paint, const SkMatrix& viewMatrix, int imageWidth,
                                    int imageHeight, std::unique_ptr<SkLatticeIter> iter,
                                    const SkRect& dst);
};

#endif
