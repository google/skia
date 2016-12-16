/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRegionOp_DEFINED
#define GrRegionOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrDrawOp;
class SkMatrix;
class SkRegion;

namespace GrRegionOp {
sk_sp<GrDrawOp> Make(GrColor color, const SkMatrix& viewMatrix, const SkRegion& region);
}

#endif
