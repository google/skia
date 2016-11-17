/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrShadowRRectBatch_DEFINED
#define GrShadowRRectBatch_DEFINED

#include "GrColor.h"

class GrDrawBatch;
class GrShaderCaps;
class SkMatrix;
class SkRRect;
class SkStrokeRec;

GrDrawBatch* CreateShadowRRectBatch(GrColor,
                                    const SkMatrix& viewMatrix,
                                    const SkRRect& rrect,
                                    const SkScalar blurRadius,
                                    const SkStrokeRec& stroke,
                                    const GrShaderCaps* shaderCaps);

#endif
