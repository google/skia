/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRegionOp_DEFINED
#define GrRegionOp_DEFINED

#include "GrTypesPriv.h"

class GrDrawOp;
class SkMatrix;
class SkRegion;
class GrPaint;

namespace GrRegionOp {
/** GrAAType must be kNone or kMSAA. */
std::unique_ptr<GrDrawOp> Make(GrPaint&&, const SkMatrix& viewMatrix, const SkRegion&, GrAAType);
}

#endif
