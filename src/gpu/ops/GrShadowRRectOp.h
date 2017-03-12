/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShadowRRectOp_DEFINED
#define GrShadowRRectOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrDrawOp;
class GrShaderCaps;
class SkMatrix;
class SkRRect;
class SkStrokeRec;

namespace GrShadowRRectOp {

std::unique_ptr<GrDrawOp> Make(GrColor, const SkMatrix& viewMatrix, const SkRRect& rrect,
                               const SkScalar blurRadius, const SkStrokeRec& stroke,
                               const GrShaderCaps* shaderCaps);
}

#endif
