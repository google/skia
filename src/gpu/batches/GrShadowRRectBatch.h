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
class GrStyle;
class SkMatrix;
struct SkRect;
class SkRRect;
class SkStrokeRec;

GrDrawBatch* CreateShadowRRectBatch(GrColor,
                                    bool needsDistance,
                                    const SkMatrix& viewMatrix,
                                    const SkRRect& rrect,
                                    const SkStrokeRec& stroke,
                                    const SkScalar blurRadius,
                                    const GrShaderCaps* shaderCaps);

#endif
