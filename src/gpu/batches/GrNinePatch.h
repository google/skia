/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNinePatch_DEFINED
#define GrNinePatch_DEFINED

#include "GrColor.h"
#include "SkCanvas.h"

class GrDrawBatch;
class SkBitmap;
class SkLatticeIter;
class SkMatrix;
struct SkIRect;
struct SkRect;

namespace GrNinePatch {
GrDrawBatch* CreateNonAA(GrColor color, const SkMatrix& viewMatrix, int imageWidth, int imageHeight,
                         std::unique_ptr<SkLatticeIter> iter, const SkRect& dst);
};

#endif
